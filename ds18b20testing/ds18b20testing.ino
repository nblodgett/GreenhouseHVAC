#include "OneWire.h" // Library needed for DS18B20 sensors
#include "DallasTemperature.h" // Library needed for DS18B20 sensors

// DS18B20 Sensor Setup
// Connect all DS18B20 to pin 30
OneWire oneWire(30);
DallasTemperature sensors(&oneWire);

// DS18B20 Sensor Addresses
uint8_t oneWireAddress[7][8] = {
  { 0x28, 0xAA, 0xC6, 0xEE, 0x37, 0x14, 0x01, 0x42 }, // #8 Ambient Temperature
  { 0x28, 0xAA, 0xCB, 0x64, 0x26, 0x13, 0x02, 0x01 }, // #7 Greenhouse Temperature
  { 0x28, 0xAA, 0x4D, 0xC0, 0x37, 0x14, 0x01, 0x9C }, // #6
  { 0x28, 0xA9, 0xAC, 0x0F, 0x30, 0x14, 0x01, 0x0B }, // #5 defective
  // { 0x28, 0xAA, 0x57, 0xAF, 0x1A, 0x13, 0x02, 0x5F }, // #4 defective
  { 0x28, 0xAA, 0x3C, 0xF9, 0x37, 0x14, 0x01, 0xE7 }, // #3
  { 0x28, 0xAA, 0xE8, 0xBD, 0x37, 0x14, 0x01, 0x48 }, // #2
  { 0x28, 0xAA, 0xC4, 0x13, 0x1B, 0x13, 0x02, 0x8E }, // #1
};

String oneWireDesc[7] = {
  "Ambient Temperature,",
  "Greenhouse Temperature,",
  "Reservoir Temperature,",
  "Radiator Temperature,",
  "Bench 1 Temperature,",
  "Bench 2 Temperature,",
  "Bench 3 Temperature,"
};

void setup() {
  Serial.begin(9600);
  pinMode(9, INPUT);

  //DS18B20 Sensor Setup
  sensors.begin();
  Serial.print(",");
  for (int i = 0; i < 7; i++) {
    Serial.print(oneWireDesc[i]);
  }
  Serial.println();
}

void loop() {
  sensors.requestTemperatures();
  Serial.print(",");
  for (int i = 0; i < 7; i++) {
    int temp1 = sensors.getTempF(oneWireAddress[i]);
    Serial.print(temp1);
    Serial.print(",");
  }
  Serial.println("");
  delay(1000);
}
