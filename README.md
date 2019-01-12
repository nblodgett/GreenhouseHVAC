# Arduino Controlled Greenhouse HVAC

Arduino based HVAC system for a small greenhouse. Uses a modified household three stage evaporative cooler as air intake and greenhouse exhaust vent/fan for the cooling side. Hot side utilizes an inexpensive instant hot water heater recirculating coolant into individually controlled heated benches and hydroponic reservoirs.

## Features
* Brings precise climate control to a hobby greenhouse while minimizing cost, and using easily sourced parts
* Ability to individually control root zone temperature of individual benches as well as separate hydroponic reservoirs
* Adruino based system allows for expansion for marginal extra cost. Possible expansion to (but not implemented):
   * Dalogging
   * Supplemental light control
   * Additional sensors such as PAR or leaf wetness
   * Additional heat zones or radiator heater
   * Advanced HVAC logic
   * Nutrient control (measurement and dosing)

## Arduino Parts
* Arduino Uno
* 16 Channel Relay Module
* 4 Channel Relay Module (for modified evaporative cooler, keeps mains voltage outside HVAC controls enclosure)
* 12 Volt Power Supply (Modified computer PSU can supply 12VDC and 5VDC)
* DS18B20 Waterproof Temperature Sensors (1 for each heating zone)
* DHT11 Sensors (Used for greenhouse ambient temperature and humidity, additional needed for outside ambient temperature)

## Installation
* Download or clone repository
* Extract and upload greenhouseHVAC.ino to arduino

## Included Libraries
* DHT.h
* OneWire.h
* DallasTemperature.h