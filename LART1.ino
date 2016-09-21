#include <LiquidCrystal_I2C.h>
#include <LibAPRS.h>
#include <TinyGPSplus.h>
#include <DRA818.h>

#define VERSION "Beta-0.2"

#define ADC_REFERENCE REF_5V

#define OPEN_SQUELCH false

#define CALLSIGN "NOCALL"
#define SSID 5
#define PTT 3 
#define FREQ 144.39
#define UPDATE_BEACON 60000

long lastupdate = 0 ;

int sent_count=0 ;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7 , 3, POSITIVE); 

DRA818 dra(&Serial, PTT);

boolean gotPacket = false ;
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

      lcd.clear();
      lcd.print(F("Pkt Rcv S:")) ; 
      lcd.print(incomingPacket.src.call) ; 
      lcd.print(F("-")) ; 
      lcd.print(incomingPacket.src.ssid) ; 
      lcd.setCursor(0,1);
      lcd.print(F("D:")) ; 
      lcd.print(incomingPacket.dst.call) ; 
      lcd.print(F("-")) ; 
      lcd.print(incomingPacket.dst.ssid) ; 

      free(packetData) ;
   }
}


void locationUpdate(char *lat, char *lon,int height = 0 ,int power=1, int gain=1, int dir=0)
{
   APRS_setLat(lat);
   APRS_setLon(lon);
   APRS_setPower(power);
   APRS_setHeight(height);
   APRS_setGain(gain);
   APRS_setDirectivity(dir);

   char *comment = "LART-1 Tracker" ;
   APRS_sendLoc(comment, strlen(comment));
}


void setup()
{
   Serial.begin(9600) ;

   lcd.begin(16,2);
   lcd.clear();
   lcd.print("LART/1 APRS BEAC"); 
   lcd.setCursor(0,1);
   lcd.print("Call: ") ;
   lcd.print(CALLSIGN) ;
   lcd.print("-") ;
   lcd.print(SSID) ;

   delay(3000) ;

   // DRA818 Setup
   dra.setFreq(FREQ);
   dra.setTXCTCSS(0);
   dra.setSquelch(2);
   dra.setRXCTCSS(0);
   dra.writeFreq();
   dra.setVolume(6);
   dra.setFilters(true, true, true);

   lcd.clear();
   lcd.print("F:") ;
   lcd.print(FREQ) ;
   lcd.setCursor(0,1) ;
   lcd.print(" TC:") ;
   lcd.print(0) ;
   lcd.print(" RC:") ;
   lcd.print(0) ;
   delay(3000) ;

   APRS_init(ADC_REFERENCE, OPEN_SQUELCH);
   APRS_setCallsign(CALLSIGN, SSID);
   // APRS_setDestination();
   APRS_setPath1("WIDE1",1);
   APRS_setPath2("WIDE2",1);
   // APRS_printSettings();



   lcd.clear();
   lcd.print("APRS setup") ;
   delay(3000) ;

   lcd.clear();
   lcd.print("Beacon Mode Start") ;
   delay(3000) ;

}


void loop()
{

   processPacket() ;

   if ( millis() - lastupdate > UPDATE_BEACON ) {
      lastupdate = millis() ;
         // this sends a fixed location only
         // beta beta beta
         // dublin QTH
         char * lat =  "3742.26N" ;
         char * lon = "12157.38W" ;
         int h = 234 ;

         locationUpdate(lat,lon,h) ;
         sent_count++ ;
         lcd.clear();
         lcd.print("loc pkg sent: ") ;
         lcd.print(sent_count) ;
   }

}
