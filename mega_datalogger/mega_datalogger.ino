/* Greenhouse Greenhouse Powered by an Arduino MEGA
    Major Revision Start: 01/03/2020
*/
// Adafruit Sensor Library must be installed, DHT.H library is dependent on it, or sketch won't compile!
#include "DHT.h" //DHT11 sensor library
#include "OneWire.h" // Library needed for DS18B20 sensors
#include "DallasTemperature.h" // Library needed for DS18B20 sensors

//Setup serial input checking
int incomingByte = 0; // for incoming serial data

//DHT Setup
unsigned long loopTime = 0; // Loop timer
int loopInterval = 5000; // ms between loop measurement intervals
int dhtData[4][2]; // Temp and humidity data from sensors
String dhtDataDesc[4][2] = { {"Enclosure Temperature,", "Enclosure Humidity,"},
  {"Greenhouse Temperature,", "Greenhouse Humidity,"},
  {"Ambient Temperature,", "Ambient Humidity,"},
  {"Cooling Output Temp,", "Cooling Output Humidity, "}
};

#define enclosureDHT22 22
#define greenhouseDHT22 23
#define ambientDHT22 24
#define coolingDHT22 25

DHT dht[] = {
  {enclosureDHT22, DHT22},
  {greenhouseDHT22, DHT22},
  {ambientDHT22, DHT22},
  {coolingDHT22, DHT22},
};

// DS18B20 Sensor Setup
// Connect all DS18B20 to pin 30
OneWire oneWire(30);
DallasTemperature sensors(&oneWire);

// DS18B20 Sensor Addresses
uint8_t oneWireAddress[7][8] = {
  { 0x28, 0xAA, 0xC6, 0xEE, 0x37, 0x14, 0x01, 0x42 }, // #8 Greenhouse Temperature
  { 0x28, 0xAA, 0xCB, 0x64, 0x26, 0x13, 0x02, 0x01 }, // #7 Ambient Temperature
  { 0x28, 0xAA, 0x4D, 0xC0, 0x37, 0x14, 0x01, 0x9C }, // #6 Reservoir
  { 0x28, 0xA9, 0xAC, 0x0F, 0x30, 0x14, 0x01, 0x0B }, // #5 Radiator
  // { 0x28, 0xAA, 0x57, 0xAF, 0x1A, 0x13, 0x02, 0x5F }, // #4 defective
  { 0x28, 0xAA, 0x3C, 0xF9, 0x37, 0x14, 0x01, 0xE7 }, // #3 Bench 1
  { 0x28, 0xAA, 0xE8, 0xBD, 0x37, 0x14, 0x01, 0x48 }, // #2 Bench 2
  { 0x28, 0xAA, 0xC4, 0x13, 0x1B, 0x13, 0x02, 0x8E }, // #1 Bench 3
};

// Temperature Data from DS18B20 sensors
int oneWireData[7];

String oneWireDesc[7] = {
  "Greenhouse Temperature,",
  "Ambient Temperature,",
  "Reservoir Temperature,",
  "Radiator Temperature,",
  "Bench 1 Temperature,",
  "Bench 2 Temperature,",
  "Bench 3 Temperature,"
};

int coolingStage = 0; // Current cooling stage
int pinState[16]; // Array of pin output states corresponds with pins 31-46

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

  setPointArray[0] = 65; // Cooling Stage -1
  setPointArray[1] = 78; // Cooling Stage 0
  setPointArray[2] = 80; // Cooling Stage 1
  setPointArray[3] = 85; // Cooling Stage 2
  setPointArray[4] = 90; // Cooling Stage 3
  setPointArray[5] = 90; // Min Res Temp
  setPointArray[6] = 95; // Max Res Temp
  setPointArray[7] = 72; // Bench 1 Temp
  setPointArray[8] = 72; // Bench 2 Temp
  setPointArray[9] = 72; // Bench 3 Temp
  setPointArray[10] = 55; // Humidity Min
  setPointArray[11] = 60; // Humidity Max

  printHeadings();
}

