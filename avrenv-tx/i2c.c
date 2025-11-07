/*
 * File:   i2c.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 06.11.2025, 17:3
 */

#include "i2c.h"

void i2cStartWrite(uint8_t addr) {
    TWI0_MADDR = addr;
    loop_until_bit_is_set(TWI0_MSTATUS, TWI_WIF_bp);
}

void i2cStartRead(uint8_t addr) {
    TWI0_MADDR = addr | 0x01;
    loop_until_bit_is_clear(TWI0_MSTATUS, TWI_RXACK_bp);
}

void i2cStop(void) {
    TWI0_MCTRLB |= TWI_MCMD_STOP_gc;
}

void i2cWrite(uint8_t data) {
    TWI0_MDATA = data;
    loop_until_bit_is_set(TWI0_MSTATUS, TWI_WIF_bp);
}

uint8_t i2cReadAck(void) {
    loop_until_bit_is_set(TWI0_MSTATUS, TWI_RIF_bp);
    TWI0_MCTRLB &= ~TWI_ACKACT_bm;
    // TWI0_MCTRLB |= TWI_MCMD_RECVTRANS_gc;

    return TWI0_MDATA;
}

uint8_t i2cReadNack(void) {
    loop_until_bit_is_set(TWI0_MSTATUS, TWI_RIF_bp);
    TWI0_MCTRLB |= TWI_ACKACT_bm;
    // TWI0_MCTRLB |= TWI_MCMD_RECVTRANS_gc;

    return TWI0_MDATA;
}