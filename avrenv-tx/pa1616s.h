/* 
 * File:   pa1616s.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 28.10.2025, 20:47
 */

#ifndef PA1616S_H
#define PA1616S_H

#include <stdlib.h>

#include "usart.h"
#include "utils.h"

#define PAS_NMEA_LEN    255
#define PAS_NMEA_CNT    2
#define PAS_NMEA_FS     ","
#define PAS_ID_LEN      5
#define PAS_GPGGA       "GPGGA"
#define PAS_GPGGA_LEN   15
#define PAS_GPRMC       "GPRMC"
#define PAS_GPRMC_LEN   13

/* Command acknowledgement */
#define PAS_ACK         "PMTK001,314,3*36"
/* Output nothing */
#define PAS_OUT_NONE    "$PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"
/* Output only GGA and RMC */
#define PAS_OUT_GGA_RMC "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"

/* Enables USART receiver */
#define enable_rx()     USART0_CTRLB |= (1 << USART_RXEN_bp)
/* Disables USART receiver */
#define disable_rx()    USART0_CTRLB &= ~(1 << USART_RXEN_bp)

/* Data read off the GPS module */
typedef struct {
    /* Timestamp in UTC: hhmmss */
    uint32_t utc;
    /* Latitude in degrees minutes (WGS84) x 10000 */
    uint32_t lat;
    /* Longitude in degrees minutes (WGS84) x 10000 */
    uint32_t lon;
    /* Fix: 0 = not available, 1 = GPS, 2 = differential GPS */
    uint8_t fix;
    /* Number of satellites used */
    uint8_t sat;
    /* Altitude in meters x 10 */
    uint16_t alt;
    /* Speed over ground in knots * 100 */
    uint16_t speed;
} NmeaData;

/**
 * Sets up USART, configures the GPS module to only output 
 * GGA and RMC messages and checks the ack.
 * 
 * @return true on success, false otherwise
 */
bool pasInit(void);

/**
 * Reads a fixed number of messages from the module and
 * sets the fields in the given struct.
 * 
 * @param data
 * @return true on success, false otherwise
 */
bool pasRead(NmeaData *data);

#endif /* PA1616S_H */
