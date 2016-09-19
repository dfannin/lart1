# LART/1
Arduino Software for an APRS Beacon Tracker
# Introduction
This arduino software provides an aprs beacon tracker, sending out periodic location information.   The arduino software uses the libAPRS library, interface circuits (based on the MicroModem/MicroAPRS schematic), and a 2 meter VHF transceiver. 
# Dependencies
requires the following libraries in your Arduino Sketchbook `libaries/` directory :
+ [LibAPRS](/markqvist/LibAPRS)
+ [TinyGPSplus](/mikalhart/TinyGPSPlus) - You need to rename the files and directory to "TinyGPSplus" (from TinyGPS++), and edit the include line in the `TinyGPSPlus.cpp`  file, otherwise it won't compile via the Makefile, due to using the non-standard naming.  

# Status
This software is beta software, and is still in major update mode.
# Building
The arduino IDE can be used to build and upload the 
