/*
 * File:   tft.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 06.07.2025, 20:26
 */

#include <util/delay.h>

#include "libtft/libtft.h"
#include "pins.h"
#include "spi.h"

void _tftDelay100() {
    _delay_ms(100);
}

void _tftSel() {
    PORTD_OUT &= ~(1 << TFT_CS_PD4);
}

void _tftDes() {
    PORTD_OUT |= (1 << TFT_CS_PD4);
}

void _tftSetCmd(void) {
    PORTD_OUT &= ~(1 << TFT_DC_PD5);
}

void _tftSetData(void){
    PORTD_OUT |= (1 << TFT_DC_PD5);
}

void _tftReset(void) {
    PORTD_OUT &= ~(1 << TFT_RST_PD6);
    _delay_ms(10);
    PORTD_OUT |= (1 << TFT_RST_PD6);
}

uint8_t _tftTx(uint8_t data) {
    return transmit(data);
}