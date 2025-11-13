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
#include <util/atomic.h>

#include "utils.h"
#include "usart.h"
#include "spi.h"
#if RFM == 69
    #include "librfm69/librfm69.h"
#endif
#if RFM == 95
    #include "librfm95/librfm95.h"
#endif
#include "libtft/libtft.h"
#include "libtft/unifont.h"

#define BLACK           0x0000
#define RED             0xf800
#define WHITE           0xffff

/* Timebase used for timing internal delays */
#define TIMEBASE_VALUE  ((uint8_t) ceil(F_CPU * 0.000001))

/* Enables periodic interrupt timer */
#define enable_pit()    RTC_PITCTRLA |= RTC_PITEN_bm

#ifndef LORA
    #define LORA    0
#endif

#define USART       1

/* Represents the data received from the transmitter */
typedef struct {
    /* Tx power in dBm */
    uint8_t power;
    /* Battery voltage in mV */
    uint16_t voltage;
    /* Temperature in degrees Celsius * 100 */
    int16_t temperature;
    /* Relative humidity in % */
    uint8_t humidity;
    /* Barometric pressure in hPa */
    uint16_t pressure;
    /* Gas sensor resistance in kOhm */
    uint16_t gasres;
    /* Latitude in degrees minutes (WGS84) x 10000 */
    uint32_t lat;
    /* Longitude in degrees minutes (WGS84) x 10000 */
    uint32_t lon;
    /* Fix: 0 = not available, 1 = GPS, 2 = differential GPS */
    uint8_t fix;
    /* Number of satellites used */
    uint8_t sat;
    /* Altitude in meters */
    uint16_t alt;
    /* Speed over ground in knots * 100 */
    uint16_t speed;
} EnvData;

/* Periodic interrupt timer interrupt count (seconds) */
static volatile uint32_t pitints = 0;

/* Time delta between transmissions in seconds */
static uint16_t rxtstart = 0;

/* The awesome Unifont font */
static const __flash Font *unifont = &unifontFont;

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

    // TFT CS, D/C and RESET pins (output pins)
    PORTD_DIRSET = (1 << TFT_CS_PD4);
    PORTD_DIRSET = (1 << TFT_DC_PD5);
    PORTD_DIRSET = (1 << TFT_RST_PD6);
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
}

/**
 * Transforms the given payload received from the transmitter to
 * the given structured data.
 *
 * @param payload received payload
 * @param len length of received payload
 * @param data structured data
 */
static void getData(uint8_t *payload, uint16_t len, EnvData *data) {
    data->temperature =  (uint16_t)payload[0] << 8;
    data->temperature |= (uint16_t)payload[1];
    data->humidity = payload[2];
    data->pressure =  (uint16_t)payload[3] << 8;
    data->pressure |= (uint16_t)payload[4];
    data->gasres =  (uint16_t)payload[5] << 8;
    data->gasres |= (uint16_t)payload[6];
    data->fix = payload[7];
    data->sat = payload[8];
    data->lat =  (uint32_t)payload[9] << 24;
    data->lat |= (uint32_t)payload[10] << 16;
    data->lat |= (uint32_t)payload[11] << 8;
    data->lat |= (uint32_t)payload[12];
    data->lon =  (uint32_t)payload[13] << 24;
    data->lon |= (uint32_t)payload[14] << 16;
    data->lon |= (uint32_t)payload[15] << 8;
    data->lon |= (uint32_t)payload[16];
    data->alt =  (uint16_t)payload[17] << 8;
    data->alt |= (uint16_t)payload[18];
    data->speed =  (uint16_t)payload[19] << 8;
    data->speed |= (uint16_t)payload[20];
    data->power = payload[21];
    data->voltage =  (uint16_t)payload[22] << 8;
    data->voltage |= (uint16_t)payload[23];
}

/**
 * Handles the given data received from the transmitter including
 * metrics from the receiver.
 *
 * @param rssi RSSI
 * @param crc payload CRC check result
 * @param dur duration since last transmission
 * @param data structured data from transmitter
 */
static void handleData(uint8_t rssi, bool crc, uint8_t dur, EnvData *data) {
    char buf[96];
    snprintf(buf, sizeof (buf), "%u,%u,%u,%u,%u,%d,%u,%u,%u,%u,%u,%lu,%lu,%u,%u\r\n",
            rssi, crc, dur,
            data->voltage, data->power,
            data->temperature, data->humidity, data->pressure, data->gasres,
            data->fix, data->sat, data->lat, data->lon, data->alt, data->speed);
    printString(buf);

    div_t tdiv = div(divRoundNearest(data->temperature, 10), 10);
    div_t sdiv = div(divRoundNearest(data->speed, 10), 10);

    // highly sophisticated IAQ algorithm
    uint8_t aqi = 5 - min(4, data->gasres / 25);

    snprintf(buf, sizeof (buf), "RSSI: %4d dBm, CRC: %u, Time: %3u s",
            -rssi, crc, dur);
    tftWriteString(0, 0, unifont, buf, BLACK, WHITE);
    snprintf(buf, sizeof (buf), "Bat: %4u mV, Pa: %2u dBm",
            data->voltage, data->power);
    tftWriteString(0, 16, unifont, buf, BLACK, WHITE);
    snprintf(buf, sizeof (buf), "T: %c%3u.%u °C, %2u%%, P: %4u hPa",
            data->temperature < 0 ? '-' : ' ', abs(tdiv.quot), abs(tdiv.rem),
            data->humidity, data->pressure);
    tftWriteString(0, 32, unifont, buf, BLACK, WHITE);
    snprintf(buf, sizeof (buf), "TVOC: %3u kOhm (Fake AQI: %d)",
            data->gasres, aqi);
    tftWriteString(0, 48, unifont, buf, BLACK, WHITE);
    snprintf(buf, sizeof (buf), "Sat Fix: %u, Nr. Sat: %2u",
            data->fix, data->sat);
    tftWriteString(0, 64, unifont, buf, BLACK, WHITE);
    snprintf(buf, sizeof (buf), "Lat: %9lu, Lon: %9lu",
            data->lat, data->lon);
    tftWriteString(0, 80, unifont, buf, BLACK, WHITE);
    snprintf(buf, sizeof (buf), "Alt: %5u m, Speed: %3u.%u kn",
            data->alt, sdiv.quot, sdiv.rem);
    tftWriteString(0, 96, unifont, buf, BLACK, WHITE);
}

/**
 * Receives and handles the data from the receiver and goes
 * back to receive mode when done.
 */
static void receiveData(void) {
    PayloadFlags flags = rfmPayloadReady();
    if (flags.ready) {
        uint8_t dur = min(UCHAR_MAX, pitints - rxtstart);
        rxtstart = pitints;

        uint8_t payload[24];
        rfmReadPayload(payload, sizeof (payload));

        EnvData data = {0};
        getData(payload, sizeof (payload), &data);
        handleData(flags.rssi, flags.crc, dur, &data);

        rfmStartReceive(false);
    }
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
    radio = rfmInit(433600, 0x24, 0x84);
    if (radio) {
        rfmSetOutputPower(2);
    } else if (USART) {
        printString("Radio init failed!\r\n");
    }

    tftInit(DISPLAY_WIDTH, DISPLAY_HEIGHT, HFLIP, VFLIP, BGR, INVERT);

    spiFast();

    tftSetFrame(BLACK);

    // start PIT after (lengthy) initialization
    enable_pit();

    // enable global interrupts
    sei();

    if (radio) {
        rfmStartReceive(false);
    }

    while (true) {
        if (radio) {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                receiveData();
            }
        }
    }

    return 0;
}
