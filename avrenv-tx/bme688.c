/*
 * File:   bme688.c
 * Author: torsten.roemer@luniks.net
 *
 * Thanks to https://github.com/boschsensortec/BME68x_SensorAPI/tree/master/examples
 *
 * Created on 4. Oktober 2025, 20:31
 */

#include "bme688.h"

static struct bme68x_dev dev;
static struct bme68x_conf conf;
static struct bme68x_heatr_conf heater_conf;

/**
 * Writes given register - data pairs to respective register(s).
 *
 * @param reg register of first register - data pair
 * @param data array with data of first register - data pair as first element,
 *   with remaining register - data pairs following
 * @param len (number of register - data pairs) - 1
 * @param intfPtr port and pin for SPI chip select
 * @return success
 */
static BME68X_INTF_RET_TYPE bmeWrite(uint8_t reg,
                                     const uint8_t *data,
                                     uint32_t len,
                                     void *intfPtr) {
    const SpiCs intf = *((SpiCs *)intfPtr);
    *intf.port &= ~(1u << intf.pin);
    transmit(reg);
    for (uint32_t i = 0; i < len; i++) {
        transmit(data[i]);
    }
    *intf.port |= (1u << intf.pin);

    return BME68X_INTF_RET_SUCCESS;
}

/**
 * Reads from register(s) into given data array starting with
 * given register auto-incrementing.
 *
 * @param reg start register
 * @param data array with data to be read from consecutive registers
 * @param len number of registers to read from
 * @param intfPtr port and pin for SPI chip select
 * @return success
 */
static BME68X_INTF_RET_TYPE bmeRead(uint8_t reg,
                                    uint8_t *data,
                                    uint32_t len,
                                    void *intfPtr) {
    const SpiCs intf = *((SpiCs *)intfPtr);
    *intf.port &= ~(1u << intf.pin);
    transmit(reg);
    for (uint32_t i = 0; i < len; i++) {
        data[i] = transmit(0x00);
    }
    *intf.port |= (1u << intf.pin);

    return BME68X_INTF_RET_SUCCESS;
}

/**
 * Delay function for BME68x - requires __DELAY_BACKWARD_COMPATIBLE__
 * so delay does not have to be a compile time constant.
 *
 * @param period in microseconds
 * @param intfPtr not used
 */
static void bmeDelayUs(uint32_t period,
                       void *intfPtr) {
    _delay_us(period);
}

int8_t bmeInit(uint16_t temp,
               uint16_t dur,
               uint8_t amb,
               SpiCs *intf) {
    _delay_ms(2);

    int8_t result;

    dev.intf = BME68X_SPI_INTF;
    dev.write = bmeWrite;
    dev.read = bmeRead;
    dev.delay_us = bmeDelayUs;
    dev.intf_ptr = intf;
    dev.amb_temp = amb;

    result = bme68x_init(&dev);
    if (result != BME68X_OK) {
        return result;
    }

    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE;
    conf.os_hum = BME68X_OS_8X;
    conf.os_pres = BME68X_OS_8X;
    conf.os_temp = BME68X_OS_8X;
    result = bme68x_set_conf(&conf, &dev);
    if (result != BME68X_OK) {
        return result;
    }

    heater_conf.enable = BME68X_ENABLE;
    heater_conf.heatr_temp = temp;
    heater_conf.heatr_dur = dur;
    result = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heater_conf, &dev);
    if (result != BME68X_OK) {
        return result;
    }

    return BME68X_OK;
}

int8_t bmeSetHeaterConf(uint16_t temp,
                        uint16_t dur) {
    int8_t result;
    heater_conf.enable = BME68X_ENABLE;
    heater_conf.heatr_temp = temp;
    heater_conf.heatr_dur = dur;
    result = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heater_conf, &dev);
    if (result != BME68X_OK) {
        return result;
    }

    return BME68X_OK;
}

int8_t bmeMeasure(struct bme68x_data *data) {

    int8_t result;
    uint8_t n_data = 0;
    uint8_t count = 0;
    do {
        result = bme68x_set_op_mode(BME68X_FORCED_MODE, &dev);
        if (result != BME68X_OK) {
            return result;
        }

        uint32_t meas_dur = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &dev) +
                heater_conf.heatr_dur * 1000UL;
        dev.delay_us(meas_dur, dev.intf_ptr);

        result = bme68x_get_data(BME68X_FORCED_MODE, data, &n_data, &dev);
        if (result < BME68X_OK) {
            return result;
        }

        count++;
    } while ((n_data != 1 || data->status != 0xb0) && count < 10);

    return BME68X_OK;
}
