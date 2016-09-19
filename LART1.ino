#include <LiquidCrystal_I2C.h>
#include <LibAPRS.h>
#include <TinyGPSplus.h>

#define VERSION "Beta-0.1"

#define ADC_REFERENCE REF_5V

#define OPEN_SQUELCH false

#define CALLSIGN "W1AW"
#define SSID 5

#define UPDATE_BEACON 10000
long lastupdate = 0 ;

int sent_count=0 ;


LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7 , 3, POSITIVE); 

#define BUTTON_SENDLOC_PIN A3
#define BUTTON_SENDLOC_WINDOW 300
int button_sendloc_state = 0 ;

void check_sendloc()
{
   static long button_sendloc_lastupdate = 0 ;

   if ( (millis() - button_sendloc_lastupdate) > BUTTON_SENDLOC_WINDOW ) {
      button_sendloc_lastupdate = millis() ;
      button_sendloc_state = digitalRead(BUTTON_SENDLOC_PIN) ;
   }
}

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
      Serial.print(F("Received APRS packet. SRC: "));
      Serial.print(incomingPacket.src.call);
      Serial.print(F("-"));
      Serial.print(incomingPacket.src.ssid);
      Serial.print(F(". DST: "));
      Serial.print(incomingPacket.dst.call);
      Serial.print(F("-"));
      Serial.print(incomingPacket.dst.ssid);
      Serial.print(F(". Data: "));

      for (int i = 0; i < incomingPacket.len; i++) {
         Serial.write(incomingPacket.info[i]);
      }
      Serial.println("");
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

   char *comment = "LART/1 Beacon" ;
   APRS_sendLoc(comment, strlen(comment));
}


void setup()
{
   Serial.begin(9600) ;

   lcd.begin(16,2);
   lcd.home();
   lcd.print("LART/1 APRS BEAC"); 
   lcd.setCursor(0,1);
   lcd.print("Call: ") ;
   lcd.print(CALLSIGN) ;
   lcd.print("-") ;
   lcd.print(SSID) ;


   APRS_init(ADC_REFERENCE, OPEN_SQUELCH);
   APRS_setCallsign(CALLSIGN, SSID);
   APRS_printSettings();

   pinMode(BUTTON_SENDLOC_PIN, INPUT_PULLUP);

   Serial.println(F("LART1 APRS Beacon Start")) ;
   Serial.print(F("Version: ")) ;
   Serial.println(VERSION) ;
   Serial.print(F("Callsign ")) ;
   Serial.print(CALLSIGN) ;
   Serial.print(F(" SSID: ")) ;
   Serial.println(SSID) ;


}


void loop()
{


   processPacket() ;


   if ( millis() - lastupdate > UPDATE_BEACON ) {
      lastupdate = millis() ;
      if ( !button_sendloc_state ) {
         // this sends a fixed location only
         // beta beta beta
         // dublin QTH
         char * lat =  "3742.26N" ;
         char * lon = "12157.38W" ;
         int h = 234 ;

         locationUpdate(lat,lon,h) ;
         sent_count++ ;
         Serial.print(F("sent lat: ")) ;
         Serial.print(lat) ;
         Serial.print(F(" lon: "));
         Serial.print(lon);
         Serial.print(F(" height: ")) ;
         Serial.println(h) ;
         Serial.print(F(" count: ")) ;
         Serial.println(sent_count) ;
      } else {
         Serial.print(F("sent location disabled\n")) ;
      }

      lcd.clear();
      lcd.home();
      lcd.print("Send Location ") ;
      lcd.print(sent_count) ;
   }




}
