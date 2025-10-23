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
static void spiWrite(const uint8_t reg,
                     const uint8_t data) {
    *_spics.port &= ~(1u << _spics.pin);
    transmit((reg << 1) & ~0x01);
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
static void spiRead(const uint8_t reg,
                    uint8_t *data,
                    const uint8_t len) {
    *_spics.port &= ~(1u << _spics.pin);
    transmit((reg << 1) | 0x01);
    for (uint8_t i = 0; i < len; i++) {
        data[i] = transmit(0x00);
    }
    *_spics.port |= (1u << _spics.pin);
}

void ensIrq(void) {
    // new data is available
}

bool ensInit(SpiCs *spics) {
    _spics = *spics;
    uint8_t data[8];

    // read and check part id
    spiRead(ENS_PART_ID, data, 2);
    uint16_t partId = data[1] << 8;
    partId |= data[0];
    if (partId != 0x0160) {
        return false;
    }

    // enable interrupt when new output data is available in the DATA registers
    // spiWrite(ENS_CONFIG, 0x23);

    // go to standard mode
    spiWrite(ENS_OPMODE, ENS_MODE_STANDARD);

    // check for error
    spiRead(ENS_DEVICE_STATUS, data, 1);
    if (data[0] & ENS_STATUS_STATER_bm) {
        return false;
    }

    return true;
}

bool ensMeasure(EnsData *ensdata) {
    uint8_t data[8];

    spiRead(ENS_DEVICE_STATUS, data, 1);
    ensdata->status = data[0];

    // expecting STATAS, NEWDAT and NEWGPR to be set, Initial Start-Up phase
    // is also okay
    if (ensdata->status != 0x83 && ensdata->status != 0x8b) {
        return false;
    }

    spiRead(ENS_DATA_AQI, data, 1);
    ensdata->aqi = data[0];

    spiRead(ENS_DATA_TVOC, data, 2);
    ensdata->tvoc = data[1] << 8;
    ensdata->tvoc |= data[0];

    spiRead(ENS_DATA_ECO2, data, 2);
    ensdata->eco2 = data[1] << 8;
    ensdata->eco2 |= data[0];

    return true;
}
