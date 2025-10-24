/* 
 * File:   ens160.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 22.10.2025, 21:47
 */

#ifndef ENS_H
#define ENS_H

#include <stdlib.h>
#include <stdbool.h>

#include "pins.h"
#include "spi.h"

/* Registers */
#define ENS_PART_ID             0x00 // 2-byte
#define ENS_OPMODE              0x10 // 1-byte
#define ENS_CONFIG              0x11 // 1-byte
#define ENS_COMMAND             0x12 // 1-byte
#define ENS_TEMP_IN             0x13 // 2-byte
#define ENS_RH_IN               0x15 // 2-byte
#define ENS_DEVICE_STATUS       0x20 // 1-byte
#define ENS_DATA_AQI            0x21 // 1-byte
#define ENS_DATA_TVOC           0x22 // 2-byte
#define ENS_DATA_ECO2           0x24 // 2-byte
#define ENS_DATA_T              0x30 // 2-byte
#define ENS_DATA_RH             0x32 // 2-byte
#define ENS_DATA_MISR           0x38 // 1-byte
#define ENS_GPR_WRITE_0         0x40
#define ENS_GPR_WRITE_1         0x41
#define ENS_GPR_WRITE_2         0x42
#define ENS_GPR_WRITE_3         0x43
#define ENS_GPR_WRITE_4         0x44
#define ENS_GPR_WRITE_5         0x45
#define ENS_GPR_WRITE_6         0x46
#define ENS_GPR_WRITE_7         0x47
#define ENS_GPR_READ_0          0x48
#define ENS_GPR_READ_1          0x49
#define ENS_GPR_READ_2          0x4a
#define ENS_GPR_READ_3          0x4b
#define ENS_GPR_READ_4          0x4c
#define ENS_GPR_READ_5          0x4d
#define ENS_GPR_READ_6          0x4e
#define ENS_GPR_READ_7          0x4f

/* Operating modes */
#define ENS_MODE_DEEP_SLEEP     0x00 // (low-power standby)
#define ENS_MODE_IDLE           0x01 // (low power)
#define ENS_MODE_STANDARD       0x02 // Gas Sensing Mode
#define ENS_MODE_RESET          0xf0

/* Commands */
#define ENS_COMMAND_NOP         0x00
#define ENS_COMMAND_GET_APPVER  0x0e // Get FW Version
#define ENS_COMMAND_CLRGPR      0xcc // Clears GPR Read Registers

/* Status masks */
#define ENS_STATUS_STATAS_bm    0x80
#define ENS_STATUS_STATER_bm    0x40
#define ENS_STATUS_VALIDITY_gm  0x0c
#define ENS_STATUS_NEWDAT_bm    0x02
#define ENS_STATUS_NEWGPR_bm    0x01

/* Data read from sensor */
typedef struct {
    uint8_t aqi;
    uint16_t tvoc;
    uint16_t eco2;
    uint8_t status;
} EnsData;

/**
 * Notified when new output data is available.
 */
void ensIrq(void);

/**
 * Initializes the ENS160 sensor.
 * 
 * @param spics port and pin for SPI chip select
 * @return success
 */
bool ensInit(SpiCs *spics);

/**
 * Performs a measurement.
 * 
 * @param data measurement data read from sensor
 * @return success
 */
bool ensMeasure(EnsData *data);

#endif /* ENS_H */
