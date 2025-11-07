/* 
 * File:   i2c.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 06.11.2025, 17:31
 */

#ifndef I2C_H
#define I2C_H

#include <avr/io.h>

#include "pins.h"

/**
 * Begins a write transaction by sending the address (bit 7-1) 
 * with write bit, waiting for write interrupt flag.
 */
void i2cStartWrite(uint8_t addr);

/**
 * Begins a read transaction by sending the address (bit 7-1) 
 * with read bit, waiting for received acknowledgement.
 */
void i2cStartRead(uint8_t addr);

/**
 * Ends the transaction.
 */
void i2cStop(void);

/**
 * Transmits the given byte of data.
 * 
 * @param data
 */
void i2cWrite(uint8_t data);

/**
 * Reads one byte with ack.
 * 
 * @return data
 */
uint8_t i2cReadAck(void);

/**
 * Reads one byte without ack.
 * 
 * @return 
 */
uint8_t i2cReadNack(void);

/**
 * Writes given byte to the given register to the client
 * with the given address (bit 7-1).
 * 
 * @param addr client address, bit 7-1
 * @param reg register to write data to
 * @param data data to write
 */
void i2cRegWrite(uint8_t addr, uint8_t reg, uint8_t data);

/**
 * Reads data (starting from) the given register from the client
 * with the given address (bit 7-1).
 * 
 * @param addr client address, bit 7-1
 * @param reg register to read data from
 * @param data read data
 * @param len length of data
 */
void i2cRegRead(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len);

#endif /* I2C_H */

