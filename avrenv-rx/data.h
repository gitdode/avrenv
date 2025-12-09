/* 
 * File:   data.h
 * Author: torstem.roemer@luniks.net
 *
 * Created on 15.11.2025, 01:27
 */

#ifndef DATA_H
#define DATA_H

#include <stdbool.h>
#include <limits.h>
#include <math.h>

#include "utils.h"
#include "usart.h"
#include "spi.h"
#include "librfm95/librfm95.h"
#include "libtft/libtft.h"
#include "libtft/unifont.h"

/* Expected length of payload from transmitter */
#define PAYLOAD_LEN 24

#define BLACK           0x0000
#define RED             0xf800
#define WHITE           0xffff

/* Periodic interrupt timer interrupt count (seconds) */
extern volatile uint32_t pitints;

/* Represents the data received from the transmitter */
typedef struct {
    /* Tx power in dBm */
    uint8_t power;
    /* Battery voltage in mV */
    uint16_t voltage;
    /* Temperature in degrees Celsius * 100 */
    int16_t temperature;
    /* Relative humidity in % */
    uint8_t humidity;
    /* Barometric pressure in hPa */
    uint16_t pressure;
    /* Gas sensor resistance in kOhm */
    uint16_t gasres;
    /* Latitude in degrees minutes (WGS84) x 10000 */
    uint32_t lat;
    /* Longitude in degrees minutes (WGS84) x 10000 */
    uint32_t lon;
    /* Fix: 0 = not available, 1 = GPS, 2 = differential GPS */
    uint8_t fix;
    /* Number of satellites used */
    uint8_t sat;
    /* Altitude in meters */
    int16_t alt;
    /* Speed over ground in knots * 100 */
    uint16_t speed;
} EnvData;

/**
 * Receives and handles the data from the receiver and goes
 * back to receive mode when done.
 */
void receiveData(void);

#endif /* DATA_H */

