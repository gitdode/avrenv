/* 
 * File:   data.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 21.11.2025, 20:24
 */

#ifndef DATA_H
#define DATA_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <error.h>

#include <json-c/json.h>

#define FIELD_LEN   16

/* Represents the data received from the transmitter */
typedef struct {
    /* Seconds since power-on */
    uint32_t time;
    /* Duration between transmissions in seconds */
    uint8_t dur;
    /* Receiver RSSI in dBm */
    uint8_t rssi;
    /* CRC: 0 = NOK, 1 = OK */
    uint8_t crc;
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
 * Returns the given structured data as Json object.
 * 
 * The returned json_object must be freed with json_object_put() after use.
 * 
 * @param data structured data
 * @return Json object
 */
json_object* to_json(EnvData *data);

/**
 * Comma-separates the given '\0' terminated line into fields 
 * and assigns the converted values to the given structured data.
 * 
 * @param data structure
 * @param line '\0' terminated string optionally ending with a '\n'
 * @return field length
 */
int read_data(EnvData *data, const char *line);

#endif /* DATA_H */

