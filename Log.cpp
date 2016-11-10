
#include "Log.h"


Log::Log(void){} 

void Log::Log_Init(Stream *serial, LiquidCrystal_I2C *lcd, int lcdcol, int lcdrow){
    this->serial = serial ;
    for( int i ; i < 3 ; i++ ) { 
        strcpy(this->line[i],"") ;
    }
    this->lcd = lcd ;
    this->lcdcol = lcdcol ;
    this->lcdrow = lcdrow ;
    this->lcd->begin(lcdcol,lcdrow) ;
} 

void Log::Log_Init(Stream *serial){
    this->serial = serial ;

    for( int i = 0  ; i < 3 ; i++ ) { 
        strcpy(this->line[i],"") ;
    }

    this->lcd = NULL ;
    this->lcdcol = 0 ;
    this->lcdrow = 0 ;
} 

void Log::send(const char *msg) {

    this->serial->println(msg) ;

    if ( this->lcd != NULL) { 

        this->lcd->clear() ;

        // shift all lines up one.
        for( int i = 1 ; i < this->lcdrow  ; i++ ) { 
              strcpy(this->line[i-1],this->line[i]) ;
        }

        // copy the new line into the bottom (make sure overflow is handled) 
        strncpy(this->line[this->lcdrow-1], msg, this->lcdcol) ;
        this->line[lcdrow-1][this->lcdcol] = '\0' ; 

        // write all the lines to the lcd
        for( int i = 0 ; i < this->lcdrow  ; i++ ) { 
            this->lcd->setCursor(0,i) ; 
            this->lcd->print(this->line[i]) ;
        }

    }

}


void Log::send(const __FlashStringHelper * ifsh) {
    char outbuf[140] ;
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
