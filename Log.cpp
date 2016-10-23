
#include "Log.h"


Log::Log(void){} 

void Log::Log_Init(Stream *serial, LiquidCrystal_I2C *lcd){
    this->serial = serial ;
    this->lcd = lcd ;
    strcpy(this->line1,"") ;
} 

void Log::send(const char *msg) {

    this->serial->println(msg) ;

    this->lcd->clear() ;

    char lcdmsg[LCDCOL+1];
    strncpy(lcdmsg,msg,LCDCOL+1) ;
    lcdmsg[LCDCOL] = '\0' ; 

    if ( strlen(this->line1) == 0 )  {
        this->lcd->print(lcdmsg) ;
    } else { 
        this->lcd->print(this->line1) ;
        this->lcd->setCursor(0,1) ; 
        this->lcd->print(lcdmsg) ;
    }

    strcpy(this->line1,lcdmsg) ;

}

void Log::send(const __FlashStringHelper * ifsh) {
    char outbuf[100] ;
    const char PROGMEM *p = (const char PROGMEM *) ifsh ;
    size_t i = 0 ;
    while(1) {
        outbuf[i] = pgm_read_byte(p++) ;
        if( outbuf[i] == 0 ) break ;
        i++ ;
        // if buf will overflow, truncate
        if ( i >= 99 ) {
            outbuf[i] = '\0';
            break ;
        }
    }

    this->send(outbuf) ; 
    
}
