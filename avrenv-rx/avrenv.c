/*
 * avrenv.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 * Receiver part of the avrenv project.
 *
 * Created on: 11.11.2025
 *     Author: torsten.roemer@luniks.net
 *
 */

#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

#include "utils.h"
#include "usart.h"
#include "spi.h"
#if RFM == 69
#include "librfm69/librfm69.h"
#endif
#if RFM == 95
#include "librfm95/librfm95.h"
#endif
#include "libsdc/libsdc.h"

/* Timebase used for timing internal delays */
#define TIMEBASE_VALUE  ((uint8_t) ceil(F_CPU * 0.000001))

/* Enables periodic interrupt timer */
#define enable_pit()    RTC_PITCTRLA |= RTC_PITEN_bm

/* Turns the LED on */
#define led_on()        PORTD_OUT |= (1 << LED_PD7)

/* Turns the LED off */
#define led_off()       PORTD_OUT &= ~(1 << LED_PD7)

#ifndef LORA
#define LORA    0
#endif

#define USART       1

/* Periodic interrupt timer interrupt count (seconds) */
static volatile uint32_t pitints = 0;

/* Periodic interrupt timer interrupt */
ISR(RTC_PIT_vect) {
    // clear flag or interrupt remains active
    RTC_PITINTFLAGS |= RTC_PI_bm;
    pitints++;
}

/**
 * PORTD pin interrupt handlers.
 */
ISR(PORTD_PORT_vect) {
    if (PORTD_INTFLAGS & PORT_INT_2_bm) {
        PORTD_INTFLAGS |= PORT_INT_2_bm;
        // RFM DIO0
        rfmIrq();
    }
    if (PORTD_INTFLAGS & PORT_INT_3_bm) {
        PORTD_INTFLAGS |= PORT_INT_3_bm;
        // RFM DIO4 (FSK)/DIO1 (LoRa)
        rfmIrq();
    }
}

/* Disables digital input buffer on all pins to save (a lot of) power */
static void initPins(void) {
    // disable digital input buffer on all PORTA pins
    PORTA_PINCONFIG = PORT_ISC_INPUT_DISABLE_gc;
    // copy pin config to all pins
    PORTA_PINCTRLUPD = 0xff;
    PORTC_PINCTRLUPD = 0xff;
    PORTD_PINCTRLUPD = 0xff;
    PORTF_PINCTRLUPD = 0xff;

    // enable input on USART0 RX pin
    PORTC_PIN1CTRL = PORT_ISC_INTDISABLE_gc;

    // set MOSI and SCK as output pin
    PORTA_DIRSET = (1 << MOSI_PA4);
    PORTA_DIRSET = (1 << SCK_PA6);
    // enable input on MISO pin
    PORTA_PIN5CTRL = PORT_ISC_INTDISABLE_gc;
    // enabling pull-up on MISO pin saves quite some power in sleep mode
    // disabling digital input buffer before going to sleep has the same effect
    PORTA_PIN5CTRL |= PORT_PULLUPEN_bm;

    // PD0 is radio module reset pin (output pin)
    PORTD_DIRSET = (1 << RFM_RST_PD0);

    // PD1 is radio module CS pin (output pin + pullup)
    PORTD_DIRSET = (1 << RFM_CS_PD1);
    PORTD_PIN1CTRL |= PORT_PULLUPEN_bm;

    // PD7 is LED pin (output pin)
    PORTD_DIRSET = (1 << LED_PD7);
}

/* Sets CPU and peripherals clock speed */
static void initClock(void) {
    // allow writing to protected register
    CPU_CCP = CCP_IOREG_gc;
    // enable and set main clock prescaler to DIV2 = 10 MHz
    CLKCTRL_MCLKCTRLB = CLKCTRL_PDIV_DIV2_gc | CLKCTRL_PEN_bm;
    // set timebase used for timing internal delays
    CLKCTRL_MCLKTIMEBASE |= (TIMEBASE_VALUE << CLKCTRL_TIMEBASE_gp);
}

/* Initializes the Periodic Interrupt Timer */
static void initRTC(void) {
    // clock RTC with 32.768 kHz internal osciallator
    RTC_CLKSEL |= RTC_CLKSEL_OSC32K_gc;
    // enable periodic interrupt
    RTC_PITINTCTRL |= RTC_PI_bm;
    // wait for PITCTRLA to be synchronized
    loop_until_bit_is_clear(RTC_PITSTATUS, RTC_CTRLBUSY_bp);
    // set periodic interrupt period in RTC clock cycles
    RTC_PITCTRLA |= RTC_PERIOD_CYC32768_gc;
}

/* Initializes the SPI */
static void initSPI(void) {
    // disable client select line, SS pin can be used as regular I/O pin
    SPI0_CTRLB |= SPI_SSD_bm;
    // SPI master mode, SPI enable
    SPI0_CTRLA |= SPI_MASTER_bm | SPI_ENABLE_bm;
}

/* Initializes pin interrupts */
static void initInts(void) {
    // PD2 sense rising edge (RFM DIO0)
    PORTD_PIN2CTRL = PORT_ISC_RISING_gc;
    // PD3 sense rising edge (RFM DIO4 (FSK)/DIO1 (LoRa))
    PORTD_PIN3CTRL = PORT_ISC_RISING_gc;
    // PD6 sense rising edge (ENS160 INT)
    // PORTD_PIN6CTRL = PORT_ISC_RISING_gc;
}

static void printData(uint8_t rssi, bool crc) {
    char buf[25];
    snprintf(buf, sizeof (buf), "RSSI: %4d dBm, CRC: %d\r\n",
            -rssi, crc);
    printString(buf);
}

/**
 * Receives and handles the measurements.
 */
static void receiveData(void) {
#if RFM == 95 && LORA
    RxFlags flags = rfmLoRaRxDone();
    if (flags.ready) {
        uint8_t payload[32];
        rfmLoRaRxRead(payload, sizeof (payload));
        printData(flags.rssi, flags.crc);
        rfmLoRaStartRx();
    }
#else
    // TODO only RFM69
    PayloadFlags flags = rfmPayloadReady();
    if (flags.ready) {
        uint8_t payload[32];
        rfmReadPayload(payload, sizeof (payload));
        printData(flags.rssi, flags.crc);
        rfmStartReceive(false);
    }
#endif
}

int main(void) {

    initPins();
    initClock();
    initRTC();
    initUSART();
    initSPI();
    initInts();

    if (USART) {
        printString("Hello avrenv receiver!\r\n");
        char rev[16];
        snprintf(rev, sizeof (rev), "MCU rev. %c%u\r\n",
                (SYSCFG_REVID >> 4) - 1 + 'A',
                SYSCFG_REVID & SYSCFG_MINOR_gm);
        printString(rev);
    }

    // slow down SPI for the breadboard wiring
    spiMid();

    bool radio = false;
#if RFM == 69
    radio = rfmInit(433600, 0x24, 0x84);
#endif
#if RFM == 95
    radio = rfmInit(868600, 0x24, 0x84, LORA);
#endif
    if (radio) {
        rfmSetOutputPower(2);
    } else if (USART) {
        printString("Radio init failed!\r\n");
    }

    // start PIT after (lengthy) initialization
    enable_pit();

    // enable global interrupts
    sei();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    if (radio) {
#if RFM == 95 && LORA
        rfmLoRaStartRx();
#else
        rfmStartReceive(false);
#endif
    }

    while (true) {
        if (radio) {
            led_on();
            receiveData();
            led_off();
        }

        // wait for USART tx to be done (before going to sleep)
        wait_usart_tx_done();

        // save some power
        sleep_mode();
    }

    return 0;
}
