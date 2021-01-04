// Adafruit Sensor Library must be installed, DHT.H library is dependent on it, or sketch won't compile!
#include "DHT.h" //DHT11 sensor library

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

void setup()
{
  Serial.begin(9600);
  for (auto& sensor : dht) {
    sensor.begin();
  }
      for (int i = 0; i < 4; i++) {
      Serial.print(dhtDataDesc[i][0]);
      Serial.print(dhtDataDesc[i][1]);
    }
  Serial.println();
}

void loop()
{
  if (millis() - loopTime > loopInterval) {
    loopTime = millis();
    for (int i = 0; i < 4; i++) {
      dhtData[i][0] = dht[i].readTemperature(true);
      dhtData[i][1] = dht[i].readHumidity();
    }

    for (int i = 0; i < 4; i++) {
      Serial.print(dhtData[i][0]);
      Serial.print(", ");
      Serial.print(dhtData[i][1]);
      Serial.print(", ");
    }
          Serial.println();
  }
}
