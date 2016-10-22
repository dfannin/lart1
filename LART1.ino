/*
 * Livermore Amateur Radio Klub (LART) APRS Tracker (LART/1)
 *
 * Provides (Automatic Position Reporting System) APRS tracker features that can be used 
 * as an GPS location transmitter "beacon" , and can also function as an  APRS packet receiver.     
 * The design is designed to integrate  the DRA818v/SA818v 2 meter 1 watt transceiver module, and
 * a NMEA compatible GPS module.  
 *
 * The following libraries are required (modified libraries are provided in the repository):
 * LibAPRS - provides the software TNC featuree
 * DRA818 - controls the  2 meter transceiver
 * TinyGPSplus - parses the GPS messages
 * LiquidCrystal_I2C (optional) - provides LCD display
 *
 * The hardware specifications and schematic are provided in the repository. It is designed to
 * run on a Arduino Mega2560 compatible board.
 * 
 * Author: David Fannin, KK6DF
 * Created: October 2016 
 * License: MIT License
 * Licenses and Copyright Notices for Modified Libraries are retained by the original authors. 
 *
 */

#if defined(__AVR_ATmega2560__) 
#define TARGET_CPU mega2560
#endif

#include "Arduino.h" 
#include "LART1_Settings.h"
#include "LiquidCrystal_I2C.h"
#include "LibAPRS.h"
#include "TinyGPSplus.h"
#include "DRA818.h"

#define VERSION "Beta-0.998m"
#define ADC_REFERENCE REF_5V

// sets PTT pin (don't change, the pin is used by Port Manipulation later on) 
#define PTT 25

// usb serial port
HardwareSerial  * serialdb   = &Serial ;
// gps port
HardwareSerial  * serialgps  = &Serial1 ;
// DRA818 port
HardwareSerial * dra_serial = &Serial2 ;



unsigned long update_beacon = UPDATE_BEACON_INIT ;

unsigned long lastupdate = 0 ;

unsigned long lastupdatedisplay = 0 ;
bool forcedisplay = false ;

int sent_count=0 ;

#ifdef OPTION_LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7 , 3, POSITIVE); 
#endif 

// GPS setup

TinyGPSPlus gps ;


DRA818 dra(dra_serial, PTT);

boolean gotPacket = false ;
int recv_count = 0 ;
AX25Msg incomingPacket;
uint8_t *packetData;

void aprs_msg_callback(struct AX25Msg *msg)
{
   if(!gotPacket) {
      gotPacket = true ;
      memcpy(&incomingPacket, msg, sizeof(AX25Msg));
      if (freeMemory() > msg->len) {
         packetData = (uint8_t*) malloc(msg->len);
         memcpy(packetData,msg->info,msg->len);
         incomingPacket.info=packetData;
      } else {
         gotPacket = false ;
      }
   }
}


void processPacket()
{
   if(gotPacket) {
      gotPacket = false ;
      recv_count++ ;
      if( gps.date.isValid() ) {
          serialdb->print(gps.date.year()) ;
          serialdb->print("-") ;
          serialdb->print(gps.date.month()) ;
          serialdb->print("-") ;
          serialdb->print(gps.date.day()) ;
          serialdb->print(" ") ;
          if (gps.time.hour() < 10 ) serialdb->print("0") ;
          serialdb->print(gps.time.hour()) ;
          serialdb->print(":") ;
          if (gps.time.minute() < 10 ) serialdb->print("0") ;
          serialdb->print(gps.time.minute()) ;
          serialdb->print(":") ;
          if (gps.time.second() < 10 ) serialdb->print("0") ;
          serialdb->print(gps.time.second()) ;
          serialdb->print(" ") ;
      }

      serialdb->print(F(",Rcv Pkt:s:"));
      serialdb->print(incomingPacket.src.call);
      serialdb->print(F("-")) ;
      serialdb->print(incomingPacket.src.ssid);
      serialdb->print(F(",d:")) ;
      serialdb->print(incomingPacket.dst.call);
      serialdb->print(F("-")) ;
      serialdb->print(incomingPacket.dst.ssid);

      for(int i = 0; i < incomingPacket.rpt_count ; i++) {
        serialdb->print(F(",r:")) ;
        serialdb->print(incomingPacket.rpt_list[i].call);
        serialdb->print(F("-")) ;
        serialdb->print(incomingPacket.rpt_list[i].ssid);
      }

      serialdb->print(F(",data:")) ;
      for(int i = 0; i < incomingPacket.len; i++) {
          serialdb->print( (char) incomingPacket.info[i]) ;
      }
      serialdb->println("") ;

      free(packetData) ;

      // serialdb->print(F("Free Ram:")) ;
      // serialdb->println(freeMemory()) ;

   }
}



