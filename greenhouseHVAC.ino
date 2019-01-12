/*
   Greenhouse Heating and Cooling Automation Program
   Created 2018 12 21
   Nick Blodgett
*/
#include "DHT.h" //DHT11 sensor library
//#include "OneWire.h" // Library needed for DS18B20 sensors
//#include "DallasTemperature.h" // Library needed for DS18B20 sensors

// Assign all pin outputs 
const int lowFanPin = 12; // Low speed cooling fan relay output
const int medFanPin = 11; // Medium speed cooling fan relay output
const int highFanPin = 10; // High speed cooling fan relay output
const int coolingPumpPin = 9; // Evaporative cooling water pump relay output
const int actuator1Pin = 8; // Exhaust vent actuator 1 relay output
const int actuator2Pin = 7; // Exhaust vent actuator 1 relay output
const int heatPump1Pin = 6; // Hot water pump table 1 relay output
const int heatPump2Pin = 5; // Hot water pump table 2 relay output
const int heatPump3Pin = 4; // Hot water pump hydroponic reservoir relay output
const int heatPump4Pin = 3; // Hot water pump hydroponic reservoir relay output
const int heatPump5Pin = 2; // Main hot water reservoir relay output

//Assign all pin inputs
const int boardTempPin = A0;
const int indoorTempPin = A1;
const int outdoorTempPin = A2;
const int zoneTempPin = A3;
const int lowWaterPin = A4;

// Init DHT Sensors
#define DHTTYPE1 DHT11

DHT indoorSensor(indoorTempPin, DHTTYPE1);
DHT outdoorSensor(outdoorTempPin, DHTTYPE1);

//DHT dht(ghTempRh, DHTTYPE);
//DHT dht(ghTempRh, DHTTYPE);

/*
// DS18B20 Sensor
OneWire oneWire(tableTemp);
DallasTemperature sensors (&oneWire);
int tableTempF;
int tableTempC;
*/

// Init Variables
int indoorTemp = 0;
int indoorRH = 0;
int outdoorTemp = 0;
int outdoorRH = 0;
int setPoint = 0; // Cooling Stage (0-3)

void setup() {
  Serial.begin(9600);

  //Setup Pin I/O
  pinMode(lowFanPin, OUTPUT);
  pinMode(medFanPin, OUTPUT);
  pinMode(highFanPin, OUTPUT);
  pinMode(coolingPumpPin, OUTPUT);
  pinMode(actuator1Pin, OUTPUT);
  pinMode(actuator2Pin, OUTPUT);
  pinMode(heatPump1Pin, OUTPUT);
  pinMode(heatPump2Pin, OUTPUT);
  pinMode(heatPump3Pin, OUTPUT);
  pinMode(heatPump4Pin, OUTPUT);
  pinMode(heatPump5Pin, OUTPUT);
  pinMode(boardTempPin, INPUT);
  //pinMode(indoorTempPin, INPUT);
  //pinMode(outdoorTempPin, INPUT);    
  pinMode(zoneTempPin, INPUT);
  pinMode(lowWaterPin, INPUT);

  // DHT11 Sensor Setup
  indoorSensor.begin();
  outdoorSensor.begin();
}

void loop() {
  readDHTsensor();
  fanSpeed();
  setZoneHeat(heatPump1Pin);
  setZoneHeat(heatPump2Pin);
  setZoneHeat(heatPump3Pin);
  setZoneHeat(heatPump4Pin);
  setZoneHeat(heatPump5Pin);
  
  // Print temperature and RH for testing
  Serial.print(" Indoor Temperature: ");
  Serial.print(indoorTemp);
  Serial.print(" Indoor Humidity: ");
  Serial.println(indoorRH);
  Serial.print(" Outdoor Temperature: ");
  Serial.print(outdoorTemp);
  Serial.print(" Outdoor Humidity: ");
  Serial.println(outdoorRH);

  //Output cooling stage result for testing
  Serial.print("Cooling Stage: ");
  Serial.println(setPoint);
  Serial.print("Low: ");
  Serial.println(digitalRead(lowFanPin));
  Serial.print("Med: ");
  Serial.println(digitalRead(medFanPin));
  Serial.print("High: ");
  Serial.println(digitalRead(highFanPin));
  // Print cooling pump status for testing
  Serial.print("Cooling Pump: ");
  Serial.println(digitalRead(coolingPumpPin));
  // Print exhaust status for testing
  Serial.print("Exhaust: ");
  Serial.print(digitalRead(actuator1Pin));
  Serial.println(digitalRead(actuator2Pin));
  
}

void readDHTsensor() {
  // Wait a two seconds between measurements.
  delay(2000);
  
  indoorTemp = indoorSensor.readTemperature(true); // Read temp in deg f
  indoorRH = indoorSensor.readHumidity();
  
  // Wait a two seconds between measurements.
  delay(2000);
  
  outdoorTemp = outdoorSensor.readTemperature(true); // Read temp in deg f
  outdoorRH = outdoorSensor.readHumidity();
}

// Set Cooling Stage
void fanSpeed() {
  //If temperature sensor does not read, exit cooling function without changing setpoint
  if (indoorTemp == 0 && indoorRH == 0) {
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
}

// Set individual heat zones based on temperature
int setZoneHeat(int x) {
  // Ff greenhouse temperature is greater than zero f init value, and less than 70 deg f, turn on zone heating
  if (indoorTemp > 0 && indoorTemp < 70) {
    digitalWrite(x, LOW);
  }
  // Else turn off zone heating
  else {
    digitalWrite(x, HIGH);
  }
}
