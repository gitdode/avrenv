/*
 * File:   data.c
 * Author: torstem.roemer@luniks.net
 *
 * Created on 15.11.2025, 01:27
 */

#include "data.h"

/* Time delta between transmissions in seconds */
static uint16_t rxtstart = 0;

/* The awesome Unifont font */
static const __flash Font *unifont = &unifontFont;

/**
 * Transforms the given payload received from the transmitter to
 * the given structured data.
 *
 * @param payload received payload
 * @param len length of received payload
 * @param data structured data
 */
static void getData(uint8_t *payload, uint16_t len, EnvData *data) {
    data->temperature =  (uint16_t)payload[0] << 8;
    data->temperature |= (uint16_t)payload[1];
    data->humidity = payload[2];
    data->pressure =  (uint16_t)payload[3] << 8;
    data->pressure |= (uint16_t)payload[4];
    data->gasres =  (uint16_t)payload[5] << 8;
    data->gasres |= (uint16_t)payload[6];
    data->fix = payload[7];
    data->sat = payload[8];
    data->lat =  (uint32_t)payload[9] << 24;
    data->lat |= (uint32_t)payload[10] << 16;
    data->lat |= (uint32_t)payload[11] << 8;
    data->lat |= (uint32_t)payload[12];
    data->lon =  (uint32_t)payload[13] << 24;
    data->lon |= (uint32_t)payload[14] << 16;
    data->lon |= (uint32_t)payload[15] << 8;
    data->lon |= (uint32_t)payload[16];
    data->alt =  (uint16_t)payload[17] << 8;
    data->alt |= (uint16_t)payload[18];
    data->speed =  (uint16_t)payload[19] << 8;
    data->speed |= (uint16_t)payload[20];
    data->power = payload[21];
    data->voltage =  (uint16_t)payload[22] << 8;
    data->voltage |= (uint16_t)payload[23];
}

/**
 * Handles the given data received from the transmitter including
 * metrics from the receiver.
 *
 * @param rssi RSSI
 * @param crc payload CRC check result
 * @param dur duration since last transmission
 * @param data structured data from transmitter
 */
static void handleData(uint8_t rssi, bool crc, uint8_t dur, EnvData *data) {
    char buf[128];
    snprintf(buf, sizeof (buf), "%lu,%u,%u,%u,%u,%u,%d,%u,%u,%u,%u,%u,%lu,%lu,%d,%u\n",
            pitints, dur, rssi, crc,
            data->voltage, data->power,
            data->temperature, data->humidity, data->pressure, data->gasres,
            data->fix, data->sat, data->lat, data->lon, data->alt, data->speed);
    printString(buf);

    div_t tdiv = div(divRoundNearest(data->temperature, 10), 10);
    div_t sdiv = div(divRoundNearest(data->speed, 10), 10);

    // highly sophisticated IAQ algorithm
    uint8_t aqi = 5 - min(4, data->gasres / 25);

    snprintf(buf, sizeof (buf), "RSSI: %4d dBm, CRC: %u, Time: %3u s",
            -rssi, crc, dur);
    tftWriteString(0, 0, unifont, buf, BLACK, WHITE);
    snprintf(buf, sizeof (buf), "Bat: %4u mV, Pa: %2u dBm",
            data->voltage, data->power);
    tftWriteString(0, 16, unifont, buf, BLACK, WHITE);
    snprintf(buf, sizeof (buf), "T: %c%3u.%u °C, %2u%%, P: %4u hPa",
            data->temperature < 0 ? '-' : ' ', abs(tdiv.quot), abs(tdiv.rem),
            data->humidity, data->pressure);
    tftWriteString(0, 32, unifont, buf, BLACK, WHITE);
    snprintf(buf, sizeof (buf), "TVOC: %3u kOhm (Fake AQI: %d)",
            data->gasres, aqi);
    tftWriteString(0, 48, unifont, buf, BLACK, WHITE);
    snprintf(buf, sizeof (buf), "Sat Fix: %u, Nr. Sat: %2u",
            data->fix, data->sat);
    tftWriteString(0, 64, unifont, buf, BLACK, WHITE);
    snprintf(buf, sizeof (buf), "Lat: %9lu, Lon: %9lu",
            data->lat, data->lon);
    tftWriteString(0, 80, unifont, buf, BLACK, WHITE);
    snprintf(buf, sizeof (buf), "Alt: %5d m, Speed: %3u.%u kn",
            data->alt, sdiv.quot, sdiv.rem);
    tftWriteString(0, 96, unifont, buf, BLACK, WHITE);
}

void receiveData(void) {
#if LORA
     RxFlags flags = rfmLoRaRxDone();
#else
     PayloadFlags flags = rfmPayloadReady();
#endif
    if (flags.ready) {
        uint8_t dur = min(UCHAR_MAX, pitints - rxtstart);
        rxtstart = pitints;

        uint8_t payload[PAYLOAD_LEN];
#if LORA
        uint8_t len = rfmLoRaRxRead(payload, sizeof (payload));
#else
        uint8_t len = rfmReadPayload(payload, sizeof (payload));
#endif
        if (len - 1 == PAYLOAD_LEN) { // len includes address byte
            EnvData data = {0};
            getData(payload, sizeof (payload), &data);
            handleData(flags.rssi, flags.crc, dur, &data);
        } else {
            printString("Received payload with unexpected length\r\n");
        }

#if LORA
        rfmLoRaStartRx();
#else
        rfmStartReceive(false);
#endif
    }
}
