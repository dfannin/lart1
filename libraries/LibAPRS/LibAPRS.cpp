#include "Arduino.h"
#include "AFSK.h"
#include "AX25.h"

Afsk modem;
AX25Ctx AX25;
extern void aprs_msg_callback(struct AX25Msg *msg);
#define countof(a) sizeof(a)/sizeof(a[0])

int LibAPRS_vref = REF_3V3;
bool LibAPRS_open_squelch = false;

unsigned long custom_preamble = 350UL;
unsigned long custom_tail = 50UL;

AX25Call src;
AX25Call dst;
AX25Call path1;
AX25Call path2;

char CALL[7] = "NOCALL";
int CALL_SSID = 0;
char DST[7] = "APZMDM";
int DST_SSID = 0;
char PATH1[7] = "WIDE1";
int PATH1_SSID = 1;
char PATH2[7] = "WIDE2";
int PATH2_SSID = 2;

AX25Call path[4];

// Location packet assembly fields
char latitude[9];
char longtitude[10];
char symbolTable = '/';
char symbol = 'n';

uint8_t power = 10;
uint8_t height = 10;
uint8_t gain = 10;
uint8_t directivity = 10;
/////////////////////////

// Message packet assembly fields
char message_recip[7];
int message_recip_ssid = -1;

int message_seq = 0;
char lastMessage[67];
size_t lastMessageLen;
bool message_autoAck = false;
/////////////////////////

void APRS_init(int reference, bool open_squelch) {
    LibAPRS_vref = reference;
    LibAPRS_open_squelch = open_squelch;

    AFSK_init(&modem);
    ax25_init(&AX25, aprs_msg_callback);
}

void APRS_poll(void) {
    ax25_poll(&AX25);
}

void APRS_setCallsign(const char *call, int ssid) {
    memset(CALL, 0, 7);
    int i = 0;
    while (i < 6 && call[i] != 0) {
        CALL[i] = call[i];
        i++;
    }
    CALL_SSID = ssid;
}

void APRS_setDestination(const char *call, int ssid) {
    memset(DST, 0, 7);
    int i = 0;
    while (i < 6 && call[i] != 0) {
        DST[i] = call[i];
        i++;
    }
    DST_SSID = ssid;
}

void APRS_setPath1(const char *call, int ssid) {
    memset(PATH1, 0, 7);
    int i = 0;
    while (i < 6 && call[i] != 0) {
        PATH1[i] = call[i];
        i++;
    }
    PATH1_SSID = ssid;
}

void APRS_setPath2(const char *call, int ssid) {
    memset(PATH2, 0, 7);
    int i = 0;
    while (i < 6 && call[i] != 0) {
        PATH2[i] = call[i];
        i++;
    }
    PATH2_SSID = ssid;
}

void APRS_setMessageDestination(const char *call, int ssid) {
    memset(message_recip, 0, 7);
    int i = 0;
    while (i < 6 && call[i] != 0) {
        message_recip[i] = call[i];
        i++;
    }
    message_recip_ssid = ssid;
}

void APRS_setPreamble(unsigned long pre) {
    custom_preamble = pre;
}

void APRS_setTail(unsigned long tail) {
    custom_tail = tail;
}

void APRS_useAlternateSymbolTable(bool use) {
    if (use) {
        symbolTable = '\\';
    } else {
        symbolTable = '/';
    }
}

void APRS_setSymbol(char sym) {
    symbol = sym;
}

/*
 * lat value is %7.2f + D{N,S} e.g.  '1234.56N'  
 * leading padding are spaces e.g ' 123.45N'
 * so leading zeros need to be added e.g. '0123.45N'
 *
 */
void APRS_setLat(const char *lat) {
    memset(latitude, 0, 9);
    int i = 0;
    while (i < 8 && lat[i] != 0) {
        if ( i < 4 && lat[i] == ' ' ) {
            latitude[i] = '0' ;
        } else {
            latitude[i] = lat[i];
        }
        i++;
    }
}

