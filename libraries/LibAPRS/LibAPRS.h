/*
 * Modified by dfannin 2016
 */
#include "Arduino.h"
#include <stdint.h>
#include <stdbool.h>

#include "FIFO.h"
#include "CRC-CCIT.h"
#include "HDLC.h"
#include "AFSK.h"
#include "AX25.h"

void APRS_init(int reference, bool open_squelch);
void APRS_poll(void);

void APRS_setCallsign(const char *call, int ssid);
void APRS_setDestination(const char *call, int ssid);
void APRS_setMessageDestination(const char *call, int ssid);
void APRS_setPath1(const char *call, int ssid);
void APRS_setPath2(const char *call, int ssid);

void APRS_setPreamble(unsigned long pre);
void APRS_setTail(unsigned long tail);
void APRS_useAlternateSymbolTable(bool use);
void APRS_setSymbol(char sym);

void APRS_setLat(const char *lat);
void APRS_setLon(const char *lon);
void APRS_setPower(int s);
void APRS_setHeight(int s);
void APRS_setGain(int s);
void APRS_setDirectivity(int s);

void APRS_sendPkt(void *_buffer, size_t length);
void APRS_sendLoc(void *_buffer, size_t length, Stream *db);
void APRS_sendMsg(void *_buffer, size_t length, Stream *db);
void APRS_msgRetry();

void APRS_printSettings(Stream *db);

int freeMemory();
