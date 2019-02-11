/*
   Greenhouse Heating and Cooling Automation Program
   Created 2018 12 21
   Nick Blodgett
*/
#include "DHT.h" //DHT11 sensor library
#include "OneWire.h" // Library needed for DS18B20 sensors
#include "DallasTemperature.h" // Library needed for DS18B20 sensors

// Assign all pin outputs
const int lowFanPin = 12; // Low speed cooling fan relay output
const int medFanPin = 11; // Medium speed cooling fan relay output
const int highFanPin = 10; // High speed cooling fan relay output
const int coolingPumpPin = 9; // Evaporative cooling water pump relay output
const int actuator1Pin = 8; // Exhaust vent actuator 1 relay output
const int actuator2Pin = 7; // Exhaust vent actuator 1 relay output
const int heatPump4Pin = 6; // Hot water pump table 1 relay output
const int heatPump3Pin = 5; // Hot water pump table 2 relay output
const int heatPump2Pin = 4; // Hot water pump hydroponic reservoir relay output
const int heatPump1Pin = 3; // Hot water pump hydroponic reservoir relay output
const int heatPump0Pin = 2; // Main hot water reservoir relay output

//Assign all pin inputs
const int boardTempPin = A0;
const int indoorTempPin = A1;
const int outdoorTempPin = A2;
const int tempZonePin = A3;
const int lowWaterPin = A4;

// Init DHT Sensors
#define DHTTYPE1 DHT11

DHT indoorSensor(indoorTempPin, DHTTYPE1);
DHT outdoorSensor(outdoorTempPin, DHTTYPE1);

// DS18B20 Sensor Addresses
DeviceAddress tempZone0 = { 0x28, 0xAA, 0xA0, 0x3D, 0x1B, 0x13, 0x02, 0xE5 };
DeviceAddress tempZone1 = { 0x28, 0xA9, 0xAC, 0x0F, 0x30, 0x14, 0x01, 0x0B };
DeviceAddress tempZone2 = { 0x28, 0xAA, 0xDE, 0xB3, 0x1B, 0x13, 0x02, 0xE6 };
DeviceAddress tempZone3 = { 0x28, 0xAA, 0x57, 0xAF, 0x1A, 0x13, 0x02, 0x5F };
DeviceAddress tempZone4 = { 0x28, 0xAA, 0xC4, 0x13, 0x1B, 0x13, 0x02, 0x8E };


// DS18B20 Sensor Setup
// Connect all DS18B20 to tempZonePin
OneWire oneWire(tempZonePin);
DallasTemperature sensors(&oneWire);
int zoneTempF;
int zoneTempC;

// Initialize Variables
int indoorTemp = 0;
int indoorRH = 0;
int outdoorTemp = 0;
int outdoorRH = 0;
int setPoint = 0; // Cooling Stage (0-3)

// Array of zone temperature sensors
int zone[5];


void setup() {
  Serial.begin(9600);

  //Setup Pin I/O
  pinMode(lowFanPin, OUTPUT);
  pinMode(medFanPin, OUTPUT);
  pinMode(highFanPin, OUTPUT);
  pinMode(coolingPumpPin, OUTPUT);
  pinMode(actuator1Pin, OUTPUT);
  pinMode(actuator2Pin, OUTPUT);
  pinMode(heatPump0Pin, OUTPUT);
  pinMode(heatPump1Pin, OUTPUT);
  pinMode(heatPump2Pin, OUTPUT);
  pinMode(heatPump3Pin, OUTPUT);
  pinMode(heatPump4Pin, OUTPUT);
  pinMode(boardTempPin, INPUT);
  pinMode(indoorTempPin, INPUT);
  pinMode(outdoorTempPin, INPUT);
  pinMode(tempZonePin, INPUT);
  pinMode(lowWaterPin, INPUT);

  // Set all OUTPUT pins to initial HIGH state
  digitalWrite(lowFanPin, HIGH);
  digitalWrite(medFanPin, HIGH);
  digitalWrite(highFanPin, HIGH);
  digitalWrite(coolingPumpPin, HIGH);
  digitalWrite(actuator1Pin, HIGH);
  digitalWrite(actuator2Pin, HIGH);
  digitalWrite(heatPump0Pin, HIGH);
  digitalWrite(heatPump1Pin, HIGH);
  digitalWrite(heatPump2Pin, HIGH);
  digitalWrite(heatPump3Pin, HIGH);
  digitalWrite(heatPump4Pin, HIGH);
  
  // DHT11 Sensor Setup
  indoorSensor.begin();
  outdoorSensor.begin();
}