/*
 * lon value is %8.2f + D{E,W} e.g.  '11234.56E'  
 * leading padding are spaces e.g '  123.45E'
 * so leading zeros need to be added e.g. '00123.45E'
 *
 */
void APRS_setLon(const char *lon) {
    memset(longtitude, 0, 10);
    int i = 0;
    while (i < 9 && lon[i] != 0) {
        if ( i < 5 && lon[i] == ' ' ) {
            longtitude[i] = '0' ;
        } else {
            longtitude[i] = lon[i];
        }
        i++;
    }
}

void APRS_setPower(int s) {
    if (s >= 0 && s < 10) {
        power = s;
    }
}

void APRS_setHeight(int s) {
    if (s >= 0 && s < 10) {
        height = s;
    }
}

void APRS_setGain(int s) {
    if (s >= 0 && s < 10) {
        gain = s;
    }
}

void APRS_setDirectivity(int s) {
    if (s >= 0 && s < 10) {
        directivity = s;
    }
}

void APRS_printSettings(Stream * db) {

    if ( db != NULL ) { 
        db->print(F("Settings:"));
        db->print(F("CS:")); db->print(CALL); db->print(F("-")); db->print(CALL_SSID);
        db->print(F(",Dst:")); db->print(DST); db->print(F("-")); db->print(DST_SSID);
        db->print(F(",P1:")); db->print(PATH1); db->print(F("-")); db->print(PATH1_SSID);
        db->print(F(",P2:")); db->print(PATH2); db->print(F("-")); db->print(PATH2_SSID);
        db->print(F(",Mdst:")); if (message_recip[0] == 0) { 
                                               db->print(F("N/A")); 
                                         } else { db->print(message_recip); 
                                                db->print(F("-")); 
                                                db->print(message_recip_ssid);
                                         }
        db->print(F(",Pre:")); db->print(custom_preamble);
        db->print(F(",Tail:")); db->print(custom_tail);
        db->print(F(",Symtab:")); if (symbolTable == '/') { db->print(F("Basic")); } else { db->print(F("Alt")); }
        db->print(F(",Sym:")); db->print(symbol);
        /*
        db->print(F(",P:")); if (power < 10) { db->print(power); } else { db->print(F("N/A")); }
        db->print(F(",H:")); if (height < 10) { db->print(height); } else { db->print(F("N/A")); }
        db->print(F(",G:")); if (gain < 10) { db->print(gain); } else { db->print(F("N/A")); }
        db->print(F(",D:")); if (directivity < 10) { db->print(directivity); } else { db->print(F("N/A")); }
        db->print(F(",Lat:")); if (latitude[0] != 0) { db->print(latitude); } else { db->print(F("N/A")); }
        db->print(F(",Lon:")); if (longtitude[0] != 0) { db->print(longtitude); } else { db->print(F("N/A")); }
        */
        db->println(F(""));
    }
}

void APRS_sendPkt(void *_buffer, size_t length) {

    uint8_t *buffer = (uint8_t *)_buffer;

    memcpy(dst.call, DST, 6);
    dst.ssid = DST_SSID;

    memcpy(src.call, CALL, 6);
    src.ssid = CALL_SSID;

    memcpy(path1.call, PATH1, 6);
    path1.ssid = PATH1_SSID;

    memcpy(path2.call, PATH2, 6);
    path2.ssid = PATH2_SSID;

    path[0] = dst;
    path[1] = src;
    path[2] = path1;
    path[3] = path2;

    ax25_sendVia(&AX25, path, countof(path), buffer, length);
}