void locationUpdate(const char *lat, const char *lon,int altitude, int height = 0 ,int power=1, int gain=0, int dir=0)
{
   char comment[43] ; // max of 36 chars with PHG data extension, 43 chars  without
   APRS_setLat(lat);
   APRS_setLon(lon);
   // APRS_setHeight(height);
   // APRS_setPower(power);
   // APRS_setGain(gain);
   // APRS_setDirectivity(dir);
   sprintf(comment,"/A=%06dLART-1 Tracker beta",altitude) ;
   APRS_sendLoc(comment, strlen(comment),serialdb);
}


double  ddtodmh(float dd) {

    double  value = dd ;
    double  degrees =  floor(value) ;
    value = ( value - degrees ) * 60  ;
    double  minutes =  floor(value ) ; 
    value =  (value - minutes) ;
    return ( degrees*100 ) + minutes + value ; 
}


void setup()
{

#ifdef OPTION_LCD
   lcd.begin(16,2);
   lcd.clear();
   lcd.print(F("LART/1 APRS BEAC")); 
   lcd.setCursor(0,1);
   lcd.print(F("Call: ")) ;
   lcd.print(CALLSIGN) ;
   lcd.print("-") ;
   lcd.print(SSID) ;
#endif

   dra_serial->begin(9600) ;
   serialgps->begin(4800) ;
   serialdb->begin(9600) ;

   serialdb->println(F("LART/1 APRS Beacon")) ;
   serialdb->print(CALLSIGN) ;
   serialdb->print("-") ;
   serialdb->println(SSID) ;
   serialdb->print(F("Version:"));
   serialdb->println(VERSION);

   delay(1000UL) ;
#ifdef OPTION_LCD
   lcd.clear();
   lcd.print(F("Version:"));
   lcd.setCursor(0,1);
   lcd.print(VERSION);
#endif
   delay(1000UL);
   // DRA818 Setup
   // must set these, then call WriteFreq
   //
   if ( dra.heartbeat() )  {
#ifdef OPTION_LCD
       lcd.clear();
       lcd.print(F("Txcr OK")) ;
#endif 
      serialdb->println(F("Transceiver Check OK")) ;
   } else {
#ifdef OPTION_LCD
       lcd.clear();
       lcd.print(F("Txcr Fail")) ;
#endif 
   serialdb->println(F("Transceiver Check Fail:")) ;
   serialdb->println(dra.response) ;
   } 

   delay(500UL);
   dra.setFreq(TXFREQ,RXFREQ);
   dra.setTXCTCSS(TXCTCSS);
   dra.setSquelch(SQUELCH);
   dra.setRXCTCSS(RXCTCSS);
   dra.setBW(BW); // 0 = 12.5k, 1 = 25k
   dra.writeFreq();
   delay(1000UL);
#ifdef OPTION_LCD
   lcd.clear();
   lcd.print(F("Freq Set"));
#endif 
   serialdb->println(F("Txcr Freq and Tones Set")) ;

   dra.setVolume(VOL);
   delay(500UL);
#ifdef OPTION_LCD
   lcd.clear();
   lcd.print(F("Vol Set"));
#endif
   serialdb->println(F("Txcr Vol Set")) ;
   dra.setVolume(VOL);

   dra.setFilters(FILTER_PREMPHASIS, FILTER_HIGHPASS, FILTER_LOWPASS);
   delay(500UL);

#ifdef OPTION_LCD
   lcd.clear();
   lcd.print(F("Filter Set"));
#endif 

   serialdb->println(F("Txcr Filters Set")) ;
   dra.setPTT(LOW);
   delay(500UL);

#ifdef OPTION_LCD
   lcd.clear();
   lcd.print(TXFREQ) ;
   lcd.print(F("/")) ;
   lcd.print(RXFREQ) ;
   lcd.setCursor(0,1) ;
   lcd.print(F(" TC: ")) ;
   lcd.print(0) ;
   lcd.print(F(" RC: ")) ;
   lcd.print(0) ;
#endif

   serialdb->print(F("Txcr TX/RX Freqs: ")) ;
   serialdb->print(TXFREQ) ;
   serialdb->print(F("/")) ;
   serialdb->print(RXFREQ) ;
   serialdb->print(F(" Tones: ")) ;
   serialdb->print(TXCTCSS) ;
   serialdb->print(F("/")) ;
   serialdb->println(RXCTCSS) ;

   delay(500UL) ;

   APRS_init(ADC_REFERENCE, OPEN_SQUELCH);
   APRS_setCallsign(CALLSIGN, SSID);
   APRS_setDestination(APRS_DEST,0);
   APRS_setPath1(PATH1,PATH1_SSID);
   APRS_setPath2(PATH2,PATH2_SSID);
   APRS_setPreamble(PREAMBLE);
   APRS_setTail(TAIL);
   APRS_setSymbol(SYMBOL);

#ifdef OPTION_LCD
   lcd.clear();
   lcd.print(F("APRS setup")) ;
#endif 
   serialdb->println(F("APRS TNC complete")) ;
   delay(500UL) ;
#ifdef OPTION_LCD
   lcd.clear();
   lcd.print(F("Setup Complete")) ;
#endif
   serialdb->println(F("Setup complete")) ;

}

