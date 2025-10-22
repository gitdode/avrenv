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
#include "bme68x/bme68x.h"

/**
 * Initializes the BME68x sensor.
 * 
 * @param temp heater temperature
 * @param dur heater duration in milliseconds
 * @param amb ambient temperature
 * @param intf port and pin for SPI chip select
 * @return error code
 */
int8_t bmeInit(uint16_t temp,
               uint16_t dur,
               uint8_t amb,
               SpiCs *intf);

/**
 * Sets heater temperature and duration in milliseconds.
 * 
 * @param temp heater temperature
 * @param dur duration in milliseconds
 * @return error code
 */
int8_t bmeSetHeaterConf(uint16_t temp, 
                        uint16_t dur);

/**
 * Performs a measurement.
 * 
 * @param data measurement data read from sensor
 * @return error code
 */
int8_t bmeMeasure(struct bme68x_data *data);

#undef __DELAY_BACKWARD_COMPATIBLE__

#endif /* BME688_H */
