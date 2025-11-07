/*
 * File:   ens160.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 22.10.2025, 21:47
 */

#include <util/delay.h>

#include "ens160.h"
#include "usart.h"

static void regWrite(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len) {
    i2cStartWrite(addr);
    i2cWrite(reg);
    for (uint8_t i = 0; i < len; i++) {
        i2cWrite(data[i]);
    }
}

static void regRead(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len) {
    i2cStartWrite(addr);
    i2cWrite(reg);
    i2cStartRead(addr);
    for (uint8_t i = 0; i < len - 1; i++) {
        data[i] = i2cReadAck();
    }
    data[len - 1] = i2cReadNack();
    _delay_ms(1);
}

void ensIrq(void) {
    // new data is available
}

bool ensInit(uint8_t addr) {
    uint8_t data[8];

    _delay_ms(10);

    // read and check part id
    regRead(addr, ENS_PART_ID, data, 2);
    uint16_t partId = data[0];
    partId |= data[1] << 8;
    if (partId != 0x0160) {
        i2cStop();

        return false;
    }

    // enable interrupt when new output data is available in the DATA registers
    // data[0] = 0x23;
    // regWrite(addr, ENS_CONFIG, data, 1);

    // go to standard mode
    data[0] = ENS_MODE_STANDARD;
    regWrite(addr, ENS_OPMODE, data, 1);

    // check for error
    regRead(addr, ENS_DEVICE_STATUS, data, 1);
    printByte(data[0]);
    if (data[0] & ENS_STATUS_STATER_bm) {
        i2cStop();

        return false;
    }

    i2cStop();

    return true;
}

bool ensMeasure(uint8_t addr, EnsData *ensdata) {
    uint8_t data[8];

    printString("ens\r\n");

    regRead(addr, ENS_DEVICE_STATUS, data, 1);
    ensdata->status = data[0];

    printByte(ensdata->status);

    // expecting STATAS, NEWDAT and NEWGPR to be set, Initial Start-Up phase
    // is also okay
    if (ensdata->status != 0x83 && ensdata->status != 0x8b) {
        return false;
    }

    regRead(addr, ENS_DATA_AQI, data, 1);
    ensdata->aqi = data[0];

    regRead(addr, ENS_DATA_TVOC, data, 2);
    ensdata->tvoc = data[1] << 8;
    ensdata->tvoc |= data[0];

    regRead(addr, ENS_DATA_ECO2, data, 2);
    ensdata->eco2 = data[1] << 8;
    ensdata->eco2 |= data[0];

    i2cStop();

    return true;
}
