#ifndef LART1_SETTINGS_H
#define LART1_SETTINGS_H

//----------- user settable values  --------------------// 
//
// turn on beacon transmitter 
#define APRS_BEACON true
// turn on aprs packet receiver serial output
#define APRS_RECV    true
// adapts beacon update time if the position is changing
#define NOTSOSMART_BEACON true
// writes SD Log file 
#define SDLOG_WRITE false

// set your call sign and ssid in the config file
// 

// default message comment (max of 35 characters) 
#define APRS_COMMENT "LART1 Tracker v1"

// DRA818 Transceiver 
// APRS 2 meter tx and rx frequency for North America  (134-174MHz) 
#define TXFREQ 144.39
#define RXFREQ 144.39
// TX and RX CTCSS tone number code  (range 0-38)   
// see: https://en.wikipedia.org/wiki/Continuous_Tone-Coded_Squelch_System#List_of_tones
#define TXCTCSS 0 
#define RXCTCSS 0 
// transciver speaker volume (1-8)
#define VOL 5 
/// transceiver squlech levels (0=open squelch, 1-8)  
#define SQUELCH 2 
// transeiver channel  bandwidth (0=12.5kHz, 1=25kHz) 
#define BW 0

// APRS TNC settings
// transciver uses open squelch (must set this to true, if SQUELCH is set to  0 ) 
#define OPEN_SQUELCH false

// Destination Address (does not need to be changed)
#define APRS_DEST "APZCCR"

// APRS Path 1 Routing 
#define PATH1 "WIDE1"
#define PATH1_SSID  1
// APRS Parth 2 Routing
#define PATH2 "WIDE2"
#define PATH2_SSID  1

// APRS preamble length (ms)
#define PREAMBLE 350L
#define TAIL 50L

// APRS Symbol
// see: http://www.aprs.net/symbols/symbols-new.txt
// short list of basic symbols
// 'n' Node (bulls eye) 
// '-' House
// '/' Red Dot
// '0'-'9' Numeral Circle
// '>' Car 
// '<' Motorcycle
// 'k' Truck
// 'R' RV 
// '[' Human
// 'b' Bike 
// 'p' Rover 
// 'o' EOC
// ';' Portable Operation (tent) 
// 'E' Special Events (eyeball)
// short list of alternate table symbols
// '-' House 
// ';' Park/Picnic event
// 'o' small circle
// 'k' SUV
#define SYMBOL 'n' 

#define USE_ALT_SYM_TABLE false 

// lcd backlight interval in seconds
#define BKLIGHT_INTERVAL 60000L

// beaconing timing 
// update beacon every xx ms after the inital period
// recommended settings:
//     fixed: 30 minutes  1800 seconds
//     fixed: 20 minutes  1200 seconds
//     mobile: 1 or 2 minutes  60 120 seconds
// #define UPDATE_BEACON_FIXED 300000L
// Fixed is used for both NoSoSmart (as stopped interval) and Fixed as the interval
#define UPDATE_BEACON_FIXED 1200000L

// timing is set to update to this interval
// if the position exceeds mph_threshold
#define UPDATE_BEACON_MOVING 90000L

// update beacon within  xx ms firs time through (for smart mode)  
#define UPDATE_BEACON_INIT  30000L

// output statistics every xx ms
#define UPDATE_DISPLAY 60000L 

// gps port speed
#define GPS_PORT_BAUD  9600

// usb port speed
#define USB_PORT_BAUD  9600

// DRA818 port speed
#define DRA818_PORT_BAUD  9600

// if threshold exceeded, then trigger position change
#define MPH_THRESHOLD  5.0

// LCD row and column settings
#define OPTION_LCD  true
#define LCDCOL 16
#define LCDROW 2

#endif
