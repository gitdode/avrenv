/*
 * File:   pa1616s.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 28.10.2025, 20:47
 */

#include "pa1616s.h"

bool pasInit(void) {
    // translated baud rate
    USART0_BAUD = UBRR;
    // async, no parity, 1 stop bit, 8 bit (POR)
    USART0_CTRLC = (0x03 << USART_CHSIZE_gp);
    // set TxD as output pin
    PORTA_DIRSET |= (1 << USART_TX_PA0);
    // enable transmitter
    USART0_CTRLB |= (1 << USART_TXEN_bp);

    // TODO what can go wrong?
    return true;
}

/*
 * TODO
 * use in conjunction with i.e. '$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29'
 * to get only GPGGA sentences
 */
uint8_t getNmeaMsg(char *data, uint8_t len) {
    // enable receiver
    USART0_CTRLB |= (1 << USART_RXEN_bp);

    uint8_t cnt = 0;
    bool msg = false;
    while (cnt < len - 1) {
        if (bit_is_set(USART0_STATUS, USART_RXCIF_bp)) {
            char c = USART0_RXDATAL;
            if (c == '$') msg = true;
            if (msg && c == '\r') break;
            if (msg) {
                data[cnt] = c;
                cnt++;
            }
        }
    }
    data[cnt] = '\0';

    // disable receiver
    USART0_CTRLB &= ~(1 << USART_RXEN_bp);

    return cnt;
}