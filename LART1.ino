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
#include "Log.h"
#include "SD.h"

#define VERSION "Beta-0.999i"
#define ADC_REFERENCE REF_5V
#define DEBUG_APRS_SETTINGS false

// sets PTT pin (don't change, the pin is used by Port Manipulation later on) 
#define PTT 25

// usb serial port
HardwareSerial  * serialdb   = &Serial ;
// gps port
HardwareSerial  * serialgps  = &Serial1 ;
// DRA818 port
HardwareSerial * dra_serial = &Serial2 ;

char aprs_callsign[10] ;
int  aprs_ssid ;
char aprs_comment[40] ;

unsigned long update_beacon = UPDATE_BEACON_INIT ;

// set the outer beacon check loop to 30 seconds (do not change).
unsigned long update_check = 15000L ;
unsigned long lastcheck = 0 ;
unsigned long lastupdate = 0 ;
unsigned long lastupdatedisplay = 0 ;
bool forcedisplay = false ;

int sent_count=0 ;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7 , 3, POSITIVE); 

unsigned long bklighttimer = 30000UL ;


// GPS 
TinyGPSPlus gps ;

// DRA818 Transceiver Module
DRA818 dra(dra_serial, PTT);

// Log
Log mylog;



char buf[100] ;

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
   char tmpbuf[100] ; 
   if(gotPacket) {
      gotPacket = false ;
      recv_count++ ;

      if ( APRS_RECV ) {

          if( gps.date.isValid() ) {
              sprintf(tmpbuf,"%4d-%02d-%02d %02d:%02d:%02d", 
                      gps.date.year(), gps.date.month(), gps.date.day(),
                      gps.time.hour(), gps.time.minute(), gps.time.second()
                     ) ;
          } 

          mylog.send(tmpbuf) ;

          sprintf(tmpbuf,"%s-%d %s-%d",
                  incomingPacket.src.call, incomingPacket.src.ssid, 
                  incomingPacket.dst.call, incomingPacket.dst.ssid );

          mylog.send(tmpbuf) ;

          int i = 0 ; 
          for(i = 0; i < incomingPacket.rpt_count ; i++) {
            sprintf(tmpbuf," %s-%d",incomingPacket.rpt_list[i].call, incomingPacket.rpt_list[i].ssid);
            mylog.send(tmpbuf) ;
          }

          for(i = 0; i < incomingPacket.len; i++) {
               tmpbuf[i]  = (char) incomingPacket.info[i] ;
          }

          tmpbuf[i] = '\0' ;
          mylog.send(tmpbuf) ;
      }


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
   sprintf(comment,"/A=%06d%s",altitude,aprs_comment) ;
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


// readConfig functions

void rtrim(char * str) {
    int endc = strlen(str) - 1 ; 

    while ( ( str[endc] == ' ' ||
              str[endc] == '\t' ) 
                && endc >= 0 ) {
        str[endc] = '\0' ; 
        endc --  ;
    }
}



void setParam(int p, const char *buf) {

    switch(p) {
        case 1:
            strcpy(aprs_callsign,buf);
            break ;
        case 2:
            aprs_ssid = atoi(buf);
            break ;
        case 3:
            strcpy(aprs_comment,buf);
            break ;
        default:
            break ;
    } 
} 

bool readConfig(File *cf) {

    int ccnt = 0 ;
    int lcnt = 0 ; 
    char pbuf[40] ;
    bool cmode = false ;
    bool cferror = false ;


    pbuf[0] = '\0';

    while (cf->available()) {

        char c = cf->read() ; 

        // terminate the line and set the param on CR or LF
        if ( c == '\r' || c == '\n' ) {
            // if the next char is NL or CR, then slurp it up
            // should work for win, linux or mac edited files
            if ( cf->peek() == '\n' || cf->peek() == '\r') {
                cf->read() ;
            }
            rtrim(pbuf) ;
            if ( strlen(pbuf) > 0 ) setParam(++lcnt, pbuf) ;
            pbuf[0] = '\0' ;
            ccnt = 0  ;
            cmode = false  ;
            continue ;
        }

        if ( cmode ) continue ;


        if ( c ==  '#' ) {
            cmode = true ;
            continue ;
        }

        if ( ccnt > 38 )  {
            cmode = true ;
            continue ;
        } 

        pbuf[ccnt++] =  c ; 
        pbuf[ccnt] = '\0' ;

    }

    return !cferror ;
    
}

void setup()
{


   dra_serial->begin(DRA818_PORT_BAUD) ;
   serialgps->begin(GPS_PORT_BAUD) ;
   serialdb->begin(USB_PORT_BAUD) ;

   if ( !SD.begin(4) ) {
       mylog.send(F("sd init fail")) ; 
       delay(1000) ;
       exit(0) ;
   }

   lcd.begin(16,2);

   mylog.Log_Init(serialdb, &lcd) ;

   mylog.send(F("LART/1 APRS TRAK")) ;

   const char *fname = "/config.txt" ;

   File configFile  = SD.open(fname) ; 

   if (! configFile) {
       mylog.send(F("cfg read fail")) ; 
       delay(1000) ;
       exit(0) ;
   }

   if (!readConfig(&configFile) )  {
       mylog.send(F("cfg set fail")) ; 
       delay(1000) ;
       exit(0) ;
   }

   mylog.send(F("cfg ok")) ; 
   configFile.close() ;

   sprintf(buf,"CS:%s-%d",aprs_callsign,aprs_ssid);  
   mylog.send(buf) ;
   delay(1000UL) ;

   sprintf(buf,"Ver: %s",VERSION);  
   mylog.send(buf);
   delay(1000UL);

   // DRA818 Setup
   // must set these, then call WriteFreq
   //
   if ( dra.heartbeat() )  {
       mylog.send(F("Txcr OK Chk")) ;
   } else {
       mylog.send(F("Txcr Fail Chk")) ;
       delay(1000) ;
       exit(0) ;
   } 
   delay(500UL);

   // set freq and tones
   dra.setFreq(TXFREQ,RXFREQ);
   dra.setTXCTCSS(TXCTCSS);
   dra.setSquelch(SQUELCH);
   dra.setRXCTCSS(RXCTCSS);
   dra.setBW(BW); // 0 = 12.5k, 1 = 25k

   char fbuf[12] ;
   if ( dra.writeFreq() ) {
       dtostrf(TXFREQ, 8, 4, fbuf) ;
       sprintf(buf,"Freq TX:%s",fbuf) ;
       mylog.send(buf);
       dtostrf(RXFREQ, 8, 4, fbuf) ;
       sprintf(buf,"Freq RX:%s",fbuf) ;
       mylog.send(buf);
    } else {
        mylog.send(F("Freq Fail"));
       delay(1000) ;
       exit(0) ;
    }
   delay(1000UL);

   if ( dra.setVolume(VOL) ) {
       sprintf(buf,"Vol: %d", VOL) ;
       mylog.send(buf);
    } else {
        mylog.send(F("Vol Fail"));
       delay(1000) ;
       exit(0) ;
    } 

   delay(500UL);

   if ( dra.setFilters(FILTER_PREMPHASIS, FILTER_HIGHPASS, FILTER_LOWPASS) ) {
      sprintf(buf,"Filter: %d %d %d",FILTER_PREMPHASIS,FILTER_HIGHPASS,FILTER_LOWPASS) ;
      mylog.send(buf);
   } else { 
      mylog.send( F("Filter Fail") );
       delay(1000) ;
       exit(0) ;
   }

   delay(500UL);

   dra.setPTT(LOW);
   delay(500UL);

   mylog.send(F("APRS setup")) ;
   APRS_init(ADC_REFERENCE, OPEN_SQUELCH);
   APRS_setCallsign(aprs_callsign, aprs_ssid);
   APRS_setDestination(APRS_DEST,0);
   APRS_setPath1(PATH1,PATH1_SSID);
   APRS_setPath2(PATH2,PATH2_SSID);
   APRS_setPreamble(PREAMBLE);
   APRS_setTail(TAIL);
   APRS_setSymbol(SYMBOL);
   if (APRS_BEACON) APRS_printSettings(serialdb) ;
   delay(500UL) ;

   sprintf(buf,"Receive Mode %d",APRS_RECV) ;
   mylog.send(buf) ;
   delay(500UL) ;

   sprintf(buf,"Beacon Mode %d",APRS_BEACON) ;
   mylog.send(buf) ;
   sprintf(buf,"Smart Mode %d",SMART_BEACON) ;
   mylog.send(buf) ;
   delay(500UL) ;

   mylog.send(F("Setup Complete")) ;
   delay(500UL) ;
   mylog.send(F("Ready")) ;

}

char  lat[] =  "0000.00N" ;
char  lon[] = "00000.00W" ;
int alt = 0 ;
bool position_changed = true ;
char  prevlat[] =  "0000.00N" ;
char  prevlon[] = "00000.00W" ;



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
               

               // position change
               //  1 100th-second = 60 feet @ 38 lat
               //  1 100th-second = 48 feet @  lon (fairly constant) 
               // set a precision value to compare 
               if ( strncmp(lat, prevlat, POSITION_CHANGE_PRECISION) != 0 ) {
                   strcpy(prevlat,lat) ;
                   position_changed = true ; 
               }
               if ( strncmp(lon, prevlon, POSITION_CHANGE_PRECISION + 1 ) != 0 ) {
                   strcpy(prevlon,lon) ;
                   position_changed = true ; 
               }


           }

           // altitude 
           if ( gps.altitude.isValid() ) {
               alt = (int) gps.altitude.feet() ;
           }

       }
   }


   // outer update check loop 
   if (APRS_BEACON && ( millis() - lastcheck) > update_check ) {
       lastcheck = millis () ;

       // don't do anything unless gps is valid
       if ( gps.location.isValid() )  {

           // set the update_beacon interval
           // for smart beacon mode , set it based on position changes
           // for dumb beacon mode, uses UPDATE_BEACON_INIT first time through, then UPDATE_BEACON
           if ( SMART_BEACON ) {
              if ( position_changed ) {
                 update_beacon = UPDATE_BEACON_MOVING ; 
                 // add this to make sure the location is sent first time thru the loop
                 if (millis() > UPDATE_BEACON_MOVING * 2) {
                    position_changed = false ;
                 } 
              } else {
                 update_beacon = UPDATE_BEACON ;
              } 
           } else { 
              // add this to make sure location is sent first time thru the loop
              if ( millis() > UPDATE_BEACON_INIT * 2 ) {
                 update_beacon = UPDATE_BEACON ;
              }
           } 

           // sprintf(buf,"up beacon %lu",update_beacon) ;
           // mylog.send(buf) ;


           // if its time, send out the location
           if ( (millis() - lastupdate) > update_beacon ) {
              lastupdate = millis() ;
              mylog.send(F("sending loc"));
              locationUpdate(lat,lon,alt,0,1,0,0) ;
              sent_count++ ;
              forcedisplay = true ; 

          }
       }
   }

   processPacket() ;

   // need the extra logic check, to avoid negative result errors with unsigned longs
   if ( ( bklighttimer < millis() ) 
           &&  ( millis() - bklighttimer)  > BKLIGHT_INTERVAL ) { 
       lcd.setBacklight(LOW) ; 
   }

   if ( forcedisplay ||  (millis() - lastupdatedisplay) > UPDATE_DISPLAY ) {
     lastupdatedisplay = millis() ;
         forcedisplay = false ;

         sprintf(buf,"s:%d r:%d",sent_count,recv_count) ;
         mylog.send(buf) ;

         if ( gps.location.isValid() )  {
             char datebuf[20] ; 
             sprintf(datebuf,"%4d-%02d-%02d %02d:%02d:%02d", 
                  gps.date.year(), gps.date.month(), gps.date.day(),
                  gps.time.hour(), gps.time.minute(), gps.time.second()
                 ) ;
            mylog.send(datebuf) ;
         } else {
            mylog.send(F("gps no fix")) ;
         }

         if ( DEBUG_APRS_SETTINGS) APRS_printSettings(serialdb) ;

     } 

}
