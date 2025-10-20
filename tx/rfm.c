/*
 * File:   rfm.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 24.03.2025, 21:07
 */

#include <util/delay.h>

#if RFM == 69
    #include "librfm69/librfm69.h"
#endif
#if RFM == 95
    #include "librfm95/librfm95.h"
#endif
#include "pins.h"
#include "spi.h"

void _rfmDelay5() {
    _delay_ms(5);
}

void _rfmOn() {
#if RFM == 69
    PORTD_OUTCLR = (1 << RFM_RST_PD0);
#endif
#if RFM == 95
    PORTD_OUTSET = (1 << RFM_RST_PD0);
#endif
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