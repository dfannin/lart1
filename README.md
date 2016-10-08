# LART/1
Arduino Software for an APRS Beacon Tracker 

a.k.a Livermore Amateur Radio Club APRS Tracker (LART) 
# Introduction
This arduino software provides an aprs beacon tracker, sending out periodic location information.   The arduino software uses the libAPRS library, interface circuits (based on the MicroModem/MicroAPRS schematic), and a 2 meter VHF transceiver. 

# Hardware
LART1 requires/assumes the following hardware:
+ an arduino board (developed on a Nano)
+ MicroModem/APRS circuit - [MicroModem Circuit](https://github.com/markqvist/MicroModem)
+ GPS that provides NMEA output - almost all of them do. The Ublox Neo-6M is a good start. 

See the Schematic directory for the  circuit schematics.

# Dependencies
requires the following libraries to be installed in your Arduino Sketchbook `libraries/` directory :
+ [LibAPRS](https://github.com/markqvist/LibAPRS)
+ [TinyGPSplus](https://github.com/mikalhart/TinyGPSPlus) - You need to rename the files and directory to "TinyGPSplus" (from TinyGPS++), and edit the include line in the `TinyGPSPlus.cpp`  file, otherwise it won't compile via the Makefile, due to using the non-standard naming.  
+ [DRA818](https://github.com/darksidelemm/dra818) - DRA818/SA818 module control library 
+ [LiquidCrystal_I2C](https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home) - Francisco Malpartida's excellent I2C library for LCD displays - use this one instead of the stock LCD library. 

# Status
This software is beta software, and is still in major update mode.

