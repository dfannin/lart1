

#ifndef LOG_H
#define LOG_H

#include "LART1_Settings.h"
#include "TinyGPSplus.h"
#include "LiquidCrystal_I2C.h"
#include <WString.h>

class Log {
    public:
        Log(void) ;
        void Log_Init(Stream *serial,LiquidCrystal_I2C *lcd, int lcdcol=16, int lcdrow=2) ;
        void Log_Init(Stream *serial) ;
        void send(const char * msg) ;
        void send(const __FlashStringHelper * ifsh) ;
        char line[4][21] ; // max display is 20x4 
        Stream * serial; 
        LiquidCrystal_I2C * lcd;
        int lcdcol;
        int lcdrow;
} ;

#endif 
