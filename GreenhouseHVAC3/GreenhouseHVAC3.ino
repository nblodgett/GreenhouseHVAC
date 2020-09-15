/*
   Arduino Greenhouse HVAC
   Date: 2020 09 15
*/

#include "DHT.h" //DHT11 sensor library
// Adafruit Sensor Library must be installed, DHT.H library is dependent on it, or sketch won't compile!
const int analogPins[] = {A0, A1, A2, A3, A4};
float h;
float t;
float f;
int temp;
int coolingStage;
int setPointArray[11];
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
  writePins();
  
  //pinmode(analogPins[0], INPUT); // Change analong pin 0 to input
  greenhouseTempRh.begin(); // begin reading temp and rh

  // Set heating and cooling temperatures in degrees F
  setPointArray[0] = 60; // Cooling Stage -1
  setPointArray[1] = 78; // Cooling Stage 0
  setPointArray[2] = 80; // Cooling Stage 1
  setPointArray[3] = 85; // Cooling Stage 2
  setPointArray[4] = 90; // Cooling Stage 3
  setPointArray[5] = 90; // Min Res Temp #1
  setPointArray[6] = 95; // Max Res Temp #1
  setPointArray[7] = 78; // Side bench Temp #2
  setPointArray[8] = 72; // Hydro res temp #3
  //setPointArray[9] = 72; // Hydro Res 1 (NFT Channel) Temp
  //setPointArray[10] = 72; // Hydro Res 2 (buckets) Temp
}

void loop() {
  readTempRh(greenhouseTempRh);
  delay(1000);
  fanSpeed();
  writePins();
  serialOutput();
}

DHT readTempRh(DHT sensor) {
  h = sensor.readHumidity();
  t = sensor.readTemperature();
  f = sensor.readTemperature(true);
  delay(100); // Wait for reading to settle

   // If temp is nan then turn off HVAC
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    coolingStage = 0;
    return;
  }

  temp = f;
  
  /*
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.println(" *F\t");
  */

 // If temp is nan then turn off HVAC

  //Heat ON
  if (temp <= setPointArray[0]) {
    coolingStage = -1;
    return;
  }
  // Heat OFF Cool OFF
  if (temp <= setPointArray[1]) {
    coolingStage = 0;
  }
  // Cool ON Stage 1
  if (temp >= setPointArray[2] && coolingStage < 1) {
    coolingStage = 1;
  }
  // Cool ON Stage 2
  if (temp >= setPointArray[3] && coolingStage < 2) {
    coolingStage = 2;
  }
  // Cool ON Stage 3
  if (temp >= setPointArray[4] && coolingStage < 3) {
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

  // SetPoint 1 - Low fan setting - Turn ON evap cooler, water pump and vent
  if (coolingStage == 1) {
    pinState[7] = LOW;
    pinState[8] = LOW;
    pinState[9] = LOW; // Evaporative cooling water pump relay output
    pinState[10] = HIGH; // Turn OFF HI fan speed
    pinState[11] = HIGH; // Turn OFF MED fan speed
    pinState[12] = LOW; // Turn ON LOW fan speed
  }

  // SetPoint 2 - MED fan setting - Turn ON evap cooler, water pump and vent
  if (coolingStage == 2) {
    pinState[7] = LOW;
    pinState[8] = LOW;
    pinState[9] = LOW; // Evaporative cooling water pump relay output
    pinState[10] = HIGH; // Turn OFF HI fan speed
    pinState[11] = LOW; // Turn ON MED fan speed
    pinState[12] = HIGH; // Turn OFF LOW fan speed
  }

  // SetPoint 3 - HI fan setting - Turn ON evap cooler, water pump and vent
  if (coolingStage == 3) {
    pinState[7] = LOW;
    pinState[8] = LOW;
    pinState[9] = LOW; // Evaporative cooling water pump relay output
    pinState[10] = LOW; // Turn ON HI fan speed
    pinState[11] = HIGH; // Turn OFF MED fan speed
    pinState[12] = HIGH; // Turn OFF LOW fan speed
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
