/*
    DRA818.cpp - DRA818U/V Comms Library.

    Copyright (C) 2014 Mark Jessop <vk5qi@rfhead.net>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    For a full copy of the GNU General Public License, 
    see <http://www.gnu.org/licenses/>.
*/

/*
modified by dfannin 2016
added rx frequency set function
added bandwidth function
added heartbeak function
added readResponse function

*/
#include "DRA818.h"


DRA818::DRA818(Stream *serial, uint8_t PTT){
    this->serial = serial;
    this->PTT_PIN = PTT;
    pinMode(this->PTT_PIN, OUTPUT);
    this->setPTT(LOW);
    this->tx_ctcss = 0;
    this->rx_ctcss = 0;
    this->tx_freq =  146.500;
    this->rx_freq = this->tx_freq;
    this->bw = 0 ; // 0 = 12.5khz, 1 = 25khz

    this->volume = 4;
    this->squelch = 0;
    this->preemph = 0;
    this->highpass = 0;
    this->lowpass = 0;
}


void DRA818::setFreq(float tx_freq, float rx_freq){
    if( (tx_freq>=136.0 && tx_freq<=174.00 ) || (tx_freq>=410.0 && tx_freq<=480.000) ){
        if ( rx_freq == 0 ) {
            this->tx_freq = tx_freq;
            this->rx_freq = tx_freq;
        } else {
            this->tx_freq = tx_freq;
            this->rx_freq = rx_freq;
        }
    }
}

// Refer to https://en.wikipedia.org/wiki/CTCSS for CTCSS values.
void DRA818::setTXCTCSS(uint8_t ctcss){
    if(ctcss>=0 && ctcss<=38){
        this->tx_ctcss = ctcss;
    }
}
void DRA818::setRXCTCSS(uint8_t ctcss){
    if(ctcss>= 0 && ctcss<=38){
        this->rx_ctcss = ctcss;
    }
}

void DRA818::setSquelch(uint8_t sql){
    if(sql >= 0 && sql<=8){
        this->squelch = sql;
    }
}

bool DRA818::writeFreq(void){
    digitalWrite(this->PTT_PIN, LOW);// Set PTT off, so we can communicate with the uC.
    delay(250); // Delay for a bit, to let the uC boot up (??)

    char freq_buffer[10];
    char rx_freq_buffer[10];

    dtostrf(this->tx_freq,8,4,freq_buffer);
    dtostrf(this->rx_freq,8,4,rx_freq_buffer);
    sprintf(this->buffer,"AT+DMOSETGROUP=%1d,%s,%s,%04d,%1d,%04d\r\n",this->bw,freq_buffer,rx_freq_buffer,this->tx_ctcss,this->squelch,this->rx_ctcss);

    this->clearinput() ;
    this->serial->write(this->buffer);
    this->serial->flush() ;
    this->readResponse() ;
    return ( strncmp(this->response,"+DMOSETGROUP:0\r\n",14) == 0 )? true : false ; 
}

bool DRA818::setVolume(uint8_t vol){
    if(vol>=1 && vol<=8){
        this->volume = vol;
    }

    digitalWrite(this->PTT_PIN, LOW); // Set PTT off, so we can communicate with the uC.
    delay(250); // Delay for a bit, to let the uC boot up (??)

    this->clearinput() ;
    sprintf(this->buffer,"AT+DMOSETVOLUME=%1d\r\n",this->volume);
    this->serial->write(this->buffer);
    this->serial->flush() ;
    this->readResponse() ;
    return ( strncmp(this->response,"+DMOSETVOLUME:0\r\n",15) == 0 )? true : false ; 
}

bool DRA818::setFilters(boolean preemph, boolean highpass, boolean lowpass){
    // Gratuitous use of the ternary operator.
    this->preemph=preemph?0:1;
    this->highpass=highpass?0:1;
    this->lowpass=lowpass?0:1;

    digitalWrite(this->PTT_PIN, LOW); // Set PTT off, so we can communicate with the uC.
    delay(250); // Delay for a bit, to let the uC boot up (??)

    this->clearinput() ;
    sprintf(this->buffer,"AT+SETFILTER=%1d,%1d,%1d\r\n",int(this->preemph),int(this->highpass),int(this->lowpass));
    this->serial->write(this->buffer);
    this->serial->flush() ;
    this->readResponse() ;
    return ( strncmp(this->response,"+DMOSETFILTER:0\r\n",15) == 0 )? true : false ; 

}

void DRA818::setBW(uint8_t bw) {
    if ( bw >= 0 && bw <=1 ) {
        this->bw = bw ;
    }
}

void DRA818::setPTT(bool ptt) {
    if ( ptt ) {
        digitalWrite(this->PTT_PIN, HIGH);
    } else {
        digitalWrite(this->PTT_PIN, LOW);
    }
}


void DRA818::clearinput(void) {
    while ( this->serial->available() > 0 ) {
        char c = this->serial->read() ;
    }
} 


void DRA818::readResponse() {

    this->response[0] = '\0' ;
    int i = 0 ;
    unsigned long  rtimeout =  millis()  ;
  
   // delay loop waiting for chars
    while ( ( millis() - rtimeout ) < 500) {
        if ( this-serial->available() > 0 )  break ;
    } 

    rtimeout = millis() ;

    while ( (millis() - rtimeout) < 100 )  {
         while  (   this->serial->available()  > 0  && i < (DRA818_BUFSIZE - 2)) {
            char c = this->serial->read() ;
            this->response[i++]=c ;
            this->response[i]='\0';
            rtimeout = millis() ;
         }
    }
}

bool DRA818::heartbeat() {
    digitalWrite(this->PTT_PIN, LOW); 
    delay(250); 

    this->clearinput() ;
    this->serial->write("AT+DMOCONNECT\r\n") ;
    this->serial->flush() ;
    this->readResponse() ;
    return ( strncmp(this->response,"+DMOCONNECT:0\r\n",13) == 0 )? true : false ; 
}
