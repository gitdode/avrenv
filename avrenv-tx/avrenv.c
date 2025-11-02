/*
 * avrenv.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 * Transmitter part of the avrenv project.
 *
 * Created on: 20.10.2025
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
#include "bme688.h"
#include "ens160.h"
#include "pa1616s.h"

/* Timebase used for timing internal delays */
#define TIMEBASE_VALUE  ((uint8_t) ceil(F_CPU * 0.000001))

/* Enables periodic interrupt timer */
#define enable_pit()    RTC_PITCTRLA |= RTC_PITEN_bm

#ifndef LORA
    #define LORA    0
#endif

#define ENS160      0

#define USART       1

/* Awake/busy interval in seconds */
#define INTERVAL    8

/* Periodic interrupt timer interrupt count (seconds) */
static volatile uint32_t pitints = 0;

/** Averaged battery voltage in millivolts */
static uint16_t bavg;

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
    if (PORTD_INTFLAGS & PORT_INT_6_bm) {
        PORTD_INTFLAGS |= PORT_INT_6_bm;
        // ENS new output data available
        ensIrq();
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

    // enable input on USART0 and USART1 RX pins
    PORTA_PIN1CTRL = PORT_ISC_INTDISABLE_gc;
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

    // PD4 is BME68x CS pin (output pin + pullup)
    PORTD_DIRSET = (1 << BME_CS_PD4);
    PORTD_PIN4CTRL |= PORT_PULLUPEN_bm;

    if (ENS160) {
        // PD5 is ENS120 CS pin (output pin + drive high)
        PORTD_DIRSET = (1 << ENS_CS_PD5);
        PORTD_OUTSET = (1 << ENS_CS_PD5);
    }
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

/* Initializes the ADC */
static void initADC(void) {
    // enable ADC0
    ADC0_CTRLA |= ADC_ENABLE_bm;
    // set ADC clock prescaler
    ADC0_CTRLB = ADC_PRESC_DIV6_gc;
    // use VDD as reference voltage
    ADC0_CTRLC = ADC_REFSEL_VDD_gc;
    // set sample duration to 16.5 * CLK_ADC
    ADC0_CTRLE = (16 << ADC_SAMPDUR_gp);
    // enable programmable gain amplifier (default 1x)
    // ADC0_PGACTRL |= (ADC_PGAEN_bm);
    // configure single 12-bit mode of operation
    ADC0_COMMAND |= ADC_MODE_SINGLE_12BIT_gc;
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

/**
 * Starts an immediate conversion and returns the result.
 *
 * @param ref reference voltage
 * @param pin input pin
 * @param dur sample duration
 * @return 12-bit conversion result
 */
static uint16_t convert(ADC_REFSEL_t ref, ADC_MUXPOS_t pin, uint8_t dur) {
    // set the given reference voltage
    ADC0_CTRLC = ref;
    // set positive input to given pin
    ADC0_MUXPOS = pin;
    // set the sample duration
    ADC0_CTRLE = dur;
    // time to settle
    _delay_us(60);
    // start an immediate conversion
    ADC0_COMMAND |= ADC_START_IMMEDIATE_gc;
    // wait for "result ready" interrupt flag
    do {} while (!(ADC0_INTFLAGS & ADC_RESRDY_bm));

    // return conversion result (clears flag)
    return ADC0_RESULT;
}

/**
 * Measures the battery voltage divided by two with 2.048V reference voltage
 * and returns it in millivolt.
 *
 * @return battery voltage
 */
static int16_t measureBat(void) {
    uint32_t adc = convert(ADC_REFSEL_2V048_gc, ADC_MUXPOS_AIN22_gc, 255);
    uint16_t mv = (adc * 2048 * 2) >> 12;

    return mv;
}

/**
 * Prints given measurements.
 *
 * @param power radio power in dBm
 * @param humidity relative humidity in %
 * @param pressure barometric pressure in hPa
 * @param data measurements from BME688
 */
static void printMeas(uint8_t power,
                      uint8_t humidity,
                      uint16_t pressure,
                      struct bme68x_data *data,
                      NmeaData *pasdata) {
    div_t tdiv = div(data->temperature, 100);

    // highly sophisticated IAQ algorithm
    uint8_t aqi = 5 - min(4, data->gas_resistance / 15000);

    char buf[128];
    snprintf(buf, sizeof (buf),
            "%5lus, %u mV, %u dBm, %c%u.%u°C, %u%%, %u hPa, %lu Ohm (AQI: %u)\r\n",
            pitints, bavg, power,
            data->temperature < 0 ? '-' : ' ', abs(tdiv.quot), abs(tdiv.rem),
            humidity, pressure,
            data->gas_resistance, aqi);
    printString(buf);

    snprintf(buf, sizeof (buf),
            "UTC: %06lu, Fix: %u, Sat: %u, Lat: %lu, Lon: %lu, Alt: %lu m, Speed: %u knots\r\n",
            pasdata->utc, pasdata->fix, pasdata->sat,
            pasdata->lat, pasdata->lon,
            pasdata->alt / 10, pasdata->speed / 100);
    printString(buf);
}

int main(void) {

    initPins();
    initClock();
    initRTC();
    initADC();
    initUSART();
    initSPI();
    initInts();

    if (USART) {
        printString("Hello avrenv transmitter!\r\n");
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
        radio = rfmInit(433600, 0x28, 0x84);
    #endif
    #if RFM == 95
        radio = rfmInit(868600, 0x28, 0x84, LORA);
    #endif
    if (radio) {
        rfmSetOutputPower(2);
    } else if (USART) {
        printString("Radio init failed!\r\n");
    }

    static SpiCs bmeSpiCs = {.port = &PORTD_OUT, .pin = BME_CS_PD4};
    int8_t bme = bmeInit(300, 100, 20, &bmeSpiCs);
    if (USART && bme != 0) {
        printString("BME688 init failed!\r\n");
        printInt(bme);
    }

    bool ens = false;
    if (ENS160) {
        static SpiCs ensSpiCs = {.port = &PORTD_OUT, .pin = ENS_CS_PD5};
        ens = ensInit(&ensSpiCs);
        if (USART && !ens) {
            printString("ENS160 init failed!\r\n");
        }
    }

    bool pas = pasInit();
    if (USART && !pas) {
        printString("PA1616S init failed!\r\n");
    }

    // start PIT after (lengthy) initialization
    enable_pit();

    // enable global interrupts
    sei();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    while (true) {
        if (pitints % INTERVAL == 0) {

            uint16_t bat = measureBat();
            if (bavg == 0) bavg = bat;
            bavg = (bavg + bat) >> 1;

            if (bavg < 2100) {
                if (USART) printString("Battery low\r\n");
            } else {
                if (ens) {
                    static EnsData ensdata = {0};
                    bool ensmeas = ensMeasure(&ensdata);
                    if (USART && ensmeas) {
                        char buf[48];
                        snprintf(buf, sizeof (buf), "AQI: %u, TVOC: %5u ppb, eCO2: %5u ppm\r\n",
                                ensdata.aqi, ensdata.tvoc, ensdata.eco2);
                        printString(buf);
                    }
                }
                if (radio && bme == 0 && pas) {
                    uint8_t power = rfmGetOutputPower();

                    static struct bme68x_data bmedata = {0};
                    int bmemeas = bmeMeasure(&bmedata);

                    NmeaData pasdata = {0};
                    bool pasread = pasRead(&pasdata);

                    if (bmemeas == 0 && pasread) {
                        uint8_t humidity = min(UCHAR_MAX,
                                divRoundNearest(bmedata.humidity, 1000));
                        uint16_t pressure = min(USHRT_MAX,
                                divRoundNearest(bmedata.pressure, 100));
                        uint16_t gasres = min(USHRT_MAX,
                                divRoundNearest(bmedata.gas_resistance, 1000));
                        uint16_t alt = min(USHRT_MAX,
                                divRoundNearest(pasdata.alt, 10));

                        uint8_t payload[] = {
                            bmedata.temperature >> 8,
                            bmedata.temperature,
                            humidity,
                            pressure >> 8,
                            pressure,
                            gasres >> 8,
                            gasres,
                            pasdata.fix,
                            pasdata.sat,
                            pasdata.lat >> 24,
                            pasdata.lat >> 16,
                            pasdata.lat >> 8,
                            pasdata.lat,
                            pasdata.lon >> 24,
                            pasdata.lon >> 16,
                            pasdata.lon >> 8,
                            pasdata.lon,
                            alt >> 8,
                            alt,
                            pasdata.speed >> 8,
                            pasdata.speed,
                            power,
                            bavg >> 8,
                            bavg
                        };
                        rfmWake();
                        rfmTransmitPayload(payload, sizeof (payload), 0x24);
                        rfmSleep();

                        if (USART) {
                            printMeas(power, humidity, pressure, &bmedata, &pasdata);
                        }
                    }
                }
            }

            // wait for USART tx to be done (before going to sleep)
            wait_usart_tx_done();
        }

        // save some power
        sleep_mode();
    }

    return 0;
}
