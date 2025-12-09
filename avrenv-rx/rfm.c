/*
 * File:   rfm.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 24.03.2025, 21:07
 */

#include <util/delay.h>

#include "librfm95/librfm95.h"
#include "pins.h"
#include "spi.h"

void _rfmDelay5() {
    _delay_ms(5);
}

void _rfmOn() {
    PORTD_OUTCLR = (1 << RFM_RST_PD0);
}

void _rfmSel() {
    PORTD_OUTCLR = (1 << RFM_CS_PD1);
}

void _rfmDes() {
    PORTD_OUTSET = (1 << RFM_CS_PD1);
}

uint8_t _rfmTx(uint8_t data) {
    return transmit(data);
}