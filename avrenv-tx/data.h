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
#include "i2c.h"
#include "bme688.h"
#include "ens160.h"
#include "pa1616s.h"
#include "librfm69/librfm69.h"
#include "libsdc/libsdc.h"

/* Turns the LED on */
#define led_on()        PORTD_OUT |= (1 << LED_PD7)

/* Turns the LED off */
#define led_off()       PORTD_OUT &= ~(1 << LED_PD7)

/**
 * Gets measurements from ENS160 and prints them. Currently only temporarily
 * used to compare gas measurements with BME688.
 */
void doEns(void);

/**
 * Gets measurements from all sensors, transmits and writes them to the
 * SD card
 *
 * @param sdc presence of the SD card
 */
void doMeas(bool sdc);

#endif /* DATA_H */

