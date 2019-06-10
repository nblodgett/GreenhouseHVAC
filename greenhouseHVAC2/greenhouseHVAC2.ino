/*
    Greenhouse Heating and Cooling Automation
    Rev 2
    Created 2019 02 15
*/

#include "DHT.h" //DHT11 sensor library
// Adafruit Sensor Library must be installed, DHT.H library is dependent on it, or sketch won't compile!
#include "OneWire.h" // Library needed for DS18B20 sensors
#include "DallasTemperature.h" // Library needed for DS18B20 sensors


int data[11]; // Array of temperature and humidity readings
int pinState[14]; // Array of pin output state
int setPointArray[11]; // Array of set point temperatures
//int ds18B20[6]; //DS18B20 sensor addresses, array position equals label number
int coolingStage = 0; // init cooling stage and assign 0
int count = 0; // Var used for water heater reset to bypass extended use shutdown
const int analogPins[] = {A0, A1, A2, A3, A4};

/*
  // Assign all pin outputs
  const in misterPin = [13]; // Misting relay output
  const int lowFanPin = [12]; // Low speed cooling fan relay output
  const int medFanPin = [11]; // Medium speed cooling fan relay output
  const int highFanPin = [10]; // High speed cooling fan relay output
  const int coolingPumpPin = [9]; // Evaporative cooling water pump relay output
  const int actuator2Pin = [8]; // Exhaust vent actuator 2 relay output
  const int actuator1Pin = [7]; // Exhaust vent actuator 1 relay output
  const int heatPump5Pin = [6]; // Hot water pump table 2 relay output
  const int heatPump4Pin = [5]; // Hot water pump table 1 relay output
  const int heatPump3Pin = [4]; // Hot water pump hydroponic reservoir relay output
  const int heatPump2Pin = [3]; // Hot water pump hydroponic reservoir relay output
  const int heatPump1Pin = [2]; // Main hot water reservoir relay output
*/

//Assign all pin inputs
//const int boardTempPin = analogPins[0]; // Board temperature sensor
const int indoorTempPin = analogPins[1]; // Greenhouse temperature DHT sensor
const int outdoorTempPin = analogPins[2]; // Outdoor ambient temperature DHT sensor
const int tempZonePin = analogPins[3]; // All DS18B20 sensors
//const int lowWaterPin = analogPins[4]; // Evaporative cooler low water sensor



// Init DHT Sensors
#define DHTTYPE1 DHT11
DHT indoorSensor(indoorTempPin, DHTTYPE1);
DHT outdoorSensor(outdoorTempPin, DHTTYPE1);

// DS18B20 Sensor Setup
// Connect all DS18B20 to tempZonePin
OneWire oneWire(tempZonePin);
DallasTemperature sensors(&oneWire);
// DS18B20 Sensor Addresses
// tempZone devices 4 and 5 damaged, need replacement
DeviceAddress t8 = { 0x28, 0xAA, 0xC6, 0xEE, 0x37, 0x14, 0x01, 0x42 };
DeviceAddress t7 = { 0x28, 0xAA, 0xCB, 0x64, 0x26, 0x13, 0x02, 0x01 };
DeviceAddress t6 = { 0x28, 0xAA, 0x4D, 0xC0, 0x37, 0x14, 0x01, 0x9C }; // Assigned Ambient
DeviceAddress t5 = { 0x28, 0xAA, 0x3C, 0xF9, 0x37, 0x14, 0x01, 0xE7 }; // Assigned Hyrdro reservoir 2 (NFT)
DeviceAddress t4 = { 0x28, 0xAA, 0xE8, 0xBD, 0x37, 0x14, 0x01, 0x48 }; // Assigned Hydro reservoir 1 (Dutch Bucket)
DeviceAddress t3 = { 0x28, 0xA9, 0xAC, 0x0F, 0x30, 0x14, 0x01, 0x0B }; // Assigned second bench
DeviceAddress t2 = { 0x28, 0xAA, 0x57, 0xAF, 0x1A, 0x13, 0x02, 0x5F }; // Assigned first bench
DeviceAddress t1 = { 0x28, 0xAA, 0xC4, 0x13, 0x1B, 0x13, 0x02, 0x8E }; // Assigned main reservoir

