/*
   Arduino Greenhouse HVAC
   Date: 2020 09 15
*/

#include "DHT.h" //DHT11 sensor library
// Adafruit Sensor Library must be installed, DHT.H library is dependent on it, or sketch won't compile!

unsigned long loopTime = 0; // Loop timer
int loopInterval = 5000; // ms between loop measurement intervals
float h; // humidity measurement
float t; // deg c measurement
float f; // deg f measurement
int temp; // temp measurement
int hLimit = 70; // humidity limit
int coolingStage; // HVAC cooling stage (-1 through 3)
const int analogPins[] = {A0, A1, A2, A3, A4};
int setPointArray[11]; // Array of temperature set points
int pinState[14]; // Array of pin output state

// Init DHT Sensors
#define DHTTYPE DHT11
DHT greenhouseTempRh(analogPins[0], DHTTYPE); // Ambient temperature pin, DHT11 sensor

void setup() {
  Serial.begin(9600);

  // Setup all analog pins for INPUT
  for (int i = 0; i <= 4; i++) {
    pinMode(analogPins[i], INPUT);
  }

  //Setup OUTPUT pins and initialize is HIGH state (relay OFF)
  for (int i = 2; i <= 13; i++) {
    pinMode(i, OUTPUT);
    pinState[i] = HIGH;
  }
  writePins(); // Update pin output states

  greenhouseTempRh.begin(); // begin reading temp and rh

  // Set heating and cooling temperatures in degrees F
  setPointArray[0] = 65; // Cooling Stage -1
  setPointArray[1] = 78; // Cooling Stage 0
  setPointArray[2] = 80; // Cooling Stage 1
  setPointArray[3] = 85; // Cooling Stage 2
  setPointArray[4] = 90; // Cooling Stage 3
  setPointArray[5] = 90; // Min Res Temp #1
  setPointArray[6] = 95; // Max Res Temp #1
  setPointArray[7] = 72; // NFT Reservoir minimum temp
  setPointArray[8] = 72; // Benches Minimum temp
  setPointArray[9] = 72; // 
  setPointArray[10] = 72; //
}

void loop() {
  if(millis() - loopTime > loopInterval) {
    loopTime = millis();
    readTempRh(greenhouseTempRh);
    fanSpeed();
    mistingRelay();
    writePins();
    serialOutput();
  }

}

DHT readTempRh(DHT sensor) {
  h = sensor.readHumidity();
  t = sensor.readTemperature();
  f = sensor.readTemperature(true);

  // If temp is nan then turn off HVAC
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    coolingStage = 0;
    return;
  }

  temp = f;

  //Heat ON
  if (temp <= setPointArray[0]) {
    coolingStage = -1;
    return;
  }
  // Heat OFF Cool OFF
  if (temp > setPointArray[0] || temp < setPointArray[2]) {
    coolingStage = 0;
  }
  // Cool ON Stage 1
  if (temp >= setPointArray[2]){
    coolingStage = 1;
  }
  // Cool ON Stage 2
  if (temp >= setPointArray[3]){
    coolingStage = 2;
  }
  // Cool ON Stage 3
  if (temp >= setPointArray[4]){
    coolingStage = 3;
  }
}

void fanSpeed() {
  // Set Point 0 - No heating or cooling - Turn OFF evap cooler, water pump, and vent
  if (coolingStage <= 0) {
    // Turn OFF fans, water pump, and close vents
    for (int i = 7; i <= 12; i++) {
      pinState[i] = HIGH;
    }
  }
  
  // Anything else start cooling
  else {
    pinState[7] = LOW; // Exhaust relay 1
    pinState[8] = LOW; // Exhause relay 2
    pinState[9] = LOW; // Evaporative cooling water pump relay output
    pinState[10] = HIGH; // Turn OFF HI fan speed
    pinState[11] = HIGH; // Turn OFF MED fan speed
    pinState[12] = HIGH; // Turn OFF LOW fan speed
  }

  // SetPoint 1 - Low fan setting
  if (coolingStage == 1) {
    pinState[12] = LOW; // Turn ON LOW fan speed
  }
  // SetPoint 2 - MED fan setting
  if (coolingStage == 2) {
    pinState[11] = LOW; // Turn ON MED fan speed
  }
  // SetPoint 3 - HI fan setting
  if (coolingStage == 3) {
    pinState[10] = LOW; // Turn ON HI fan speed
  }
}

void mistingRelay() {
  // If temp is nan then turn off HVAC
  if (isnan(h)) {
    Serial.println("Failed to read humidity from DHT sensor, cannot activate misters until corrected!");
    pinState[13] = HIGH;
    return;
  }
  
  // If cooling stage is less than 3 turn OFF misters
  if(coolingStage < 3) {
    pinState[13] = HIGH;
    return;
  }
  // If humidity is less than the limit turn ON misters
  if(h < hLimit) {
    pinState[13] = LOW;
    Serial.println("Misters Turned ON");
  }
  // Else turn OFF misters
  else {
    pinState[13] = HIGH;
    Serial.println("Misters Turned OFF");
  }
}


// Write all the pin states on OUTPUT pins
void writePins() {
  for (int i = 2; i <= 13; i++) {
    digitalWrite(i, pinState[i]);
  }
}

void serialOutput() {
  Serial.print(f);
  Serial.println(" Degrees F");
  Serial.print(h);
  Serial.println(" % Humidity");
  Serial.print("Cooling Stage: ");
  Serial.println(coolingStage);
}
