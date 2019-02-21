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
const int analogPins[] = {A0,A1,A2,A3,A4};

/*
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
  for (int i = 2; i <= 12; i++) {
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
  heaterCount();
  writePins();
  printData();
  
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
    int x = 2;
    int y = 5;
    int z = 2;
    pin = pinState[x]; // Which pin to change state of pump
    setMax = setPoint[y]; // Match correct temp in array to the pump
    temp = data[i]; // Temp reading for area
    if (temp < setMax) {
      pin = LOW; // Turn ON pump
    }
    if (temp > setMax) {
      pin = HIGH; // Turn OFF pump
    }
    x++;
    y++;
    z++;
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
    data[10] = sensors.getTempF(t8);
    delay(1000);
}

// Write all the pin states on OUTPUT pins
void writePins() {
  for (int i = 2; i <= 13; i++) {
    digitalWrite(i, pinState[i]); 
  }
}

// Water heater shutoff bypass, turn OFF reservoir heater pump for 5 seconds every 10 minutes
void heaterCount() {
  if (count >= 120) { // program has 2+2+1=5 seconds of delay in it, turn OFF pump every 120 program cycles
    digitalWrite(2, HIGH); // Turn OFF reservoir pump
    delay(5000); // Wait 5 seconds
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
}
