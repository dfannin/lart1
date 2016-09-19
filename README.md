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

# Dependencies
requires the following libraries to be installed in your Arduino Sketchbook `libraries/` directory :
+ [LibAPRS](https://github.com/markqvist/LibAPRS)
+ [TinyGPSplus](https://github.com/mikalhart/TinyGPSPlus) - You need to rename the files and directory to "TinyGPSplus" (from TinyGPS++), and edit the include line in the `TinyGPSPlus.cpp`  file, otherwise it won't compile via the Makefile, due to using the non-standard naming.  

# Status
This software is beta software, and is still in major update mode.

# Building
The Arduino IDE can be used to build and upload the firmware.  Alternatively, if you have a Linux avrdude/arduino.mk environment installed, you can use the attached Makefile to build and upload the firmware without the arduino IDE.
