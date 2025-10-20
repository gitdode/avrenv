/* 
 * File:   spi.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 15.07.25, 17:09
 */

#ifndef SPI_H
#define SPI_H

#include <avr/io.h>

/**
 * Sets slow SPI speed.
 */
void spiSlow(void);

/**
 * Sets medium SPI speed.
 */
void spiMid(void);

/**
 * Sets fast SPI speed.
 */
void spiFast(void);

/**
 * Transmits the given byte and returns the byte reveived at the same time.
 * @param data byte to be written
 * @return byte read while writing
 */
uint8_t transmit(uint8_t data);

#endif /* SPI_H */

