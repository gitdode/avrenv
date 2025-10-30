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
    char data[NMEA_LEN];
    readSingle(data);

    disable_rx();

    // check ack
    int8_t ack = strcmp(PAS_ACK, data);

    return ack == 0;
}

bool pasRead(NmeaData *data) {
    char nmea[NMEA_CNT][NMEA_LEN];
    enable_rx();
    uint8_t cnt = grabMany(nmea);
    disable_rx();
    USART0_CTRLB &= ~(1 << USART_RXEN_bp);

    for (uint8_t i = 0; i < cnt; i++) {
        if (strncmp("$GPGGA", nmea[i], 6) == 0) {
            printString("GGA\r\n");
        } else if (strncmp("$GPRMC", nmea[i], 6) == 0) {
            printString("RMC\r\n");
        }
        printString(nmea[i]);
        printString("\r\n");
    }

    return true;
}
