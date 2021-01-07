/* Greenhouse Greenhouse Powered by an Arduino MEGA
    Major Revision Start: 01/03/2020
*/
// Adafruit Sensor Library must be installed, DHT.H library is dependent on it, or sketch won't compile!
#include "DHT.h" //DHT11 sensor library
#include "OneWire.h" // Library needed for DS18B20 sensors
#include "DallasTemperature.h" // Library needed for DS18B20 sensors

//Setup serial input checking
int incomingByte = 0; // for incoming serial data
unsigned long tempTime = 0; // millis - heaterTime
unsigned long heaterTime = 0; //Heater cooldown timer
int heatOn = 6000; // ms heater is on 600000
int heatCooldown = 7000; // ms heater cools down, to avoid shutoff 660000

unsigned long loopTime = 0; // Loop timer
int loopInterval = 5000; // ms between loop measurement intervals
int dhtData[3][2]; // Temp and humidity data from sensors

//String dhtDataDesc[3][2] = {
String dhtDataDesc [3][2] = {
  {"Enclosure Temperature,", "Enclosure Humidity,"},
  {"Greenhouse Temperature,", "Greenhouse Humidity,"},
  {"Ambient Temperature,", "Ambient Humidity,"},
  //{"Cooling Output Temp,", "Cooling Output Humidity, "}
};

//int heatArr[7][2] = {
int heatArr[7][2] = {
  {0, 100}, // Ambient Heating Temps (not used)
  {55, 60}, // Greenhouse Temps
  {90, 100}, // reservoir Temps
  {130, 140}, // Radiator Temps
  {78, 80}, // Bench #1 Temps
  {78, 80}, // Bench #2 Temps
  {78, 80}, // Bench #3 Temps
};

#define enclosureDHT22 22
#define greenhouseDHT22 23
#define ambientDHT22 24
//#define coolingDHT22 25

DHT dht[] = {
  {enclosureDHT22, DHT22},
  {greenhouseDHT22, DHT22},
  {ambientDHT22, DHT22},
  //{coolingDHT22, DHT22},
};

// DS18B20 Sensor Setup
// Connect all DS18B20 to pin 30
OneWire oneWire(30);
DallasTemperature sensors(&oneWire);

// DS18B20 Sensor Addresses
uint8_t oneWireAddress[7][8] = {
  { 0x28, 0xAA, 0xC4, 0x13, 0x1B, 0x13, 0x02, 0x8E }, // #1 OK 01/06/2021
  { 0x28, 0xA9, 0xAC, 0x0F, 0x30, 0x14, 0x01, 0x0B }, // #2 OK 01/06/2021
  { 0x28, 0xAA, 0xE8, 0xBD, 0x37, 0x14, 0x01, 0x48 }, // #4 OK 01/06/2021
  { 0x28, 0xAA, 0x3C, 0xF9, 0x37, 0x14, 0x01, 0xE7 }, // #5 OK 01/06/2021
  { 0x28, 0xAA, 0x4D, 0xC0, 0x37, 0x14, 0x01, 0x9C }, // #6 OK 01/06/2021
  { 0x28, 0xAA, 0xCB, 0x64, 0x26, 0x13, 0x02, 0x01 }, // #7 OK 01/06/2021
  { 0x28, 0xAA, 0xC6, 0xEE, 0x37, 0x14, 0x01, 0x42 }, // #8 OK 01/06/2021
};

// Temperature Data from DS18B20 sensors, heat ON/OFF LOW/HIGH
int oneWireData[7][2] = {
  {0, HIGH}, // Ambient (heat not used)
  {0, HIGH}, // Greenhouse
  {0, HIGH}, // Reservoir
  {0, HIGH}, // Radiator
  {0, HIGH}, // Bench 1
  {0, HIGH}, // Bench 2
  {0, HIGH}, // Bench 3
};

String oneWireDesc[7] = {
  "Ambient Temperature,"
  "Greenhouse Temperature,",
  "Reservoir Temperature,",
  "Radiator Temperature,",
  "Bench 1 Temperature,",
  "Bench 2 Temperature,",
  "Bench 3 Temperature,"
};

int coolingStage = 0; // Current cooling stage
int pinState[16]; // Array of pin output states corresponds with pins 31-45
// array position 0 unused
// pin 46, and relay 16 unused

// Set heating and cooling temperatures in degrees F
int setPointArray[12];


void setup() {
  Serial.begin(9600);
  //Setup DHT Sensors
  for (auto& sensor : dht) {
    sensor.begin();
  }
  // Setup pin 9 for ds18b20 sensors
  pinMode(9, INPUT);
  //DS18B20 Sensor Setup
  sensors.begin();

  //Setup OUTPUT pins and initialize is HIGH state (relay OFF)
  for (int i = 0; i < 16; i++) {
    pinState[i] = HIGH;
  }
  writePins();

  setPointArray[0] = 65; // Cooling Stage -1
  setPointArray[1] = 78; // Cooling Stage 0
  setPointArray[2] = 80; // Cooling Stage 1
  setPointArray[3] = 85; // Cooling Stage 2
  setPointArray[4] = 90; // Cooling Stage 3
  setPointArray[5] = 55; // Humidity Min
  setPointArray[6] = 60; // Humidity Max

  printHeadings();
}

