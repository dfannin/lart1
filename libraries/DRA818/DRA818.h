/*
    DRA818.h - DRA818U/V Comms Library.

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
 * Modified by Dfannin 2016
 */

#ifndef DRA818_h
#define DRA818_h

#define DRA818_BUFSIZE 60

#define PTT_ON  true
#define PTT_OFF  false
#include "Arduino.h"
#include <stdio.h>
#include <Stream.h>

class DRA818 {
    public:
        // Constructors
        DRA818(Stream *serial, uint8_t PTT, bool ptt_logic=LOW);

        void setFreq(float tx_freq, float rx_freq=0);
        void setTXCTCSS(uint8_t ctcss);
        void setRXCTCSS(uint8_t ctcss);
        void setSquelch(uint8_t sql);
        bool setVolume(uint8_t vol);
        bool setFilters(boolean preemph, boolean highpass, boolean lowpass);
        bool writeFreq(void);
        void setBW(uint8_t bw);
        void setPTT(bool ptt);
        void clearinput(void) ;
        void readResponse() ;
        bool heartbeat();

        uint8_t PTT_PIN;
        bool ptt_on;
        bool ptt_off;
        uint8_t tx_ctcss;
        uint8_t rx_ctcss;
        float tx_freq;
        float rx_freq;
        uint8_t volume;
        uint8_t squelch;
        uint8_t preemph;
        uint8_t highpass;
        uint8_t lowpass;
        uint8_t bw;
        char response[DRA818_BUFSIZE];
        char buffer[DRA818_BUFSIZE];


    private:
        Stream *serial;

};
#endif