void loop()
{
  checkSerialInput();
  // Sensor measurements
  if (millis() - loopTime > loopInterval) {
    loopTime = millis();
    measureTemps();
    //Pass both greenhouse temp readings to set cooling stage func
    setCoolingStage(dhtData[1][0], oneWireData[0]);
    fanSpeed();
    mistingRelay();
    writePins();
  }
}

// Print the initial data headings in serial output
void printHeadings() {
  // DHT Sensor heading
  for (int i = 0; i < 4; i++) {
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
  for (int i = 0; i < 4; i++) {
    //Read each DHT Sensor temp and humidity and write to array
    dhtData[i][0] = dht[i].readTemperature(true);
    dhtData[i][1] = dht[i].readHumidity();
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
    oneWireData[i] = sensors.getTempF(oneWireAddress[i]);
    // Print temps to serial
    Serial.print(oneWireData[i]);
    Serial.print(",");
  }
  Serial.println("");
}


int setCoolingStage(int dhtTemp, int dsTemp) {
  // Error checking data, turn off greenhouse if both temperatures are errors
  int temp;
  if (dhtTemp == 0 && dsTemp == -196) {
    coolingStage = 0;
    return;
  }
  // If DHT22 temp is 0, use DS18B20 temperature incase it is an error
  if (dhtTemp == 0) {
    temp = dsTemp;
  }
  // Use DHT22 temperature for all other cases
  else temp = dhtTemp;

  Serial.println(temp); // Temporary check temp is correct

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
};

void fanSpeed() {
  // Set Point 0 - No heating or cooling - Turn OFF evap cooler, water pump, and vent
  if (coolingStage <= 0) {
    // Turn OFF fans, water pump, and close vents
    for (int i = 3; i <= 8; i++) {
      pinState[i] = HIGH;
    }
  }

  // Anything else start cooling
  else {
    pinState[8] = LOW; // Exhaust relay 2 ON
    pinState[7] = LOW; // Exhause relay 1 ON
    pinState[6] = LOW; // Evaporative cooling water pump ON
    pinState[5] = HIGH; // Turn OFF HI fan speed
    pinState[4] = HIGH; // Turn OFF MED fan speed
    pinState[3] = HIGH; // Turn OFF LOW fan speed
  }
  // Changed to turn fan on HI on all setttings to avoid issues starting fan at lower speed
  // SetPoint 1 - Low fan setting
  if (coolingStage == 1) {
    //pinState[3] = LOW; // Turn ON LOW fan speed
    pinState[5] = LOW; // Turn ON HI fan speed
  }
  // SetPoint 2 - MED fan setting
  if (coolingStage == 2) {
    //pinState[4] = LOW; // Turn ON MED fan speed
    pinState[5] = LOW; // Turn ON HI fan speed
  }
  // SetPoint 3 - HI fan setting
  if (coolingStage == 3) {
    pinState[5] = LOW; // Turn ON HI fan speed
  }
}

void mistingRelay() {
  int h = dhtData[1][1];
  int hMin = setPointArray[10];
  int hMax = setPointArray[11];
  
  // If temp is nan then turn off HVAC
  if (h == 0) {
    Serial.println("Failed to read humidity from DHT sensor, cannot activate misters until corrected!");
    pinState[1] = HIGH;
    return;
  }
  
  // If cooling stage is less than 3 turn OFF misters
  if(coolingStage < 3) {
    pinState[1] = HIGH;
    return;
  }
  // If humidity is less than the limit turn ON misters
  if(h <= hMin) {
    pinState[1] = LOW;
    Serial.println("Misters Turned ON");
    return;
  }
  // If humidity is over the max turn OFF misters
  if(h >= hMax) {
    pinState[1] = HIGH;
    Serial.println("Misters Turned OFF");
    return;
  }
}



// Write all the pin states on OUTPUT pins
void writePins() {
  for (int i = 0; i <= 15; i++) {
    int pinLocation = i + 31; // Pin location corresponds to 31-46
    digitalWrite(pinLocation, pinState[i]);
  }
}
