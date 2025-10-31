/*
 * File:   pa1616s.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 28.10.2025, 20:47
 */

#include <util/delay.h>

#include "pa1616s.h"

static void writeCmd(const char *data) {
    USART0_STATUS |= USART_TXCIF_bm;
    uint8_t i = 0;
    while (data[i]) {
        loop_until_bit_is_set(USART0_STATUS, USART_DREIF_bp);
        USART0_TXDATAL = data[i];
        i++;
    }
}

static uint8_t readSingle(char *data) {
    uint8_t pos = 0;
    while (pos < PAS_NMEA_LEN - 1) {
        if (bit_is_set(USART0_STATUS, USART_RXCIF_bp)) {
            char c = USART0_RXDATAL;
            if (c == '\r') continue;
            if (c == '\n') break;
            data[pos] = c;
            pos++;
        }
    }
    data[pos] = '\0';

    return pos;
}

static uint8_t grabMany(char data[PAS_NMEA_CNT][PAS_NMEA_LEN]) {
    uint8_t cnt = 0;
    while (cnt < PAS_NMEA_CNT) {
        uint8_t pos = 0;
        bool msg = false;
        while (pos < PAS_NMEA_LEN - 1) {
            if (bit_is_set(USART0_STATUS, USART_RXCIF_bp)) {
                char c = USART0_RXDATAL;
                if (!msg) {
                    if (c == '$') msg = true;
                } else {
                    if (c == '\r') continue;
                    if (c == '\n') break;
                    data[cnt][pos] = c;
                    pos++;
                }
            }
        }
        data[cnt][pos] = '\0';
        cnt++;
    }

    return cnt;
}

bool pasInit(void) {
    // translated baud rate
    USART0_BAUD = UBRR;
    // async, no parity, 1 stop bit, 8 bit (POR)
    USART0_CTRLC = (0x03 << USART_CHSIZE_gp);
    // set TxD as output pin
    PORTA_DIRSET |= (1 << USART_TX_PA0);
    // enable transmitter
    USART0_CTRLB |= (1 << USART_TXEN_bp);

    /*
     $CDACK,34,0*79
     $CDACK,103*50
     $CDACK,105*56
     $PMTK011,MTKGPS*08
     $PMTK010,001*2E
     $PMTK010,002*2D
     */

    // ignore (power on) messages for 1 second
    _delay_ms(1000);

    // configure no output and wait for it to stop
    writeCmd(PAS_OUT_NONE);
    _delay_ms(1000);

    enable_rx();

    // configure to output only GGA and RMC messages
    writeCmd(PAS_OUT_GGA_RMC);

    // read ack
    char data[PAS_NMEA_LEN];
    readSingle(data);

    disable_rx();

    // check ack
    int8_t ack = strcmp(PAS_ACK, data);

    return ack == 0;
}

// TODO check checksum
bool pasRead(NmeaData *data) {
    char nmea[PAS_NMEA_CNT][PAS_NMEA_LEN];
    enable_rx();
    uint8_t cnt = grabMany(nmea);
    disable_rx();

    for (uint8_t i = 0; i < cnt; i++) {
        char *token;
        char *string = nmea[i];
        if (strncmp(PAS_GPGGA, nmea[i], PAS_ID_LEN) == 0) {
            uint8_t i = 0;
            while ((token = strsep(&string, PAS_NMEA_FS))) {
                if (i == 1) data->utc = atol(token);
                if (i == 2) data->lat = strtof(token, NULL) * 10000;
                if (i == 4) data->lon = strtof(token, NULL) * 10000;
                if (i == 6) data->fix = atol(token);
                if (i == 7) data->sat = atol(token);
                if (i == 9) data->alt = strtof(token, NULL) * 10;
                i++;
            };

            if (i != PAS_GPGGA_LEN) return false;
        } else if (strncmp(PAS_GPRMC, nmea[i], PAS_ID_LEN) == 0) {
            uint8_t i = 0;
            while ((token = strsep(&string, PAS_NMEA_FS))) {
                if (i == 7) data->speed = strtof(token, NULL) * 100;
                i++;
            };

            if (i != PAS_GPRMC_LEN) return false;
        }
    }

    return cnt != PAS_NMEA_CNT;
}
