#include "constants.h"

#ifndef DEVICE_CONFIGURATION
#define DEVICE_CONFIGURATION


#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__) 
#define TARGET_CPU m328p
#endif

#if defined(__AVR_ATmega2560__) 
#define TARGET_CPU mega2560
#endif

#ifndef F_CPU
    #define F_CPU 16000000
#endif

#ifndef FREQUENCY_CORRECTION
    #define FREQUENCY_CORRECTION 0
#endif

// Sampling & timer setup
#define CONFIG_AFSK_DAC_SAMPLERATE 9600

// Port settings
#if TARGET_CPU == m328p
    #define DAC_PORT PORTD
    #define DAC_DDR  DDRD
    #define LED_PORT PORTB
    #define LED_DDR  DDRB
    #define ADC_PORT PORTC
    #define ADC_DDR  DDRC
#endif

#if TARGET_CPU == mega2560
    #define DAC_PORT PORTA
    #define DAC_DDR  DDRA
    #define LED_PORT PORTL
    #define LED_DDR  DDRL
    #define ADC_PORT PORTF
    #define ADC_DDR  DDRF
#endif

#endif
