/*
 * File:   data.c
 * Author: torstem.roemer@luniks.net
 *
 * Created on 15.11.2025, 01:27
 */

#include "data.h"

/**
 * Transmits given payload to the receiver.
 *
 * @param payload data to be transmitted
 * @param len payload length
 */
static void transmitMeas(uint8_t *payload, uint8_t len) {
    rfmWake();
#if LORA
    rfmLoRaTx(payload, sizeof (payload));
#else
    rfmTransmitPayload(payload, len, 0x24);
#endif
    rfmSleep();
}

/**
 * Writes given measurements to SD card.
 *
 * @param sdaddr address to write to
 * @param power radio power in dBm
 * @param humidity relative humidity in %
 * @param pressure barometric pressure in hPa
 * @param bmedata measurements from BME688
 * @param pasdata data from PA1616S
 * @return success
 */
static bool writeMeas(uint32_t sdaddr,
                      uint8_t power,
                      uint8_t humidity,
                      uint16_t pressure,
                      struct bme68x_data *bmedata,
                      NmeaData *pasdata) {
    char buf[SD_BLOCK_SIZE] = {0};
    snprintf(buf, sizeof (buf),
            "%lu,%u,%u,%u,%u,%u,%lu,%06lu,%u,%u,%lu,%lu,%lu,%u\n",
            pitints, bavg, power,
            bmedata->temperature,
            humidity, pressure,
            bmedata->gas_resistance,
            pasdata->utc, pasdata->fix, pasdata->sat,
            pasdata->lat, pasdata->lon,
            pasdata->alt, pasdata->speed);

    return sdcWriteSingleBlock(sdaddr, (uint8_t *)buf);
}

/**
 * Prints given measurements.
 *
 * @param power radio power in dBm
 * @param humidity relative humidity in %
 * @param pressure barometric pressure in hPa
 * @param bmedata measurements from BME688
 * @param pasdata data from PA1616S
 */
static void printMeas(uint8_t power,
                      uint8_t humidity,
                      uint16_t pressure,
                      struct bme68x_data *bmedata,
                      NmeaData *pasdata) {
    div_t tdiv = div(bmedata->temperature, 100);

    // highly sophisticated IAQ algorithm
    uint8_t aqi = 5 - min(4, bmedata->gas_resistance / 25000);

    char buf[128];
    snprintf(buf, sizeof (buf),
            "%5lus, %u mV, %u dBm, %c%u.%u°C, %u%%, %u hPa, %lu Ohm (AQI: %u)\r\n",
            pitints, bavg, power,
            bmedata->temperature < 0 ? '-' : ' ', abs(tdiv.quot), abs(tdiv.rem),
            humidity, pressure,
            bmedata->gas_resistance, aqi);
    printString(buf);

    snprintf(buf, sizeof (buf),
            "UTC: %06lu, Fix: %u, Sat: %u, Lat: %lu, Lon: %lu, Alt: %lu m, Speed: %u knots\r\n",
            pasdata->utc, pasdata->fix, pasdata->sat,
            pasdata->lat, pasdata->lon,
            pasdata->alt / 10, pasdata->speed / 100);
    printString(buf);
}

void doEns(void) {
    EnsData ensdata = {0};
    bool ensmeas = ensMeasure(ENS_I2C_ADDR_LOW, &ensdata);
    if (USART && ensmeas) {
        char buf[48];
        snprintf(buf, sizeof (buf), "AQI: %u, TVOC: %5u ppb, eCO2: %5u ppm\r\n",
                ensdata.aqi, ensdata.tvoc, ensdata.eco2);
        printString(buf);
    }
}

void doMeas(bool sdc, uint32_t sdaddr) {
    uint8_t power = rfmGetOutputPower();

    struct bme68x_data bmedata = {0};
    int bmemeas = bmeMeasure(&bmedata);

    NmeaData pasdata = {0};
    bool pasread = false;
    if (bavg < BAT_PAS_MV && pas_sta()) {
        // bat too low and PAS is on
        pas_off();
    }

    // bat okay or PAS is off
    pasread = !pas_sta() || pasRead(&pasdata);

    if (bmemeas == 0 && pasread) {
        uint8_t humidity = min(UCHAR_MAX,
                divRoundNearest(bmedata.humidity, 1000));
        uint16_t pressure = min(USHRT_MAX,
                divRoundNearest(bmedata.pressure, 100));
        uint16_t gasres = min(USHRT_MAX,
                divRoundNearest(bmedata.gas_resistance, 1000));
        uint16_t alt = min(USHRT_MAX,
                divRoundNearest(pasdata.alt, 10));

        uint8_t payload[] = {
            bmedata.temperature >> 8,
            bmedata.temperature,
            humidity,
            pressure >> 8,
            pressure,
            gasres >> 8,
            gasres,
            pasdata.fix,
            pasdata.sat,
            pasdata.lat >> 24,
            pasdata.lat >> 16,
            pasdata.lat >> 8,
            pasdata.lat,
            pasdata.lon >> 24,
            pasdata.lon >> 16,
            pasdata.lon >> 8,
            pasdata.lon,
            alt >> 8,
            alt,
            pasdata.speed >> 8,
            pasdata.speed,
            power,
            bavg >> 8,
            bavg
        };

        // here so LED is on a bit longer to be visible
        if (sdc && bavg > BAT_SDC_MV) {
            led_on();
        }

        transmitMeas(payload, sizeof (payload));

        if (sdc && bavg > BAT_SDC_MV) {
            bool sdcwrite = writeMeas(sdaddr, power, humidity, pressure,
                                      &bmedata, &pasdata);
            led_off();
            if (USART && !sdcwrite) {
                printString("Writing to SD card failed!\r\n");
            }
        }
        if (USART) {
            printMeas(power, humidity, pressure,
                      &bmedata, &pasdata);
        }
    }
}