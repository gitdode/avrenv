/*
 * File:   pa1616s.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 28.10.2025, 20:47
 */

#include <util/delay.h>

#include "pa1616s.h"

/**
 * Calculates the checksum by XORing the characters of the given string
 * until the '*' character, compares it with the checksum included in the
 * message and returns the result.
 *
 * @param msg message to be checked
 * @return check result
 */
static bool checkSum(char *msg) {
    char ckstr[3];
    uint8_t cksum = 0;
    for (; *msg != '\0'; msg++) {
        if (*msg == '*') {
            memcpy(ckstr, ++msg, sizeof (ckstr));
            break;
        } else {
            cksum ^= *msg;
        }
    }

    return xstrtoi(ckstr) == cksum;
}

/**
 * Writes the given command to the module.
 *
 * @param data command
 */
static void writeCmd(const char *data) {
    USART0_STATUS |= USART_TXCIF_bm;
    uint8_t i = 0;
    while (data[i]) {
        loop_until_bit_is_set(USART0_STATUS, USART_DREIF_bp);
        USART0_TXDATAL = data[i];
        i++;
    }
}

/**
 * Reads one or many lines of output from the module,
 * starting with the first occurrence of '$'.
 *
 * @param data lines of output
 * @param num number of lines to be read
 * @param len length of the given data array
 * @return number of lines read
 */
// TODO timeout blocking function?
static uint8_t readOut(char *data, uint8_t num, uint16_t len) {
    uint8_t cnt = 0;
    for (; cnt < num; cnt++) {
        uint16_t pos = cnt * PAS_NMEA_LEN;
        bool msg = false;
        while (pos < len - 1) {
            if (bit_is_set(USART0_STATUS, USART_RXCIF_bp)) {
                char c = USART0_RXDATAL;
                if (!msg) {
                    if (c == '$') msg = true;
                } else {
                    if (c == '\r') continue;
                    if (c == '\n') break;
                    data[pos] = c;
                    pos++;
                }
            }
        }
        data[pos] = '\0';
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
    readOut(data, 1, PAS_NMEA_LEN);

    disable_rx();

    // check ack
    int8_t ack = strcmp(PAS_ACK, data);

    return ack == 0;
}

bool pasRead(NmeaData *data) {
    char nmea[PAS_NMEA_CNT * PAS_NMEA_LEN];
    enable_rx();
    uint8_t cnt = readOut(nmea, PAS_NMEA_CNT, PAS_NMEA_CNT * PAS_NMEA_LEN);
    disable_rx();

    for (uint8_t i = 0; i < cnt; i++) {
        char *token;
        char *msg = &nmea[i * PAS_NMEA_LEN];
        bool valid = checkSum(msg);
        if (strncmp(PAS_GPGGA, msg, PAS_ID_LEN) == 0) {
            uint8_t i = 0;
            while ((token = strsep(&msg, PAS_NMEA_FS))) {
                switch (i) {
                    case 1: data->utc = atol(token); break;
                    case 2: data->lat = strtof(token, NULL) * 10000; break;
                    case 4: data->lon = strtof(token, NULL) * 10000; break;
                    case 6: data->fix = atol(token); break;
                    case 7: data->sat = atol(token); break;
                    case 9: data->alt = strtof(token, NULL) * 10; break;
                    default: break;
                }
                i++;
            };

            if (i != PAS_GPGGA_LEN || !valid) return false;
        } else if (strncmp(PAS_GPRMC, msg, PAS_ID_LEN) == 0) {
            uint8_t i = 0;
            while ((token = strsep(&msg, PAS_NMEA_FS))) {
                switch (i) {
                    case 7: data->speed = strtof(token, NULL) * 100; break;
                    default: break;
                }
                i++;
            };

            if (i != PAS_GPRMC_LEN || !valid) return false;
        }
    }

    return cnt == PAS_NMEA_CNT;
}