void setup() {
  Serial.begin(9600);

  /*
    //Setup Pin I/O
    pinMode(boardTempPin, INPUT);
    pinMode(indoorTempPin, INPUT);
    pinMode(outdoorTempPin, INPUT);
    pinMode(tempZonePin, INPUT);
    pinMode(lowWaterPin, INPUT);
  */

  // Setup all analog pins for INPUT
  for (int i = 0; i <= 4; i++) {
    pinMode(analogPins[i], INPUT);
  }

  //Setup OUTPUT pins and initialize is HIGH state (relay OFF)
  for (int i = 2; i <= 13; i++) {
    pinMode(i, OUTPUT);
    pinState[i] = HIGH;
  }
  writePins();

  // DHT11 Sensor Setup
  indoorSensor.begin();
  outdoorSensor.begin();

  //DS18B20 Sensor Setup
  sensors.begin();

  // Set heating and cooling temperatures in degrees F
  setPointArray[0] = 55; // Cooling Stage -1
  setPointArray[1] = 78; // Cooling Stage 0
  setPointArray[2] = 80; // Cooling Stage 1
  setPointArray[3] = 85; // Cooling Stage 2
  setPointArray[4] = 90; // Cooling Stage 3
  setPointArray[5] = 90; // Min Res Temp
  setPointArray[6] = 95; // Max Res Temp
  setPointArray[7] = 72; // Bench 1 Temp
  setPointArray[8] = 72; // Bench 2 Temp
  setPointArray[9] = 72; // Hydro Res 1 (NFT Channel) Temp
  setPointArray[10] = 72; // Hydro Res 2 (Bench 1 Res) Temp
}

void loop() {
  readDHT();
  readZoneTemps();
  setPoint();
  fanSpeed();
  heat();
  counter();
  writePins();
  printData();

  /* TESTING MIST SYTEM

    digitalWrite(13, LOW); // Turn ON misters
    Serial.println("MIST  ON");
    delay(5000); // Wait 5 seconds
    digitalWrite(13, HIGH); // Turn OFF misters
    Serial.println("MIST OFF");
    delay(5000); // Wait 5 seconds

  */

}

// Read and write DHT readings
void readDHT() {
  data[6] = indoorSensor.readTemperature(true); // Read temp in deg f
  data[7] = indoorSensor.readHumidity(); // Read humidity
  delay(2000); // Wait a two seconds between measurements.

  data[8] = outdoorSensor.readTemperature(true); // Read temp in deg f
  data[9] = outdoorSensor.readHumidity(); // read humidity
  delay(2000); // Wait a two seconds between measurements.
}

/* Basic HVAC logic is cooling stage only increases (fan speed only increase) until temp cools to setpoint 0, then turns OFF
   This avoids cooling fan constantly changing speeds with small temperature changes
   Heating will not engage until cooling stage is -1, heating and cooling cannot both be ON at one time
*/
void setPoint() {
  int indoorTemp = data[10]; // Read indoor temp and assign to var indoorTemp
  if (indoorTemp <= setPointArray[0]) {
    coolingStage = -1;
    return;
  }
  if (indoorTemp <= setPointArray[1]) {
    coolingStage = 0;
  }
  if (indoorTemp >= setPointArray[2] && coolingStage < 1) {
    coolingStage = 1;
  }
  if (indoorTemp >= setPointArray[3] && coolingStage < 2) {
    coolingStage = 2;
  }
  if (indoorTemp >= setPointArray[4] && coolingStage < 3) {
    coolingStage = 3;
  }
}

void fanSpeed() {
  // Set Point 0 - No heating or cooling - Turn OFF evap cooler, water pump, and vent
  if (coolingStage <= 0) {
    setPumpAndVent(HIGH);
    for (int i = 10; i <= 12; i++) {
      pinState[i] = HIGH;
    }
  }

  // SetPoint 1 - Low fan setting - Turn ON evap cooler, water pump and vent
  if (coolingStage == 1) {
    setPumpAndVent(LOW); // Turn ON pump and vent
    pinState[10] = HIGH; // Turn OFF HI fan speed
    pinState[11] = HIGH; // Turn OFF MED fan speed
    pinState[12] = LOW; // Turn ON LOW fan speed
  }

  // SetPoint 2 - MED fan setting - Turn ON evap cooler, water pump and vent
  if (coolingStage == 2) {
    setPumpAndVent(LOW);
    pinState[10] = HIGH; // Turn OFF HI fan speed
    pinState[11] = LOW; // Turn ON MED fan speed
    pinState[12] = HIGH; // Turn OFF LOW fan speed
  }

  // SetPoint 3 - HI fan setting - Turn ON evap cooler, water pump and vent
  if (coolingStage == 3) {
    setPumpAndVent(LOW);
    pinState[10] = LOW; // Turn ON HI fan speed
    pinState[11] = HIGH; // Turn OFF MED fan speed
    pinState[12] = HIGH; // Turn OFF LOW fan speed
  }
}

