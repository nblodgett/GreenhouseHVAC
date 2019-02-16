/*
    Greenhouse Heating and Cooling Automation
    Rev 2
    Created 2019 02 15
*/

#include "DHT.h" //DHT11 sensor library
// Adafruit Sensor Library must be installed, DHT.H library is dependent on it, or sketch won't compile!
#include "OneWire.h" // Library needed for DS18B20 sensors
#include "DallasTemperature.h" // Library needed for DS18B20 sensors


int data[10]; // Array of temperature and humidity readings
int pinState[14]; // Array of pin output state
int setPointArray[11]; // Array of set point temperatures
int ds18B20[6]; //DS18B20 sensor addresses, array position equals label number
int coolingStage = 0; // init cooling stage and assign 0

// Assign all pin outputs
const int lowFanPin = data[12]; // Low speed cooling fan relay output
const int medFanPin = data[11]; // Medium speed cooling fan relay output
const int highFanPin = data[10]; // High speed cooling fan relay output
const int coolingPumpPin = data[9]; // Evaporative cooling water pump relay output
const int actuator2Pin = data[8]; // Exhaust vent actuator 2 relay output
const int actuator1Pin = data[7]; // Exhaust vent actuator 1 relay output
const int heatPump5Pin = data[6]; // Hot water pump table 2 relay output
const int heatPump4Pin = data[5]; // Hot water pump table 1 relay output
const int heatPump3Pin = data[4]; // Hot water pump hydroponic reservoir relay output
const int heatPump2Pin = data[3]; // Hot water pump hydroponic reservoir relay output
const int heatPump1Pin = data[2]; // Main hot water reservoir relay output

//Assign all pin inputs
const int boardTempPin = A0; // Board temperature sensor
const int indoorTempPin = A1; // Greenhouse temperature DHT sensor
const int outdoorTempPin = A2; // Outdoor ambient temperature DHT sensor
const int tempZonePin = A3; // All DS18B20 sensors
const int zoneTempPin = A3;
const int lowWaterPin = A4; // Evaporative cooler low water sensor



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
DeviceAddress t5 = { 0x28, 0xAA, 0xA0, 0x3D, 0x1B, 0x13, 0x02, 0xE5 };
DeviceAddress t4 = { 0x28, 0xAA, 0xDE, 0xB3, 0x1B, 0x13, 0x02, 0xE6 };
DeviceAddress t3 = { 0x28, 0xA9, 0xAC, 0x0F, 0x30, 0x14, 0x01, 0x0B }; // Assigned second bench
DeviceAddress t2 = { 0x28, 0xAA, 0x57, 0xAF, 0x1A, 0x13, 0x02, 0x5F }; // Assigned first bench
DeviceAddress t1 = { 0x28, 0xAA, 0xC4, 0x13, 0x1B, 0x13, 0x02, 0x8E }; // Assigned main reservoir

void setup() {
  Serial.begin(9600);

  //Setup Pin I/O
  pinMode(boardTempPin, INPUT);
  pinMode(indoorTempPin, INPUT);
  pinMode(outdoorTempPin, INPUT);
  pinMode(tempZonePin, INPUT);
  pinMode(lowWaterPin, INPUT);

  //Setup OUTPUT pins and initialize is HIGH state (relay OFF)
  for (int i = 2; i <= 12; i++) {
    pinMode(i, OUTPUT);
    pinState[i] = HIGH;
  }
  writePins();
  
  // DHT11 Sensor Setup
  indoorSensor.begin();
  outdoorSensor.begin();

  // Set heating and cooling temperatures
  setPointArray[0] = 70; // Cooling Stage -1
  setPointArray[1] = 75; // Cooling Stage 0
  setPointArray[2] = 80; // Cooling Stage 1
  setPointArray[3] = 85; // Cooling Stage 2
  setPointArray[4] = 90; // Cooling Stage 3
  setPointArray[5] = 125; // Min Res Temp
  setPointArray[6] = 130; // Max Res Temp
  setPointArray[7] = 74; // Bench 1 Temp
  setPointArray[8] = 74; // Bench 2 Temp
  setPointArray[9] = 74; // Hydro Res 1 (NFT Channel) Temp
  setPointArray[10] = 74; // Hydro Res 2 (Bench 1 Res) Temp
}

void loop() {
  readDHT();
  readZoneTemps();
  setPoint();
  fanSpeed();
  heat();
  writePins();
  printData();
  delay(1000);
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
 * This avoids cooling fan constantly changing speeds with small temperature changes
 * Heating will not engage until cooling stage is -1, heating and cooling cannot both be ON at one time
 */
 
void setPoint() {
  int indoorTemp = data[6]; // Read indoor temp and assign to var indoorTemp
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
      pinState[i]= HIGH;
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
  for (int i = 7; i <=9; i++) {
    pinState[i] = x;
  }
}

void heat() {
  int temp = data[6]; // Actual res temp
  int setMin = setPointArray[5]; // Minimum res temp set point
  int setMax = setPointArray[6]; // Maximum res temp set point
  int pin = pinState[2]; // Pin to change state  
  // Main res heating logic
  if (temp <= setMin) {
    pin = LOW; // Turn ON pump
  }
  if (temp >= setMax) {
    pin = HIGH; // Turn OFF pump
  }
  
  // Heating logic for benches and hydro reservoirs
  for(int i = 1; i <= 5; i++) {
    pin = pinState[i + 1]; // Which pin to change state of pump
    setMax = setPoint[i + 4]; // Match correct temp in array to the pump
    temp = data[i +1]; // Temp reading for area
    if (temp < setMax) {
      pin = LOW; // Turn ON pump
    }
    if (temp > setMax) {
      pin = HIGH; // Turn OFF pump
    }
  }
}

// Read all zone temperatures (DS1820B sensor)
void readZoneTemps() {
    data[1] = sensors.getTempF(t1);
    data[2] = sensors.getTempF(t2);
    data[3] = sensors.getTempF(t3);
    data[4] = sensors.getTempF(t4);
    data[5] = sensors.getTempF(t5);
}

// Write all the pin states on OUTPUT pins
void writePins() {
  for (int i = 2; i <= 13; i++) {
    digitalWrite(i, pinState[i]); 
  }
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
}
