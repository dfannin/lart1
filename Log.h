

#ifndef LOG_H
#define LOG_H

#include "TinyGPSplus.h"
#include "LiquidCrystal_I2C.h"
#include <WString.h>

#define LCDCOL 16 

class Log {
    public:
        Log(void) ;
        void Log_Init(Stream *serial,LiquidCrystal_I2C *lcd) ;
        void send(const char * msg) ;
        void send(const __FlashStringHelper * ifsh) ;
        char line1[LCDCOL+1] ;
        Stream * serial; 
        LiquidCrystal_I2C * lcd;
} ;

#endif 
