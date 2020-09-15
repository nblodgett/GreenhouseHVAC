/*
   Arduino Automated Greenhouse HVAC
*/

#include "DHT.h" //DHT11 sensor library
// Adafruit Sensor Library must be installed, DHT.H library is dependent on it, or sketch won't compile!
#include "OneWire.h" // Library needed for DS18B20 sensors
#include "DallasTemperature.h" // Library needed for DS18B20 sensors



// Description of all DS18B10 sensors
char *sensorDesc [] =
{
  "#0 Empty",
  "#1 Reservoir",
  "#2 Rear Bench",
  "#3 Empty"
  "#4 Side Bench"
  "#5 NFT Reservoir",
  "#6 Greenhouse",
  "#7 Not Used",
  "#8 Not Used",
};

// All DS18B20 sensor addresses
byte sensorAddress [9] [8] =
{
  { },
  { 0x28, 0xAA, 0xC4, 0x13, 0x1B, 0x13, 0x02, 0x8E}, // #1 Tag
  { 0x28, 0xAA, 0x57, 0xAF, 0x1A, 0x13, 0x02, 0x5F}, // #2 Tag
  { },
  { 0x28, 0xAA, 0xE8, 0xBD, 0x37, 0x14, 0x01, 0x48}, // #4 Tag
  { 0x28, 0xAA, 0x3C, 0xF9, 0x37, 0x14, 0x01, 0xE7}, // #5 Tag
  { 0x28, 0xAA, 0x4D, 0xC0, 0x37, 0x14, 0x01, 0x9C}, // #6 Tag
  { 0x28, 0xAA, 0xCB, 0x64, 0x26, 0x13, 0x02, 0x01}, // #7 Tag
  { 0x28, 0xAA, 0xC6, 0xEE, 0x37, 0x14, 0x01, 0x42}, // #8 Tag
};

//Description of each output pin
char *pinDesc [] =
{
  "#0 No 0 pin",
  "#1 Empty",
  "#2 Reservoir heater solenoid",
  "#3 NFT reservoir solenoid",
  "#4 Dutch  solenoid",
  "#5 ",
  "#6",
  "#7 Exhaust relay #1",
  "#8 Exhaust relay #2",
  "#9 Evaporative cooler water pump relay",
  "#10 High speed evaporative cooling fan relay",
  "#11 Medium speed evaporative cooling fan relay",
  "#12 Low speed evaporative cooling fan relay",
  "#13 Overhead mist relay",
};
int pinState[14]; // Array of pin output state

//Description of each analog pin, all analog pins will be input pins
char *analogPinDesc [] =
{
  "#0 Enclosure temp/rh sensor",
  "#1 Greenhouse temp/rh sensor",
  "#2 Outdoor temp/rh sensor",
  "#3 All DS18B20 sensors",
  "#4 Evaporative cooler low water trigger",
};
const int analogPin[] = {A0, A1, A2, A3, A4};

// Description of temperature set points in degrees F
char *coolingStageDesc [] =
{
  "Heating stage",
  "No Heat/Cool stage",
  "Cooling stage low",
  "Cooling stage med",
  "Cooling stage Hi",
};

// Heat/cool stage set points
int *coolingStage [] =
{
  70,
  78,
  80,
  85,
  90,
};

// Description of heat set points
char *setPointDesc [][2] =
{
  { "Minimum temp: ", "Maximum temp: " },
  { "heat reservoir", "heat reservoir" },
  { "NFT reservoir", "NFT reservoir" },
  { "dutch bucket reservoir", "dutch bucket reservoir" },
  { "rear bench", "rear bench" },
  { "side bench", "side bench" },
};

// Heat set point array
int setPoint [][2] =
{
  {"min", "max"},
  {90, 95},
  {64, 66},
  {68, 70},
  {76, 78},
  {76, 78},
};

void setup() {
  Serial.begin(9600);

  // Setup all analog pins for INPUT
  for (int i = 0; i <= 4; i++) {
    pinMode(analogPin[i], INPUT);
  }

  //Setup OUTPUT pins and initialize is HIGH state (relay OFF)
  for (int i = 2; i <= 13; i++) {
    pinMode(i, OUTPUT);
    pinState[i] = HIGH;
  }

  //DS18B20 Sensor Setup
  //sensors.begin();

}

void loop() {
  // put your main code here, to run repeatedly:

}
