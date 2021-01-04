/* Greenhouse Greenhouse Powered by an Arduino MEGA
    Major Revision Start: 01/03/2020
*/
// Adafruit Sensor Library must be installed, DHT.H library is dependent on it, or sketch won't compile!
#include "DHT.h" //DHT11 sensor library
#include "OneWire.h" // Library needed for DS18B20 sensors
#include "DallasTemperature.h" // Library needed for DS18B20 sensors

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
  { 0x28, 0xAA, 0x4D, 0xC0, 0x37, 0x14, 0x01, 0x9C }, // #6
  { 0x28, 0xA9, 0xAC, 0x0F, 0x30, 0x14, 0x01, 0x0B }, // #5 defective
  // { 0x28, 0xAA, 0x57, 0xAF, 0x1A, 0x13, 0x02, 0x5F }, // #4 defective
  { 0x28, 0xAA, 0x3C, 0xF9, 0x37, 0x14, 0x01, 0xE7 }, // #3
  { 0x28, 0xAA, 0xE8, 0xBD, 0x37, 0x14, 0x01, 0x48 }, // #2
  { 0x28, 0xAA, 0xC4, 0x13, 0x1B, 0x13, 0x02, 0x8E }, // #1
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

  printHeadings();
}

void loop()
{
  if (millis() - loopTime > loopInterval) {
    loopTime = millis();
    //DHT Sensor reading and writing to array
    for (int i = 0; i < 4; i++) {
      dhtData[i][0] = dht[i].readTemperature(true);
      dhtData[i][1] = dht[i].readHumidity();

      Serial.print(dhtData[i][0]);
      Serial.print(", ");
      Serial.print(dhtData[i][1]);
      Serial.print(", ");
    }
    
  // DS18B20 Sensor reading and writing to array
    sensors.requestTemperatures();
    Serial.print(",");
    for (int i = 0; i < 7; i++) {
      oneWireData[i] = sensors.getTempF(oneWireAddress[i]);
      Serial.print(oneWireData[i]);
      Serial.print(",");
    }
    
    Serial.println("");
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
