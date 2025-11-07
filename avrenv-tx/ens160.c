/*
 * File:   ens160.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 22.10.2025, 21:47
 */

#include "ens160.h"

void ensIrq(void) {
    // new data is available
}

bool ensInit(uint8_t addr) {
    uint8_t data[2];

    _delay_ms(10);

    // read and check part id
    i2cRegRead(addr, ENS_PART_ID, data, 2);
    uint16_t partId = data[1] << 8;
    partId |= data[0];
    if (partId != 0x0160) {
        return false;
    }

    // enable interrupt when new output data is available in the DATA i2cRegisters
    i2cRegWrite(addr, ENS_CONFIG, 0x23);

    // go to standard mode
    i2cRegWrite(addr, ENS_OPMODE, ENS_MODE_STANDARD);

    // check for error
    i2cRegRead(addr, ENS_DEVICE_STATUS, data, 1);
    if (data[0] & ENS_STATUS_STATER_bm) {
        return false;
    }

    return true;
}

bool ensMeasure(uint8_t addr, EnsData *ensdata) {
    uint8_t data[2];

    i2cRegRead(addr, ENS_DEVICE_STATUS, data, 1);
    ensdata->status = data[0];

    // check for error and validity
    if (ensdata->status & ENS_STATUS_STATER_bm ||
        (ensdata->status & ENS_STATUS_VALIDITY_gm) == ENS_VALIDITY_INVALID) {
        return false;
    }

    i2cRegRead(addr, ENS_DATA_AQI, data, 1);
    ensdata->aqi = data[0];

    i2cRegRead(addr, ENS_DATA_TVOC, data, 2);
    ensdata->tvoc = data[1] << 8;
    ensdata->tvoc |= data[0];

    i2cRegRead(addr, ENS_DATA_ECO2, data, 2);
    ensdata->eco2 = data[1] << 8;
    ensdata->eco2 |= data[0];

    return true;
}
