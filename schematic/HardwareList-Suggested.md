# Suggested Hardware List for LART/1 Tracker

What you'll need for the LART/1 Tracker (a/o 2016) 

+ Arduino Mega2560 board or compatible
    + LART/1 uses three serial ports, so the Mega is the only choice. The script is too large to compile for the Uno/Nano or other 328p-based boards. 
    + Mega2560-core (from Inhaos)  http://www.inhaos.com/uploadfile/otherpic/DS-Mega2560-CORE-V01-EN.pdf
        + this one was used for development
    + Arduino Mega2560 - https://www.arduino.cc/en/Main/ArduinoBoardMega2560 
    + Search Ebay for "Mega2560-core mini" - should be around $14 with shipping. 
+ NMEA GPS with serial port
    + The Neo-6m GPS module, packaged as a flight controller,  is recommended. It has the complete GPS module and  antenna in a small plastic dome housing thats perfect for this app. The app requires a stream of NMEA messages at 9600 baud. 
    + Search for "Neo6-M gps module built-in compass for APM flight controller" at Ebay.  You should find them for $10-$11, with shipping included.  
+ DRA818v Module (2 meter transceiver module)  
    + This is a serial controlled 2 meter 1 Watt VHF transceiver, packaged as an SMD module.  It's also known as the SA818v. 
    + There are two types of modules ( the "v" and the "u",for vhf and uhf, respectively) - make sure you get the "v" version.
    + Search for "DRA818v module" - you should be able to get them for $11 with shipping. 
    + They are also packaged on a board with audio circuitry and Low Pass Filter (also more expensive).  They may work, but you'll need to figure out how.
+ SD card module for Arduino
    + provides the SD card interface for reading the configuration.  
    + Search Ebay for "arduino SD card module"  About $0.74 , shipped.
+ LCD Display (this is optional)
    + provide character output on a 16x2 display.  Not really needed, but nice. They also use 20 to 80mW of power (with the backlight on).
    + You'll need the 16x2 LCD Display with i2c interface. 
    + Search Ebay for "arduino lcd 16x2  display i2c" - about $3-4, shipped
+ APRS circuit
    + you'll need to build the APRS circuit (shown in the schematic)
    + The board is a basic circuit, with resistors, caps and connectors. You'll also need a 2N7000 mosfet for the PTT switch.
    + You'll also need to plan out the connectors (0.1 inch headers recommended) and the antenna connection (SMA is a good connector for this)
+ Arduino IDE programming environment
    + You'll need an Arduino IDE programming environment 
    + Software (free) is available here: https://www.arduino.cc/en/Main/Software
    + works on windows/mac/linux
