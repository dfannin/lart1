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

#define VERSION "Beta-0.999p"
#define ADC_REFERENCE REF_5V
#define DEBUG_APRS_SETTINGS false

//  define APRS_PTT_LOW for TX=LOW hardware
//  or APRS_PTT_HIGH for TX=HIGH hardware
#define APRS_PTT_TX_LOW 

// status led 
//
#define LED_TX 48
#define LED_RX 47
#define LED_GPS 36
//
// Transceiver Ctl pins
// PTT pin (don't change, the pin is used by Port Manipulation later on) 
#define PTT 25

// PowerDown Logic Pin (puts DRA818 into sleep mode)
#define PD 35 

// usb serial port
HardwareSerial  * serialdb   = &Serial ;
// gps port
HardwareSerial  * serialgps  = &Serial1 ;
// DRA818 port
HardwareSerial * dra_serial = &Serial2 ;

// setting the default parameters
char aprs_callsign[10] ;
int  aprs_ssid ;
char aprs_comment[40] ;
bool aprs_beacon = APRS_BEACON  ;
bool aprs_recv = APRS_RECV ;
bool notsosmart_beacon = NOTSOSMART_BEACON ; 
bool sdlog_write = SDLOG_WRITE ;
double txfreq = TXFREQ ;  
double rxfreq = RXFREQ ;
int txctcss = TXCTCSS ;
int rxctcss = RXCTCSS ;
int volume = VOL ;
int squelch = SQUELCH ;

unsigned long update_beacon = UPDATE_BEACON_INIT ;

unsigned long update_beacon_fixed = UPDATE_BEACON_FIXED ;
unsigned long update_beacon_moving = UPDATE_BEACON_MOVING ;

// set the outer beacon check loop to 30 seconds (do not change).
unsigned long update_check = 30000L ;
unsigned long lastcheck = 0 ;
unsigned long lastupdate = 0 ;
unsigned long lastupdatedisplay = 0 ;
unsigned long lastgpsupdate = 0 ;
bool forcedisplay = false ;

bool gpsfix = false ;

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
   if(gotPacket) {
      char outbuf[180] ; 
      char tmpbuf[100];

      gotPacket = false ;
      recv_count++ ;

      outbuf[0] = '\0';
      strcpy(tmpbuf,"0000-00-00 00:00:00 ") ;

      if ( aprs_recv ) {

          if( gps.date.isValid() ) {
              sprintf(tmpbuf,"%4d-%02d-%02d %02d:%02d:%02d ", 
                      gps.date.year(), gps.date.month(), gps.date.day(),
                      gps.time.hour(), gps.time.minute(), gps.time.second()
                     ) ;
          } 


          strcat(outbuf,tmpbuf) ;

          sprintf(tmpbuf,"%s-%d,%s-%d",
                  incomingPacket.src.call, incomingPacket.src.ssid, 
                  incomingPacket.dst.call, incomingPacket.dst.ssid );

          strcat(outbuf,tmpbuf) ;

          int i = 0 ; 
          for(i = 0; i < incomingPacket.rpt_count ; i++) {
            sprintf(tmpbuf," %s-%d",incomingPacket.rpt_list[i].call, incomingPacket.rpt_list[i].ssid);
            strcat(outbuf,tmpbuf) ;
          }

          int maxlen = incomingPacket.len ; 

          if ( strlen(outbuf) + maxlen > 179 ) maxlen = 179 - strlen(outbuf) ;

          for(i = 0; i < maxlen ; i++) {
               tmpbuf[i]  = (char) incomingPacket.info[i] ;
          }

          strcat(outbuf,tmpbuf) ;
          mylog.send(outbuf) ;
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



void mytrim(char * str) {
    int i ; 
    int begin = 0 ;
    int end = strlen(str) - 1 ; 
    while ( isspace((unsigned char) str[begin])) begin++ ;
    while (( end >= begin) && isspace((unsigned char) str[end])) end-- ;
    for( i = begin; i<=end ; i++ ) str[i-begin] = str[i] ;
    str[i-begin] = '\0';
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
        case 4:
            aprs_beacon =(bool)  atoi(buf) ;
            break ;
        case 5:
            aprs_recv = (bool) atoi(buf) ;
            break ;
        case 6:
            notsosmart_beacon = (bool) atoi(buf) ;
            break ;
        case 7:
            sdlog_write = (bool) atoi(buf) ;
            break ;
        case 8:
            txfreq =  atof(buf) ;
            break ;
        case 9:
            rxfreq =  atof(buf) ;
            break ;
        case 10:
            txctcss =  atoi(buf) ;
            break ;
        case 11:
            rxctcss =  atoi(buf) ;
            break ;
        case 12:
            volume =  atoi(buf) ;
            break ;
        case 13:
            squelch =  atoi(buf) ;
            break ;
        case 25:
            update_beacon_fixed =  atol(buf) ;
            break ;
        case 26:
            update_beacon_moving =  atol(buf) ;
            break ;
        case 0: 
            break ;
        default:
            break ;
    } 
} 


bool readConfig(File *cf) {

    int ccnt = 0 ;
    int pnum = 0 ;  
    char pbuf[50] ;
    char pnumbuf[10] ;

    bool pnmode = true ;
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
            mytrim(pbuf) ;

            if ( strlen(pbuf) > 0 || pnum != 0 ) setParam(pnum, pbuf) ;
            ccnt = 0  ;
            pnum = 0 ; 
            pbuf[0] = '\0' ;
            pnumbuf[0]= '\0';
            cmode = false  ;
            pnmode = true ;
            continue ;
        }


        if ( cmode ) { 
            continue ;
        }


        if ( c ==  '#'  || ccnt > 48 ) {
            cmode = true ;
            continue ;
        }


        if (pnmode) {
            // if the parameter number is not found, then punt and ignore the line
            if ( ccnt > 9 ) {
                cmode = true ;
                cferror = true ; 
                pnmode = false ;
                ccnt = 0 ;
                continue ;
            }
            // when the number is read
            // move to reading the parameter value
            if ( c == ',' ) {
                pnmode = false ;
                ccnt = 0 ; 
                pnum  = atoi(pnumbuf) ;
                continue ;
            } 

            // read the parameter number and store in a buffer

            pnumbuf[ccnt++] = c ; 
            pnumbuf[ccnt] = '\0' ;
            continue ; 

        }

        pbuf[ccnt++] =  c ; 
        pbuf[ccnt] = '\0' ;

    }

    return !cferror ;
    
}


