/*
 * File:   ens160.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 22.10.2025, 21:47
 */

#include "ens160.h"

/* SPI CS port and pin */
static SpiCs _spics;

/**
 * Writes given register and data.
 *
 * @param reg register to write to
 * @param data to write to register
 */
static void ensWrite(uint8_t reg,
                     const uint8_t data) {
    *_spics.port &= ~(1u << _spics.pin);
    transmit(reg | 0x01);
    transmit(data);
    *_spics.port |= (1u << _spics.pin);
}

/**
 * Reads from register(s) into given data array starting with
 * given register auto-incrementing.
 *
 * @param reg start register
 * @param data array with data to be read from consecutive registers
 * @param len number of registers to read from
 */
static void ensRead(uint8_t reg,
                    uint8_t *data,
                    uint32_t len) {
    *_spics.port &= ~(1u << _spics.pin);
    transmit(reg & ~0x01);
    for (uint32_t i = 0; i < len; i++) {
        data[i] = transmit(0x00);
    }
    *_spics.port |= (1u << _spics.pin);
}

bool ensInit(SpiCs *spics) {
    _spics = *spics;
    // TODO
    return true;
}

bool ensMeasure(EnsData *data) {
    // TODO
    return true;
}
