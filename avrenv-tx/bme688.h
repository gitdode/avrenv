/* 
 * File:   bme688.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 4. Oktober 2025, 20:31
 */

#ifndef BME688_H
#define BME688_H

/* Allow passing a variable to _delay_* functions */
#define __DELAY_BACKWARD_COMPATIBLE__

#include <stdlib.h>
#include <util/delay.h>

#include "pins.h"
#include "spi.h"
#include "usart.h"
#include "bme68x/bme68x.h"

/* Port and pin for SPI chip select */
typedef struct {
    volatile uint8_t *port;
    uint8_t pin;
} Intf;

/**
 * Initializes the BME68x sensor.
 * 
 * @param temp heater temperature
 * @param dur heater duration in milliseconds
 * @param amb ambient temperature
 * @param intf port and pin for SPI chip select
 * @return error code
 */
int8_t initBME68x(uint16_t temp,
                  uint16_t dur,
                  uint8_t amb,
                  Intf *intf);

/**
 * Sets heater temperature and duration in milliseconds.
 * 
 * @param temp heater temperature
 * @param dur duration in milliseconds
 * @return error code
 */
int8_t bme68xSetHeaterConf(uint16_t temp, 
                           uint16_t dur);

/**
 * Performs a measurement.
 * 
 * @param data measurement data read from sensor
 * @return error code
 */
int8_t bme68xMeasure(struct bme68x_data *data);

#undef __DELAY_BACKWARD_COMPATIBLE__

#endif /* BME688_H */
