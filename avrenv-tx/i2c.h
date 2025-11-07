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

#define wait_idle() do {} while ((TWI0_MSTATUS & TWI_BUSSTATE_gm) != TWI_BUSSTATE_IDLE_gc)

/**
 * Begins a transaction by sending the address with write bit,
 * waiting for write interrupt flag.
 */
void i2cStartWrite(uint8_t addr);

/**
 * Begins a transaction by sending the address with read bit,
 * waiting for received acknowledgement.
 */
void i2cStartRead(uint8_t addr);

/**
 * Ends a message.
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

#endif /* I2C_H */

