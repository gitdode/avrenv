/*
 * File:   spi.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 1. April 2023, 18:59
 */

#include "spi.h"

void spiSlow(void) {
    // fOSC/64
    SPI0_CTRLA |= SPI_PRESC_DIV64_gc;
}

void spiMid(void) {
    // fOSC/16
    SPI0_CTRLA |= SPI_PRESC_DIV16_gc;
}

void spiFast(void) {
    // fOSC/4 (default)
    SPI0_CTRLA |= SPI_PRESC_DIV4_gc;
}

uint8_t transmit(uint8_t data) {
    SPI0_DATA = data;
    loop_until_bit_is_set(SPI0_INTFLAGS, SPI_IF_bp);

    return SPI0_DATA;
}
