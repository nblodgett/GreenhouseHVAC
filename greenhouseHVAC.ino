/*
   Greenhouse Heating and Cooling Automation Program
   Rev 0
   Created 2018 12 21
   Nick Blodgett
*/
#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

int lowFan = 13; // Low speed cooling fan relay output
int medFan = 12; // Medium speed cooling fan relay output
int highFan = 11; // High speed cooling fan relay output
int coolingPump = 10; // Evaporative cooling water pump relay output
int actuator1 = 9; // Exhaust vent linear actuator 1 relay output
int actuator2 = 8; // Exhaust vent linear actuator 2 relay output
int pumpAlarm = 7; // Evaporative cooling low water sensor input(NC)
int heatPump1 = 6; // hot water pump table 1 relay output
int heatPump2 = 5; // hot water pump table 2 relay output
int heatPump3 = 4; // hot water pump table 3 relay output
int heatPump4 = 3; // hot water pump hydroponic reservoir relay output
int ghTempRh = 2; // Greenhouse temperature and RH sensor (DHT11) input
int ambTempRh = 1; // Ambient temperature and RH sensor (DHT11) input
int tableTemp = 0; // Table/hydroponic reservoir temperature inputs (DS18B20) (qty 4)

void setup() {
  Serial.begin(9600);
  dht.begin();
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
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  /*
    if(h > maxHum || t > maxTemp) {
      digitalWrite(fan, HIGH);
    } else {
     digitalWrite(fan, LOW);
    }
  */
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");
}