// Dynamic RAM usage of this function is 30 bytes
void APRS_sendLoc(void *_buffer, size_t length, Stream *db) {
    size_t payloadLength = 20+length;
    bool usePHG = false;
    if (power < 10 && height < 10 && gain < 10 && directivity < 9) {
        usePHG = true;
        payloadLength += 7;
    }
    uint8_t *packet = (uint8_t*)malloc(payloadLength);
    uint8_t *ptr = packet;
    packet[0] = '=';
    packet[9] = symbolTable;
    packet[19] = symbol;
    ptr++;
    memcpy(ptr, latitude, 8);
    ptr += 9;
    memcpy(ptr, longtitude, 9);
    ptr += 10;
    if (usePHG) {
        packet[20] = 'P';
        packet[21] = 'H';
        packet[22] = 'G';
        packet[23] = power+48;
        packet[24] = height+48;
        packet[25] = gain+48;
        packet[26] = directivity+48;
        ptr+=7;
    }
    if (length > 0) {
        uint8_t *buffer = (uint8_t *)_buffer;
        memcpy(ptr, buffer, length);
    }

    if ( db != NULL ) { 
        db->print("Loc Pkt Sent:") ; 
        char pbuf[60] ; 
        size_t i ; 
        for (i = 0 ; i < payloadLength ; i++ ) {
            pbuf[i]  = (char) packet[i] ; 
        }
        pbuf[i] = '\0' ; 

        db->println(pbuf) ;
    }

    APRS_sendPkt(packet, payloadLength);
    free(packet);
}

// Dynamic RAM usage of this function is 18 bytes
void APRS_sendMsg(void *_buffer, size_t length, Stream *db) {
    if (length > 67) length = 67;
    size_t payloadLength = 11+length+4;

    uint8_t *packet = (uint8_t*)malloc(payloadLength);
    uint8_t *ptr = packet;
    packet[0] = ':';
    int callSize = 6;
    int count = 0;
    while (callSize--) {
        if (message_recip[count] != 0) {
            packet[1+count] = message_recip[count];
            count++;
        }
    }
    if (message_recip_ssid != -1) {
        packet[1+count] = '-'; count++;
        if (message_recip_ssid < 10) {
            packet[1+count] = message_recip_ssid+48; count++;
        } else {
            packet[1+count] = 49; count++;
            packet[1+count] = message_recip_ssid-10+48; count++;
        }
    }
    while (count < 9) {
        packet[1+count] = ' '; count++;
    }
    packet[1+count] = ':';
    ptr += 11;
    if (length > 0) {
        uint8_t *buffer = (uint8_t *)_buffer;
        memcpy(ptr, buffer, length);
        memcpy(lastMessage, buffer, length);
        lastMessageLen = length;
    }

    message_seq++;
    if (message_seq > 999) message_seq = 0;

    packet[11+length] = '{';
    int n = message_seq % 10;
    int d = ((message_seq % 100) - n)/10;
    int h = (message_seq - d - n) / 100;

    packet[12+length] = h+48;
    packet[13+length] = d+48;
    packet[14+length] = n+48;

    if ( db != NULL ) { 
        db->print("Msg Pkt Sent:") ; 
        char pbuf[60] ; 
        size_t i ; 
        for (i = 0 ; i < payloadLength ; i++ ) {
            pbuf[i]  = (char) packet[i] ; 
        }
        pbuf[i] = '\0' ; 

        db->println(pbuf) ;
    }
    
    APRS_sendPkt(packet, payloadLength);
    free(packet);
}

void APRS_msgRetry() {
    message_seq--;
    APRS_sendMsg(lastMessage, lastMessageLen,NULL);
}

// For getting free memory, from:
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1213583720/15

extern unsigned int __heap_start;
extern void *__brkval;

struct __freelist {
  size_t sz;
  struct __freelist *nx;
};

extern struct __freelist *__flp;

int freeListSize() {
  struct __freelist* current;
  int total = 0;
  for (current = __flp; current; current = current->nx) {
    total += 2; /* Add two bytes for the memory block's header  */
    total += (int) current->sz;
  }
  return total;
}

int freeMemory() {
  int free_memory;
  if ((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__heap_start);
  } else {
    free_memory = ((int)&free_memory) - ((int)__brkval);
    free_memory += freeListSize();
  }
  return free_memory;
}