void loop()
{
  checkSerialInput();
  // Sensor measurements
  if (millis() - loopTime > loopInterval) {
    loopTime = millis();
    measureTemps();
    setCoolingStage();
    fanSpeed();
    mistingRelay();
    setHeat();
    writePins();

    /*
      // Checking pin states
      for (int i = 1; i < 16; i++)
      {
        Serial.print(pinState[i]);
      }
      Serial.println();
    */
  }
}

// Print the initial data headings in serial output
void printHeadings() {
  // DHT Sensor heading
  for (int i = 0; i < 3; i++) {
    Serial.print(dhtDataDesc[i][0]);
    Serial.print(dhtDataDesc[i][1]);
  }
  // DS18B20 heading
  Serial.print(",");
  for (int i = 0; i < 7; i++) {
    Serial.print(oneWireDesc[i]);
  }
  Serial.println();
}

void checkSerialInput() {
  // If any serial data is sent by computer, reprint headings
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    if (incomingByte == 10) {
      printHeadings();
    }
  }
}

void measureTemps() {
  //Loop through each DHT22 sensor
  for (int i = 0; i < 3; i++) {
    //Read each DHT Sensor temp and humidity and write to array
    float t = dht[i].readTemperature(true);
    float h = dht[i].readHumidity();

    if ( isnan(t)) {
      dhtData[i][0] = 999;
    }
    else {
      dhtData[i][0] = t;
    }
    if ( isnan(h)) {
      dhtData[i][1] = 999;
    }
    else {
      dhtData[i][1] = h;
    }

    // Print temp and humidity to serial
    Serial.print(dhtData[i][0]);
    Serial.print(", ");
    Serial.print(dhtData[i][1]);
    Serial.print(", ");
  }
  // DS18B20 Sensor reading
  sensors.requestTemperatures();
  Serial.print(",");
  // Loop through all DS18B20 sensors
  for (int i = 0; i < 7; i++) {
    // Get temps and write to array
    float t = sensors.getTempF(oneWireAddress[i]);
    oneWireData[i][0] = t;

    // Print temps to serial
    Serial.print(oneWireData[i][0]);
    Serial.print(",");
  }
  Serial.println("");
}


void setCoolingStage() {
  int dhtTemp = dhtData[1][0];
  int dsTemp = oneWireData[1][0];
  // Error checking data, turn off greenhouse if both temperatures are errors
  int temp = -999;

  if (dhtTemp > -50 && dhtTemp < 200) {
    temp = dhtTemp;
  }
  else if (dsTemp > -50 && dsTemp < 200) {
    temp = dsTemp;
  }
  else {
    coolingStage = 0;
    Serial.println("Both greenhouse sensors out of range; abort setting heat/cool stage");
    return;
  }
  //Serial.print(temp);
  //Serial.print(" deg f ");
  //Serial.print("Cooling Stage: ");
  //Serial.println(coolingStage); // Temporary check temp is correct

  //Heat ON
  if (temp <= setPointArray[0]) {
    coolingStage = -1;
    return;
  }
  // Heat OFF Cool OFF
  if (temp > setPointArray[0] || temp < setPointArray[2]) {
    coolingStage = 0;
  }
  // Cool ON Stage 1
  if (temp >= setPointArray[2]) {
    coolingStage = 1;
  }
  // Cool ON Stage 2
  if (temp >= setPointArray[3]) {
    coolingStage = 2;
  }
  // Cool ON Stage 3
  if (temp >= setPointArray[4]) {
    coolingStage = 3;
  }
  Serial.print("CoolingStage: ");
  Serial.println(coolingStage);
}

void fanSpeed() {
  // Cooling Stage 0 or -1; No cooling, Turn OFF evap cooler, water pump, and vent
  if (coolingStage <= 0) {
    // Turn OFF fans, water pump, and close vents
    for (int i = 1; i <= 8; i++) {
      pinState[i] = HIGH;
    }
    return;
  }

  // Anything else start cooling
  pinState[8] = LOW; // Exhaust relay 2 ON
  pinState[7] = LOW; // Exhause relay 1 ON
  pinState[6] = LOW; // Evaporative cooling water pump ON
  pinState[5] = HIGH; // Turn OFF HI fan speed
  pinState[4] = HIGH; // Turn OFF MED fan speed
  pinState[3] = HIGH; // Turn OFF LOW fan speed
  pinState[2] = HIGH; // Turn OFF HI Intake Fan
  //pinState[1] = HIGH; // Turn OFF misters, controlled by mistingRelay func

  // Changed to turn fan on HI on all setttings to avoid issues starting fan at lower speed
  // SetPoint 1 - Low fan setting
  if (coolingStage == 1) {
    //pinState[3] = LOW; // Turn ON LOW fan speed
    pinState[5] = LOW; // Turn ON HI fan speed
    Serial.println("Evap Cooler Low");
  }
  // SetPoint 2 - MED fan setting
  if (coolingStage == 2) {
    //pinState[4] = LOW; // Turn ON MED fan speed
    pinState[5] = LOW; // Turn ON HI fan speed
    Serial.println("Evap Cooler Med");
  }
  // SetPoint 3 - HI fan setting
  if (coolingStage == 3) {
    pinState[5] = LOW; // Turn ON HI fan speed
    pinState[2] = LOW; // Turn ON HI Intake Fan
    Serial.println("Evap Cooler Hi");

  }
}

