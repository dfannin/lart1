#ifndef LART1_SETTINGS_H
#define LART1_SETTINGS_H

//----------- user settable values  --------------------// 
//
// turn on beacon transmitter 
#define APRS_BEACON  true 
// turn on aprs packet receiver output
#define APRS_RECV    true
// adapts beacon update time if the position is changing
#define SMART_BEACON true 

// set your call sign and ssid  here
// don't use mine.
#define CALLSIGN "KK6DF"
#define SSID 2

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
// transciver filter settings 
// note: "true" bypasses the  filter (turns the filter off), "false"  sets to normal mode (turns on) 
// yes, this is confusing!
#define FILTER_PREMPHASIS false
#define FILTER_HIGHPASS true
#define FILTER_LOWPASS true

// APRS TNC settings
// transciver uses open squelch (must set this to true, if SQUELCH is set to  0 ) 
#define OPEN_SQUELCH false

// Destination Address (does not need to be changed)
#define APRS_DEST "APZMDM"

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
// see: http://www.aprs.net/vm/DOS/SYMBOLS.HTM
// short list of symbols
// 'n' Plain Node
// '-' House QTH
// '/' Dot
// '0'-'9' Numeral Circle
// '>' Car
// 'R' RV
// '[' Runner
// 'b' Bike
// 'p' Rover (puppy)
#define SYMBOL 'n' 

//
// has an LCD
#define OPTION_LCD  true
// backlight interval in seconds
#define BKLIGHT_INTERVAL 20000L

// beaconing timing 
// update beacon every xx ms after the inital period
// should not be set to less than 60 seconds
#define UPDATE_BEACON 300000L
// update beacon every xx ms during the initial period
#define UPDATE_BEACON_INIT  60000L
// number of times to send out a beacon during the intial period
// after that, interval is changed to update_beacon
int beacon_init_count = 1 ;

// output statistics every xx ms
#define UPDATE_DISPLAY 60000L 

#endif
