
#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__) 
#define TARGET_CPU m328p
#endif

#if defined(__AVR_ATmega2560__) 
#define TARGET_CPU mega2560
#endif

#include "Arduino.h" 
#include "LiquidCrystal_I2C.h"
#include "LibAPRS.h"
#include "TinyGPSplus.h"
#include "DRA818.h"

#if TARGET_CPU == mega2560
#define MEGA 1
#endif

#ifndef MEGA 
#include "SoftwareSerial.h"
#endif

#define VERSION "Beta-0.996m"

#define ADC_REFERENCE REF_5V
#define OPEN_SQUELCH false

#define OPTION_LCD  true
#define CALLSIGN "KK6DF"
#define SSID 2

#ifdef MEGA
#define PTT 25
#else
#define PTT 3 
#endif

#define FREQ 144.39
#define UPDATE_BEACON 300000L
#define UPDATE_BEACON_INIT  60000L

#define NOP __asm__ __volatile__("nop\n\t")

#ifdef MEGA
HardwareSerial  * serialgps  = &Serial1 ;
HardwareSerial  * serialdb   = &Serial ;
#else
HardwareSerial *serialgps = &Serial; 
#endif


int beacon_init_count = 1 ;

unsigned long update_beacon = UPDATE_BEACON_INIT ;

unsigned long lastupdate = 0 ;

#define UPDATE_DISPLAY 60000L 
unsigned long lastupdatedisplay = 0 ;
bool forcedisplay = false ;

int sent_count=0 ;

#ifdef OPTION_LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7 , 3, POSITIVE); 
#endif 

// GPS setup

TinyGPSPlus gps ;

// DRA818 setup
#ifdef MEGA 
HardwareSerial * dra_serial = &Serial2 ;
#else
#define DRA_RXD A1
#define DRA_TXD A2
SoftwareSerial dra818_serial(DRA_RXD,DRA_TXD);
SoftwareSerial * dra_serial  = &dra818_serial;
#endif

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
#ifdef MEGA
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
#endif

      free(packetData) ;

#ifdef MEGA
      // serialdb->print(F("Free Ram:")) ;
      // serialdb->println(freeMemory()) ;
#endif

   }
}



void locationUpdate(const char *lat, const char *lon,int altitude, int height = 0 ,int power=1, int gain=0, int dir=0)
{
    char comment[43] ; // max of 36 chars with PHG data extension, 43 chars  without
   APRS_setLat(lat);
   APRS_setLon(lon);
   APRS_setHeight(height);
   APRS_setPower(power);
   APRS_setGain(gain);
   APRS_setDirectivity(dir);
   sprintf(comment,"/A=%06dLART-1 Tracker beta",altitude) ;
   // sprintf(comment,"LART-1 Tracker") ;
   // strcpy(comment,"LART-1 Tracker") ;
   // char * comment = "LART-1 Tracker" ;

   serialdb->print(F("comment added: ")) ;
   serialdb->println(comment) ; 

   APRS_sendLoc(comment, strlen(comment));
}


void mydelay(unsigned long msec) {
    unsigned long future = millis() + msec ;
    while ( (future - millis()) > 0 ) {
        NOP ;
    } ;
    return ;  
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

#ifdef MEGA
   dra_serial->begin(9600) ;
   serialgps->begin(4800) ;
   serialdb->begin(9600) ;
#else
   dra_serial->begin(9600);
   serialgps->begin(4800) ;
#endif

#ifdef MEGA
   serialdb->println(F("LART/1 APRS Beacon")) ;
   serialdb->print(CALLSIGN) ;
   serialdb->print("-") ;
   serialdb->println(SSID) ;
   serialdb->print(F("Version:"));
   serialdb->println(VERSION);
#endif 

   mydelay(1000UL) ;
#ifdef OPTION_LCD
   lcd.clear();
   lcd.print(F("Version:"));
   lcd.setCursor(0,1);
   lcd.print(VERSION);
#endif
   mydelay(1000UL);
   // DRA818 Setup
   // must set these, then call WriteFreq
   //
   if ( dra.heartbeat() )  {
#ifdef OPTION_LCD
       lcd.clear();
       lcd.print(F("TXCR OK")) ;
#endif 
#ifdef MEGA
   serialdb->println(F("TXCR OK")) ;
#endif
   } else {
#ifdef OPTION_LCD
       lcd.clear();
       lcd.print(F("TXCR Fail")) ;
#endif 
#ifdef MEGA
   serialdb->println(F("TXCR Fail:")) ;
   serialdb->println(dra.response) ;
#endif
   } 

   mydelay(1000UL);
   dra.setFreq(FREQ);
   dra.setTXCTCSS(0);
   dra.setSquelch(4);
   dra.setRXCTCSS(0);
   dra.setBW(0); // 0 = 12.5k, 1 = 25k
   dra.writeFreq();
   mydelay(1000UL);
#ifdef OPTION_LCD
   lcd.clear();
   lcd.print(F("Freq Set"));
#endif 
   dra.setVolume(5);
   mydelay(1000UL);
#ifdef OPTION_LCD
   lcd.clear();
   lcd.print(F("Vol Set"));
#endif
   dra.setFilters(false, true, true);
   mydelay(1000UL);
   lcd.clear();
   lcd.print(F("Filter Set"));
   dra.setPTT(LOW);
   mydelay(1000UL);
#ifdef OPTION_LCD
   lcd.clear();
   lcd.print(F("F:")) ;
   lcd.print(FREQ) ;
   lcd.setCursor(0,1) ;
   lcd.print(F(" TC: ")) ;
   lcd.print(0) ;
   lcd.print(F(" RC: ")) ;
   lcd.print(0) ;
#endif
   mydelay(1000UL) ;

   APRS_init(ADC_REFERENCE, OPEN_SQUELCH);
   APRS_setCallsign(CALLSIGN, SSID);
   APRS_setDestination("APZMDM",0);
   APRS_setPath1("WIDE1",1);
   APRS_setPath2("WIDE2",1);
   APRS_setPreamble(350L);
   APRS_setTail(50L);
   APRS_setSymbol('n');

#ifdef OPTION_LCD
   lcd.clear();
   lcd.print(F("APRS setup")) ;
   mydelay(1000UL) ;
   lcd.clear();
   lcd.print(F("Setup Complete")) ;
#endif
#ifdef MEGA
   serialdb->println(F("Setup complete")) ;
#endif

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

         if ( beacon_init_count-- <= 0 ) {
            update_beacon = UPDATE_BEACON ;
            beacon_init_count = 10000 ;
         }
#ifdef OPTION_LCD
         lcd.clear();
         lcd.print(F("sending packet"));
#endif
#ifdef MEGA
         serialdb->println(F("Sending location packet:")) ;
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
#ifdef MEGA
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
#endif 

     } 

}
