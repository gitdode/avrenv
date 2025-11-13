/*
 * usart.c
 *
 *  Created on: 16.05.2025
 *      Author: torsten.roemer@luniks.net
 */

#include "usart.h"

static volatile bool usartReceived = false;

char usartData[USART_LENGTH];

/**
 * Called when data was received via USART.
 */
ISR(USART1_RXC_vect) {
    if (!usartReceived && bit_is_set(USART1_STATUS, USART_RXCIF_bp)) {
        char data = USART1_RXDATAL;
        size_t length = strlen(usartData);
        if (length < USART_LENGTH - 1 && data != '\n' && data != '\r') {
            usartData[length] = data;
        } else {
            usartData[length] = '\0';
            usartReceived = true;
        }
    }
}

void initUSART(void) {
    // translated baud rate
    USART1_BAUD = UBRR;
    // async, no parity, 1 stop bit, 8 bit (POR)
    USART1_CTRLC = (0x03 << USART_CHSIZE_gp);
    // set TxD as output pin
    PORTC_DIRSET |= (1 << USART_TX_PC0);
    // enable only transmitter
    USART1_CTRLB |= (1 << USART_TXEN_bp);
    // enable receive complete interrupt
    // USART1_CTRLA |= (1 << USART_RXCIE_bp);
}

bool isUSARTReceived(void) {
    return usartReceived;
}

void getUSARTData(char *data, size_t size) {
    if (size > 0) {
        data[0] = '\0';
        strncat(data, usartData, size - 1);
        memset(usartData, 0, USART_LENGTH);
        usartReceived = false;
    }
}

void printString(const char *data) {
    USART1_STATUS |= USART_TXCIF_bm;
    uint8_t i = 0;
    while (data[i]) {
        loop_until_bit_is_set(USART1_STATUS, USART_DREIF_bp);
        USART1_TXDATAL = data[i];
        i++;
    }
}

void printInt(int16_t data) {
    char buf[9];
    snprintf(buf, sizeof (buf), "%d\r\n", data);
    printString(buf);
}

void printHex(uint16_t data) {
    char buf[7];
    snprintf(buf, sizeof (buf), "0x%02x\r\n", data);
    printString(buf);
}

void printByte(uint8_t byte) {
    char string[] = {'0', 'b', '?', '?', '?', '?', '?', '?', '?', '?', '\r', '\n', '\0'};
    for (uint8_t i = 8; i-- > 0; ) {
        string[9 - i] = byte & (1 << i) ? '1' : '0';
    }
    printString(string);
}