void loop() {
  readDHTsensor();
  fanSpeed();

  // Request DS18B20 Zone heating temperatures and assign to variable
  sensors.requestTemperatures(); // Send the command to get temperatures
  delay(2000);
  zone[0] = sensors.getTempF(tempZone0);
  zone[1] = sensors.getTempF(tempZone1);
  zone[2] = sensors.getTempF(tempZone2);
  zone[3] = sensors.getTempF(tempZone3);
  zone[4] = sensors.getTempF(tempZone4);
  Serial.println("Temperature Zones: ");
  Serial.println(zone[0]);
  Serial.println(zone[1]);
  Serial.println(zone[2]);
  Serial.println(zone[3]);
  Serial.println(zone[4]);
  
  // Zone 1 is heat reservoir, turn on heat at 74, turn off at 90
  int x1 = zone[0];
  int pin1 = heatPump0Pin;
  // If temperature sensor reads after init value
  if (x1 > 0) {
    // Turn on reservoir heat
    if (x1 < 74) {
      digitalWrite(pin1, LOW);
    }
    // Turn off reservoir heat
    if (x1 > 90) {
      digitalWrite(pin1, HIGH);
    }

  }
  // If zone temperature is above the initialized temperature (zero F) and below x degrees F, turn on heat
  int x2 = zone[1];
  int pin2 = heatPump1Pin;
  if (x2 > 0 && x2 < 72) {
    digitalWrite(pin2, LOW);
  }
  // Else turn off zone heating
  else {
    digitalWrite(pin2, HIGH);
  }
  // If zone temperature is above the initialized temperature (zero F) and below x degrees F, turn on heat
  int x3 = zone[2];
  int pin3 = heatPump2Pin;
  if (x3 > 0 && x3 < 72) {
    digitalWrite(pin3, LOW);
  }
  // Else turn off zone heating
  else {
    digitalWrite(pin3, HIGH);
  }
  // If zone temperature is above the initialized temperature (zero F) and below x degrees F, turn on heat
  int x4 = zone[3];
  int pin4 = heatPump3Pin;
  if (x4 > 0 && x4 < 72) {
    digitalWrite(pin4, LOW);
  }
  // Else turn off zone heating
  else {
    digitalWrite(pin4, HIGH);
  }
  // If zone temperature is above the initialized temperature (zero F) and below x degrees F, turn on heat
  int x5 = zone[4];
  int pin5 = heatPump4Pin;
  if (x5 > 0 && x5 < 72) {
    digitalWrite(pin5, LOW);
  }
  // Else turn off zone heating
  else {
    digitalWrite(pin5, HIGH);

  }
}

void readDHTsensor() {
  indoorTemp = indoorSensor.readTemperature(true); // Read temp in deg f
  indoorRH = indoorSensor.readHumidity();

  // Wait a two seconds between measurements.
  delay(2000);

  outdoorTemp = outdoorSensor.readTemperature(true); // Read temp in deg f
  outdoorRH = outdoorSensor.readHumidity();
  delay(2000);

  Serial.print("Indoor Temp: ");
  Serial.println(indoorTemp);
  Serial.print("Indoor RH: ");
  Serial.println(indoorRH);
}

// Set Cooling Stage
void fanSpeed() {
  //If temperature sensor does not read, exit cooling function without changing setpoint
  if (indoorTemp == 0 && indoorRH == 0) {
    Serial.println("Temperature Error, Cooling not functioning!");
    return;
  }

  // Condition: temperature is less than 75 deg f, Cooling system OFF
  if (indoorTemp < 75) {
    setPoint = 0;
    digitalWrite(lowFanPin, HIGH);
    digitalWrite(medFanPin, HIGH);
    digitalWrite(highFanPin, HIGH);
    digitalWrite(coolingPumpPin, HIGH); // Turn cooling pump OFF
    digitalWrite(actuator1Pin, HIGH); // Close vent or turn OFF exhaust fan
    digitalWrite(actuator2Pin, HIGH); // Close vent or turn OFF exhaust fan
  }
  // Condition: if temperature is greater than 80 and cooling not on, turn on low
  if (indoorTemp > 80 && setPoint < 1) {
    setPoint = 1;
    digitalWrite(lowFanPin, LOW);
    digitalWrite(medFanPin, HIGH);
    digitalWrite(highFanPin, HIGH);
    digitalWrite(coolingPumpPin, LOW); // Turn cooling pump ON
    digitalWrite(actuator1Pin, LOW); // Open vent or turn ON exhaust fan
    digitalWrite(actuator2Pin, LOW); // Open vent or turn ON exhaust fan
  }
  // Condition: If temperature is greater than 85 and cooling is off or low, turn on medium
  if (indoorTemp > 85 && setPoint < 2) {
    setPoint = 2;
    digitalWrite(lowFanPin, HIGH);
    digitalWrite(medFanPin, LOW);
    digitalWrite(highFanPin, HIGH);
    digitalWrite(coolingPumpPin, LOW); // Turn cooling pump ON
    digitalWrite(actuator1Pin, LOW); // Open vent or turn ON exhaust fan
    digitalWrite(actuator2Pin, LOW); // Open vent or turn ON exhaust fan
  }
  // Condition: If temperature is greater than 90 and cooling is off, low, or med, turn on high
  if (indoorTemp > 90 && setPoint < 3) {
    setPoint = 3;
    digitalWrite(lowFanPin, HIGH);
    digitalWrite(medFanPin, HIGH);
    digitalWrite(highFanPin, LOW);
    digitalWrite(coolingPumpPin, LOW); // Turn cooling pump ON
    digitalWrite(actuator1Pin, LOW); // Open vent or turn ON exhaust fan
    digitalWrite(actuator2Pin, LOW); // Open vent or turn ON exhaust fan
  }
  
  Serial.print("Set Point: ");
  Serial.println(setPoint);
}
