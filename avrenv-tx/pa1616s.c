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
    char c;
    while ((c = data[i++]) != '\0') {
        loop_until_bit_is_set(USART0_STATUS, USART_DREIF_bp);
        USART0_TXDATAL = c;
    }
}

static uint8_t readSingle(char *data) {
    uint8_t pos = 0;
    while (pos < NMEA_LEN - 1) {
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

static uint8_t grabMany(char data[NMEA_CNT][NMEA_LEN]) {
    uint8_t cnt = 0;
    while (cnt < NMEA_CNT) {
        uint8_t pos = 0;
        bool msg = false;
        while (pos < NMEA_LEN - 1) {
            if (bit_is_set(USART0_STATUS, USART_RXCIF_bp)) {
                char c = USART0_RXDATAL;
                if (!msg && c == '$') msg = true;
                if (msg) {
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
    // enable transmitter and receiver
    USART0_CTRLB |= (1 << USART_TXEN_bp) | (1 << USART_RXEN_bp);

    // FIXME first read all other acks on power on

    // output only GGA and RMC
    writeCmd(PAS_SET_OUTPUT);

    // read ACK
    char data[NMEA_LEN];
    readSingle(data);
    int8_t ack = strcmp(PAS_ACK, data);

    // disable receiver
    USART0_CTRLB &= ~(1 << USART_RXEN_bp);

    return true;
}

bool pasRead(NmeaData *data) {
    char nmea[NMEA_CNT][NMEA_LEN];
    // enable receiver
    USART0_CTRLB |= (1 << USART_RXEN_bp);
    uint8_t cnt = grabMany(nmea);
    // disable receiver
    USART0_CTRLB &= ~(1 << USART_RXEN_bp);

    for (uint8_t i = 0; i < cnt; i++) {
        printString(nmea[i]);
        printString("\r\n");
    }

    return true;
}
