/*
   Greenhouse Heating and Cooling Automation Program
   Created 2018 12 21
   Nick Blodgett
*/
#include "DHT.h" //DHT11 sensor library
//#include "OneWire.h" // Library needed for DS18B20 sensors
//#include "DallasTemperature.h" // Library needed for DS18B20 sensors

// Assign all pin I/O
const int lowFan = 12; // Low speed cooling fan relay output
const int medFan = 11; // Medium speed cooling fan relay output
const int highFan = 10; // High speed cooling fan relay output
const int coolingPump = 9; // Evaporative cooling water pump relay output
const int actuator1 = 8; // Exhaust vent actuator 1 relay output
//const int heatPump1 = 7; // hot water pump table 1 relay output
//const int heatPump2 = 6; // hot water pump table 2 relay output
//const int heatPump3 = 5; // hot water pump table 3 relay output
//const int heatPump4 = 4; // hot water pump hydroponic reservoir relay output
const int ghTempRh = 3; // Greenhouse temperature and RH sensor (DHT11) input
//const int tableTemp = 2; // Table/hydroponic reservoir temperature inputs (DS18B20) (qty 4)

// DHT11 sensor
#define DHTTYPE DHT11
DHT dht(ghTempRh, DHTTYPE);

/*
// DS18B20 Sensor
OneWire oneWire(tableTemp);
DallasTemperature sensors (&oneWire);
int tableTempF;
int tableTempC;
*/

// Greenhouse Temperature and Humidity measurements
int temperature = 0;
int humidity = 0;
int setPoint = 0;

void setup() {
  Serial.begin(9600);
  //DHT11 Sensor Setup
  dht.begin();
//  sensors.begin();

  //Setup Pin I/O
  pinMode(lowFan, OUTPUT);
  pinMode(medFan, OUTPUT);
  pinMode(highFan, OUTPUT);
  pinMode(coolingPump, OUTPUT);
  pinMode(actuator1, OUTPUT);
//  pinMode(heatPump1, OUTPUT);
//  pinMode(heatPump2, OUTPUT);
//  pinMode(heatPump3, OUTPUT);
//  pinMode(heatPump4, OUTPUT);
  pinMode(ghTempRh, OUTPUT);
  //pinMode(tableTemp, INPUT);
}

void loop() {
  readDHT();
  fanSpeed();

  // Print temperature and RH for testing
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *F ");
  //Output cooling stage result for testing
  Serial.print("Cooling Stage: ");
  Serial.println(setPoint);
  Serial.print("Low: ");
  Serial.println(digitalRead(lowFan));
  Serial.print("Med: ");
  Serial.println(digitalRead(medFan));
  Serial.print("High: ");
  Serial.println(digitalRead(highFan));
  // Print cooling pump status for testing
  Serial.print("Cooling Pump: ");
  Serial.println(digitalRead(coolingPump));
  // Print exhaust status for testing
  Serial.print("Exhaust: ");
  Serial.println(digitalRead(actuator1));
}

void readDHT() {
  // Wait a two seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  humidity = dht.readHumidity();
  // Read temperature as Fahrenheit 
  temperature = dht.readTemperature(true);
}



// Set Cooling Stage
void fanSpeed() {
  //If temperature sensor does not read, exit cooling function without changing setpoint
  if (temperature == 0 && humidity == 0) {
    return;
  }
  
  // Condition: temperature is less than 75 deg f, Cooling system OFF
  if (temperature < 75) {
    setPoint = 0;
    digitalWrite(lowFan, HIGH);
    digitalWrite(medFan, HIGH);
    digitalWrite(highFan, HIGH);
    digitalWrite(coolingPump, HIGH); // Turn cooling pump OFF
    digitalWrite(actuator1, HIGH); // Close vent or turn OFF exhaust fan
  }
  // Condition: if temperature is greater than 80 and cooling not on, turn on low
  if (temperature > 80 && setPoint < 1) {
    setPoint = 1;
    digitalWrite(lowFan, LOW);
    digitalWrite(medFan, HIGH);
    digitalWrite(highFan, HIGH);
    digitalWrite(coolingPump, LOW); // Turn cooling pump ON
    digitalWrite(actuator1, LOW); // Open vent or turn ON exhaust fan   
  }
  // Condition: If temperature is greater than 85 and cooling is off or low, turn on medium
  if (temperature > 85 && setPoint < 2) {
    setPoint = 2;
    digitalWrite(lowFan, HIGH);
    digitalWrite(medFan, LOW);
    digitalWrite(highFan, HIGH);
    digitalWrite(coolingPump, LOW); // Turn cooling pump ON
    digitalWrite(actuator1, LOW); // Open vent or turn ON exhaust fan    
  }
  // Condition: If temperature is greater than 90 and cooling is off, low, or med, turn on high
  if (temperature > 90 && setPoint < 3) {
    setPoint = 3;
    digitalWrite(lowFan, HIGH);
    digitalWrite(medFan, HIGH);
    digitalWrite(highFan, LOW);
    digitalWrite(coolingPump, LOW); // Turn cooling pump ON
    digitalWrite(actuator1, LOW); // Open vent or turn ON exhaust fan
  }
}
