/* 
 * File:   pa1616s.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 28.10.2025, 20:47
 */

#ifndef PA1616S_H
#define PA1616S_H

#include "usart.h"

#define NMEA_LEN    255
#define NMEA_CNT    2

#define PAS_ACK         "$PMTK001,314,3*36"
/* Output only GGA and RMC */
#define PAS_SET_OUTPUT  "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"

typedef struct {
    bool fix;
    uint32_t lat;
    uint32_t lon;
    uint16_t alt;
    uint16_t speed;
} NmeaData;

bool pasInit(void);

bool pasRead(NmeaData *data);

#endif /* PA1616S_H */