void mistingRelay() {
  int h = dhtData[1][1];
  int hMin = setPointArray[5];
  int hMax = setPointArray[6];

  if (h <= 0 || h > 100) {
    pinState[1] = HIGH; // Turn OFF misters
    Serial.println("Humidity data out of range; aborting mist function");
    return;
  }

  // If cooling stage is less than 3 turn OFF misters
  if (coolingStage < 3) {
    pinState[1] = HIGH;
    //Serial.println(" ");
    return;
  }
  // If humidity is less than the limit turn ON misters
  if (h <= hMin) {
    pinState[1] = LOW;
    Serial.println("Misters Turned ON");
    return;
  }
  // If humidity is over the max turn OFF misters
  if (h >= hMax) {
    pinState[1] = HIGH;
    Serial.println("Misters Turned OFF");
    return;
  }
}


// Set heat ON/OFF for each of the 6 heat zones with corresponding DS18B20 sensors
void setHeat() {

  if (coolingStage > -1) {
    heaterTime = millis(); // Reset heat timer since no longer heating
    Serial.println("Heat mode not enabled, heater timer reset");
    for (int i = 9; i <= 15; i++) { // Cycle through and turn off all heat pumps and actuators
      pinState[i] = HIGH;
      return; // Exit out of function without turning on any heat
    }
  }
  // ms since heat reset
  tempTime = millis() - heaterTime;

  // Reset timer after heatOff cooldown exceeded
  if (tempTime > heatCooldown) {
    heaterTime = millis();
    Serial.println("RESET heat Timer");
  }

  for (int i = 1; i <= 6; i++) { // Skipping pos 0, ambient temp
    int temp = oneWireData[i][0]; // Temp
    int heat = oneWireData[i][1]; // Heat ON/OFF
    int maxTemp = heatArr[i][0]; // Max Temp
    int minTemp = heatArr[i][1]; // Min Temp

    if (temp > maxTemp) {
      heat = HIGH; // Turn OFF heat
    }
    if (temp < minTemp) {
      heat = LOW; // Turn ON heat
    }
    // Error checking, turn OFF heat if DS18B20 errors
    if (temp < -50 || temp > 200) {
      //Serial.println("Abort heating, temp out of range on DS18b20 array position: ");
      //Serial.println(i);
      heat = HIGH;
    }
  }
  int greenhouse = oneWireData[1][1]; // greenhouse heating ON/OFF setting
  int reservoir = oneWireData[2][1]; // reservior actuator ON/OFF setting
  int radiator = oneWireData[3][1]; // radiator actuator ON/OFF setting


  // If greenhouse is not heating turn off radiator heating OR
  // If reservior needs to be headed turn off radiator heating
  if (greenhouse = HIGH || reservoir  == LOW ) {
    radiator = HIGH;
  }
  // Heat reservoir
  if (reservoir == LOW) {
    pinState[9] = LOW; // Turn on heater pump
    pinState[14] = LOW; // Turn ON reservoir actuator
    pinState[15] = HIGH; // Turn OFF radiator actuator
  }
  // Heat radiator
  if (radiator == LOW) {
    pinState[9] = LOW; // Turn on heater pump
    pinState[14] = HIGH; // Turn OFF reservoir actuator
    pinState[15] = LOW; // Turn ON radiator actuator
  }


  // HEATER COOLDOWN TIMER
  // If heater is in cooldown turn off heater pump
  if (tempTime >= heatOn && tempTime <= heatCooldown) {
    pinState[9] = HIGH;
    Serial.println("Cooldown, Heat Pump OFF");
  }
  else if (pinState[9] == LOW) {
    Serial.println("Heat Pump ON");
  }

  // Cycle through bench temps and turn on circ pump and actuator when needed
  pinState[10] = HIGH; // Initially turn OFF circ pump, turn ON with actuators
  // TODO: add greenhouse, reservoir and radiator heating to this loop
  for (int i = 4; i <= 6; i++) {
    int heatSetting = oneWireData[i][1];
    int pin = i + 7; // Actuator pins are 11-13
    if (heatSetting == LOW) {
      pinState[pin] = LOW; // Actuator ON
      pinState[10] = LOW; // Circ pump ON if at least one actuator is on
    }
  }
}

// Write all the pin states on OUTPUT pins
void writePins() {
  for (int i = 1; i < 16; i++) {
    int pinLocation = i + 30; // Pin location corresponds to 31-46
    digitalWrite(pinLocation, pinState[i]);
  }
}
