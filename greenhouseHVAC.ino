/*
   Greenhouse Heating and Cooling Automation Program
   Created 2018 12 21
   Nick Blodgett
*/
#include "DHT.h" //DHT11 sensor library

// Assign all pin I/O
const int lowFan = 13; // Low speed cooling fan relay output
const int medFan = 12; // Medium speed cooling fan relay output
const int highFan = 11; // High speed cooling fan relay output
const int coolingPump = 10; // Evaporative cooling water pump relay output
const int actuator1 = 9; // Exhaust vent actuator 1 relay output
const int actuator2 = 8; // Exhaust vent actuator 2 relay output
const int pumpAlarm = 7; // Evaporative cooling low water sensor input(NC)
const int heatPump1 = 6; // hot water pump table 1 relay output
const int heatPump2 = 5; // hot water pump table 2 relay output
const int heatPump3 = 4; // hot water pump table 3 relay output
const int heatPump4 = 3; // hot water pump hydroponic reservoir relay output
const int ghTempRh = 2; // Greenhouse temperature and RH sensor (DHT11) input
const int ambTempRh = 1; // Ambient temperature and RH sensor (DHT11) input
const int tableTemp = 0; // Table/hydroponic reservoir temperature inputs (DS18B20) (qty 4)

//DHT11 sensor
#define DHTTYPE DHT11
DHT dht(ghTempRh, DHTTYPE);

// Greenhouse Temperature and Humidity measurements
int temperature = 0;
int humidity = 0;
int setPoint = 0;
int pumpAlarmStatus; // Status of evap cooling water pump low water sensor

void setup() {
  Serial.begin(9600);
  //DHT11 Sensor Setup
  dht.begin();

  //Setup Pin I/O
  pinMode(lowFan, OUTPUT);
  pinMode(medFan, OUTPUT);
  pinMode(highFan, OUTPUT);
  pinMode(coolingPump, OUTPUT);
  pinMode(actuator1, OUTPUT);
  pinMode(actuator2, OUTPUT);
  pinMode(pumpAlarm, INPUT);
  pinMode(heatPump1, OUTPUT);
  pinMode(heatPump2, OUTPUT);
  pinMode(heatPump3, OUTPUT);
  pinMode(heatPump4, OUTPUT);
  pinMode(ghTempRh, OUTPUT);
  pinMode(ambTempRh, INPUT);
  pinMode(tableTemp, INPUT);

}

void loop() {
  readDHT();
  fanSpeed();
}

void readDHT() {
  // Wait a two seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  humidity = dht.readHumidity();
  // Read temperature as Celsius
  temperature = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Convert temperature from C to F
  temperature = (temperature * 9/5) + 32;

  // Print temperature and RH for testing
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *F ");
}

// Set Cooling Stage
void fanSpeed() {

  pumpAlarmStatus = digitalRead(pumpAlarm); // Check for low evap cooling water event
  // Condition: temperature is less than 75 deg f, Cooling system OFF
  if (temperature < 75) {
    setPoint = 0;
    digitalWrite(coolingPump, LOW); // Turn off evap cooling water pump
    digitalWrite(lowFan, LOW);
    digitalWrite(medFan, LOW);
    digitalWrite(highFan, LOW);
    digitalWrite(actuator1, LOW); // Shut roof vent
    digitalWrite(actuator2, LOW); // or shut off exhaust fan
  }
  // Condition: if temperature is greater than 80 and cooling not on, turn on low
  if (temperature > 80 && setPoint < 1) {
    setPoint = 1;
    // Turn on evap cooling water pump, will not turn on if low water sensor is detected
    digitalWrite(coolingPump, pumpAlarmStatus);
    digitalWrite(lowFan, HIGH);
    digitalWrite(medFan, LOW);
    digitalWrite(highFan, LOW);
    digitalWrite(actuator1, HIGH); // Open roof vent
    digitalWrite(actuator2, HIGH); // or energize exhaust fan
  }
  // Condition: If temperature is greater than 85 and cooling is off or low, turn on medium
  if (temperature > 85 && setPoint < 2) {
    setPoint = 2;
    // Turn on evap cooling water pump, will not turn on if low water sensor is detected
    digitalWrite(coolingPump, pumpAlarmStatus);
    digitalWrite(lowFan, LOW);
    digitalWrite(medFan, HIGH);
    digitalWrite(highFan, LOW);
    digitalWrite(actuator1, HIGH); // Open roof vent
    digitalWrite(actuator2, HIGH); // or energize exhaust fan

  }
  // Condition: If temperatuer is greater than 90 and cooling is off, low, or med, turn on high
  if (temperature > 90 && setPoint < 3) {
    setPoint = 3;
    // Turn on evap cooling water pump, will not turn on if low water sensor is detected
    digitalWrite(coolingPump, pumpAlarmStatus);
    digitalWrite(lowFan, LOW);
    digitalWrite(medFan, LOW);
    digitalWrite(highFan, HIGH);
    digitalWrite(actuator1, HIGH); // Open roof vent
    digitalWrite(actuator2, HIGH); // or energize exhaust fan
  }

  //Output cooling stage result for testing
  Serial.print("Cooling Stage: ");
  Serial.println(setPoint);
  Serial.print("Low: ");
  Serial.println(digitalRead(lowFan));
  Serial.print("Med: ");
  Serial.println(digitalRead(medFan));
  Serial.print("High: ");
  Serial.println(digitalRead(highFan));
  Serial.print("coolingPump: ");
  Serial.println(digitalRead(coolingPump));
  Serial.print("Pump Alarm: ");
  Serial.println(pumpAlarmStatus);  

}