// Set cooling pump and vent to HIGH OR LOW by passing in HIGH OR LOW
int setPumpAndVent (int x) {
  for (int i = 7; i <= 9; i++) {
    pinState[i] = x;
  }
}

// Function that runs heating for  benches and reservoirs
void heat() {
  // Turn off heating and exit heat function if cooling is on
  if (coolingStage > -1) {
    for (int i = 2; i <= 6; i++) {
      pinState[i] = HIGH;
    }
    return;
  }

  int temp = data[1]; // Reservoir temp
  int setMin = setPointArray[5]; // Minimum res temp set point
  int setMax = setPointArray[6]; // Maximum res temp set point
  // Main res heating logic
  if (temp < setMin) {
    pinState[2] = LOW; // Turn ON pump
  }
  if (temp > setMax) {
    pinState[2] = HIGH; // Turn OFF pump
  }

  /* Heating Logic
     Loop through zone 2-5, and turn ON heat pump if temperature is less than set point
  */

  int pin = 3; // Pin of heat pump
  int set = 7; // Defined heat set point of zone in array
  int var = 2; // Measured temp of heated zone in array
  for (int i = 2; i <= 5; i++) {
    if (data[var] < setPointArray[set]) { // If measured temp is less than set temp
      pinState[pin] = LOW; // Turn ON heat pump
    }
    else {
      pinState[pin] = HIGH; // Turn OFF heat pump
      Serial.println("heat OFF");
    }
    pin++;
    set++;
    var++;
  }
}



// Read all zone temperatures (DS1820B sensor) and write to data array
void readZoneTemps() {
  sensors.requestTemperatures();
  data[1] = sensors.getTempF(t1);
  data[2] = sensors.getTempF(t2);
  data[3] = sensors.getTempF(t3);
  data[4] = sensors.getTempF(t4);
  data[5] = sensors.getTempF(t5);
  data[10] = sensors.getTempF(t6);
  delay(1000);

  // If temperature is outside of normal range or returns error, replace with value 999
  for (int i = 1; i <= 10; i++) {
    if (data[i] > 140 || data[i] < -40) {
      data[i] = 999;
    }
  }
}

// Write all the pin states on OUTPUT pins
void writePins() {
  for (int i = 2; i <= 13; i++) {
    digitalWrite(i, pinState[i]);
  }
}

// Water heater shutoff bypass, turn OFF reservoir heater pump for 60 seconds every 10 minutes
// Misting counter for cooling stages, curerntly set to all stages, turn on mist for 60 seconds every 10 minutes
// 10 minutes is roughly 120 cycles, 12 cycles a minute

void counter() {
  if (count >= 12) { // program has about 5 seconds of delay in it, turn OFF pump every 'count' program cycles

    if (coolingStage < 0) {
      digitalWrite(2, HIGH); // Turn OFF reservoir pump
      delay(5000); // Wait 5 seconds
    }
    if (coolingStage > 0) {
      digitalWrite(13, LOW); // Turn ON misters
      Serial.println("MIST  ON");
      delay(5000); // Wait 5 seconds
      digitalWrite(13, HIGH); // Turn OFF misters
      Serial.println("MIST  OFF");
    }
    count = 0; // Reset counter
  }
  count++;
}

// Print all the temperature and pin state data for
void printData() {
  for (int i = 1; i <= 10; i++) {
    Serial.print(data[i]);
    Serial.print(",");
  }
  for (int i = 2; i <= 12; i++) {
    Serial.print(pinState[i]);
    Serial.print(",");
  }
  Serial.println(" ");
  Serial.print("Cooling Stage: ");
  Serial.println(coolingStage);
  Serial.print("Count: ");
  Serial.println(count);
}