char  lat[] =  "0000.00N" ;
char  lon[] = "00000.00W" ;
int alt = 0 ;



void loop()
{

   while(serialgps->available() > 0 ) {
       if(gps.encode(serialgps->read())) {
           if( gps.location.isValid()) {
              double dmh ; 
               // latitude
               if ( gps.location.rawLat().negative ) {
                   dmh = ddtodmh( -gps.location.lat() )  ;
                   dtostrf(dmh,7,2,lat) ;
                   lat[7] = 'S';
               } else {
                   dmh = ddtodmh( gps.location.lat() )  ;
                   dtostrf(dmh,7,2,lat) ;
                   lat[7] = 'N';
               }
               // longitude
               if ( gps.location.rawLng().negative ) {
                   dmh = ddtodmh( -gps.location.lng() )  ;
                   dtostrf(dmh,8,2,lon) ;
                   lon[8] = 'W';
               } else {
                   dmh = ddtodmh(gps.location.lng() )  ;
                   dtostrf(dmh,8,2,lon) ;
                   lon[8] = 'E';
               }

           }

           // altitude 
           if ( gps.altitude.isValid() ) {
               alt = (int) gps.altitude.feet() ;
           }

       }
   }

   if ( millis() - lastupdate > update_beacon ) {
     lastupdate = millis() ;
     // const char * lat =  "3742.44N" ;
     // const char * lon = "12157.54W" ;
     // 

     if ( gps.location.isValid() )  {

         if ( --beacon_init_count <= 0 ) {
            update_beacon = UPDATE_BEACON ;
            beacon_init_count = 10000 ;
         }
#ifdef OPTION_LCD
         lcd.clear();
         lcd.print(F("sending packet"));
#endif
         locationUpdate(lat,lon,alt,0,1,0,0) ;
         sent_count++ ;
         forcedisplay = true ; 
      }
   }

   processPacket() ;


   if ( forcedisplay ||  millis() - lastupdatedisplay > UPDATE_DISPLAY ) {
     lastupdatedisplay = millis() ;
         forcedisplay = false ;

#ifdef OPTION_LCD
         lcd.clear();
         lcd.print(F("s: ")) ;
         lcd.print(sent_count) ;
         lcd.print(F(" r: ")) ;
         lcd.print(recv_count) ;
         lcd.setCursor(0,1);

         if ( gps.location.isValid() )  {
            lcd.print(F("gps fix")) ;
         } else {
            lcd.print(F("gps no fix")) ;
         }
#endif
         serialdb->print(F("Stats:sent:")) ;
         serialdb->print(sent_count) ;
         serialdb->print(F(",recv:")) ;
         serialdb->print(recv_count) ;
         serialdb->print(F(",")) ;
         if ( gps.location.isValid() )  {
            serialdb->println(F("gps fix")) ;
         } else {
            serialdb->println(F("gps no fix")) ;
         }
         APRS_printSettings(serialdb) ;

     } 

}