// blinks the LEDs on startup
//
void testleds(void) {
   pinMode(LED_TX,OUTPUT) ;
   pinMode(LED_RX,OUTPUT) ;
   pinMode(LED_GPS,OUTPUT) ;

   digitalWrite(LED_TX,HIGH) ;
   digitalWrite(LED_RX,LOW);
   digitalWrite(LED_GPS,LOW) ;
   delay(1000) ;
   digitalWrite(LED_TX,LOW);
   digitalWrite(LED_RX,HIGH) ;
   delay(1000) ;
   digitalWrite(LED_RX,LOW) ;
   digitalWrite(LED_GPS,HIGH) ;
   delay(1000) ;
   digitalWrite(LED_GPS,LOW) ;
} 



void setup()
{
   // initializes the PTT to high or low
   // set to HIGH, if PTT is LOW to TX
   // set to LOW   if PTT is HIGH to TX
   pinMode(PTT, OUTPUT) ;
   digitalWrite(PTT, HIGH ) ;
   // initializes PD pin to HiGH for DRA818
   // keeps the DRA818 active
   pinMode(PD, OUTPUT) ;
   digitalWrite(PD, HIGH) ;

   strcpy(aprs_comment,APRS_COMMENT) ;

   testleds() ;

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
       mylog.send(F("no cfg file")) ; 
       delay(1000) ;
       exit(0) ;
   }

   if (!readConfig(&configFile) )  {
       mylog.send(F("cfg read fail")) ; 
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
   dra.setFreq(txfreq,rxfreq);
   dra.setTXCTCSS(txctcss);
   dra.setSquelch(squelch);
   dra.setRXCTCSS(rxctcss);
   dra.setBW(BW); // 0 = 12.5k, 1 = 25k

   char fbuf[12] ;
   if ( dra.writeFreq() ) {
       dtostrf(txfreq, 8, 4, fbuf) ;
       sprintf(buf,"Freq TX:%s",fbuf) ;
       mylog.send(buf);
       dtostrf(rxfreq, 8, 4, fbuf) ;
       sprintf(buf,"Freq RX:%s",fbuf) ;
       mylog.send(buf);
    } else {
        mylog.send(F("Freq Fail"));
       delay(1000) ;
       exit(0) ;
    }
   delay(1000UL);

   if ( dra.setVolume(volume) ) {
       sprintf(buf,"Vol: %d", volume) ;
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

   dra.setPTT(PTT_OFF);
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
   if (aprs_beacon) APRS_printSettings(serialdb) ;
   delay(500UL) ;

   sprintf(buf,"Receive Mode %d",aprs_recv) ;
   mylog.send(buf) ;

   sprintf(buf,"Beacon Mode %d",aprs_beacon) ;
   mylog.send(buf) ;
   delay(500UL) ;

   sprintf(buf,"NotSmart Mode %d",notsosmart_beacon) ;
   mylog.send(buf) ;
#ifdef APRS_PTT_TX_LOW 
   sprintf(buf,"PTT Logic LOW") ;
#else
   sprintf(buf,"PTT Logic HIGH") ;
#endif 
   mylog.send(buf) ;
   delay(500UL) ;

   mylog.send(F("Setup Complete")) ;
   delay(500UL) ;
   mylog.send(F("Ready")) ;

}

char  lat[] =  "0000.00N" ;
char  lon[] = "00000.00W" ;
double latd = 0.0 ;
double lond = 0.0 ;

char  prevlat[] =  "0000.00N" ;
char  prevlon[] = "00000.00W" ;
double prevlatd = 0.0 ;
double prevlond = 0.0 ;

int alt = 0 ;

bool position_changed = false ;
bool lastpositionsent = false ;



void loop()
{

// process the gps packets
while(serialgps->available() > 0 ) { 
       
   if ( gps.encode( serialgps->read() ) )  

       if( gps.location.isValid() ) {
          double dmh ; 
           latd = gps.location.lat() ;
           lond = gps.location.lng() ;
           // latitude
           if ( gps.location.rawLat().negative ) {
               dmh = ddtodmh( -latd )  ;
               dtostrf(dmh,7,2,lat) ;
               lat[7] = 'S';
           } else {
               dmh = ddtodmh( latd )  ;
               dtostrf(dmh,7,2,lat) ;
               lat[7] = 'N';
           }
           // longitude
           if ( gps.location.rawLng().negative ) {
               dmh = ddtodmh( -lond )  ;
               dtostrf(dmh,8,2,lon) ;
               lon[8] = 'W';
           } else {
               dmh = ddtodmh(lond )  ;
               dtostrf(dmh,8,2,lon) ;
               lon[8] = 'E';
           }

       } // end lat lon update 

           // altitude 
       if ( gps.altitude.isValid() ) {
           alt = (int) gps.altitude.feet() ;
       } // end altitude update 

} // end readgps 

   // check if position has changed
   if ( ( millis() - lastgpsupdate )   >   30000  ) { 
           lastgpsupdate = millis() ;
           // position change
           //  1 100th-second = 60 feet @ 38 lat
           //  1 100th-second = 48 feet @  lon (fairly constant) 
           // for precision "6" , 13 miles per hour
           /*
           // calculate distance (in miles) 
           double distM  = TinyGPSPlus.distanceBetween(latd,lond, prevlatd, prevlond) * 0.6214 ; 

           // convert to MPH
           double mph = ( distM / 15 ) * 3600 ;  

           if ( mph > 8.0 ) {

           }
           */
           

           if ( strncmp(lat, prevlat, POSITION_CHANGE_PRECISION) != 0 ) {
               strcpy(prevlat,lat) ;
               prevlatd = latd ;
               position_changed = true ; 
           } 


           if ( strncmp(lon, prevlon, POSITION_CHANGE_PRECISION + 1 ) != 0 ) {
               strcpy(prevlon,lon) ;
               prevlond = lond ;
               position_changed = true ; 
           }
   }




   // outer update check loop 
   if (aprs_beacon  && ( millis() - lastcheck) > update_check ) {
       lastcheck = millis () ;

       // don't do anything unless gps is valid
       if ( gps.location.isValid() )  {

           // set the update_beacon interval
           // for not so smart beacon mode , set it based on position changes
           // for dumb beacon mode, uses UPDATE_BEACON_INIT first time through, then UPDATE_BEACON_FIXED
           //
           // Notsosmart algorithm
           // if ( position changed ) 
           // then interval = moving, sent out next time 
           // else if ( lastposition was sent) 
           //      then interval = fixed
           //      else keep the previous one (fixed or moving) 
           // 
           //
           if ( notsosmart_beacon ) {
              if ( position_changed ) {
                 update_beacon = update_beacon_moving ; 
                 position_changed = false ;
                 lastpositionsent = false ;
              } else {
                 if (lastpositionsent) { 
                    update_beacon = update_beacon_fixed ;
                    lastpositionsent = false ;
                 } 
              } 
           } else { 
              // add this to make sure the location is sent first time thru the loop
              if (lastpositionsent) {
                 update_beacon = update_beacon_fixed ;
                 lastpositionsent = false ;
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
              lastpositionsent = true ;
              forcedisplay = true ; 

          }
       }
   }

   // read (and output received packets
   processPacket() ;

   // check the backlight
   // need the extra logic check, to avoid negative result errors with unsigned longs
   if ( ( bklighttimer < millis() ) 
           &&  ( millis() - bklighttimer)  > BKLIGHT_INTERVAL ) { 
       lcd.setBacklight(LOW) ; 
   }

   // update the display 
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

         // update gps led

         digitalWrite(LED_GPS,gps.location.isValid()) ;

     } 

}
