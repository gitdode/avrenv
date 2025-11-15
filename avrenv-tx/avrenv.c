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
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "data.h"

/* Timebase used for timing internal delays */
#define TIMEBASE_VALUE  ((uint8_t) ceil(F_CPU * 0.000001))

/* Enables periodic interrupt timer */
#define enable_pit()    RTC_PITCTRLA |= RTC_PITEN_bm

/* FSK or LoRa modulation scheme */
#ifndef LORA
    #define LORA    0
#endif

/* Radio frequency in kHz */
#define FREQUENCY   433600
/* Node address (FSK) */
#define NODE_ADDR   0x28
/* Broadcast address (FSK) */
#define CAST_ADDR   0x84

/* BME688 heater temperature in °C */
#define HEATER_TEMP 300
/* BME688 heater duration in ms */
#define HEATER_DUR  100
/* Initial ambient temperature in °C */
#define AMB_TEMP    20

/* Enable ENS160 gas sensor */
#define ENS160      0

/* Awake/busy interval in seconds */
#define INTERVAL    4

/* Battery low threshold */
#define BAT_LOW_MV  3000

/* Periodic interrupt timer interrupt count (seconds) */
volatile uint32_t pitints = 0;

/** Averaged battery voltage in millivolts */
uint16_t bavg;

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

    // enable pull-up on I2C SDA and SCL pins
    // PORTA_PIN2CTRL |= PORT_PULLUPEN_bm;
    // PORTA_PIN3CTRL |= PORT_PULLUPEN_bm;

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

    // PD6 is SD card CS pin (output pin + pullup)
    PORTD_DIRSET = (1 << SDC_CS_PD6);
    PORTD_PIN6CTRL |= PORT_PULLUPEN_bm;

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

/* Initializes the Watchdog */
static void initWDT(void) {
    // allow writing to protected register
    CPU_CCP = CCP_IOREG_gc;
    // 8 seconds watchdog period
    WDT_CTRLA |= WDT_PERIOD_8KCLK_gc;
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

/* Initializes the I2C */
static void initI2C(void) {
    // set host baud rate
    TWI0_MBAUD = 94; // 50 kHz
    // enable TWI host with smart mode enabled
    TWI0_MCTRLA |= TWI_SMEN_bm | TWI_ENABLE_bm;
    // force bus to idle state
    TWI0_MSTATUS |= TWI_BUSSTATE_IDLE_gc;
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
    uint32_t adc = convert(ADC_REFSEL_2V048_gc, ADC_MUXPOS_AIN30_gc, 255);
    uint16_t mv = (adc * 2048 * 2) >> 12;

    return mv;
}

int main(void) {

    initPins();
    initClock();
    initRTC();
    initWDT();
    initADC();
    if (USART) initUSART();
    initSPI();
    initI2C();
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

    bool radio = rfmInit(FREQUENCY, NODE_ADDR, CAST_ADDR);
    if (radio) {
        rfmSetOutputPower(2);
    } else if (USART) {
        printString("Radio init failed!\r\n");
    }

    static BmeIntf bmeIntf = {.port = &PORTD_OUT, .pin = BME_CS_PD4,
                              .addr = BME_I2C_ADDR_LOW};
    int8_t bme = bmeInit(HEATER_TEMP, HEATER_DUR, AMB_TEMP, &bmeIntf);
    if (USART && bme != 0) {
        printString("BME688 init failed!\r\n");
        printInt(bme);
    }

    bool ens = false;
    if (ENS160) {
        ens = ensInit(ENS_I2C_ADDR_LOW);
        if (USART && !ens) {
            printString("ENS160 init failed!\r\n");
        }
    }

    bool sdc = sdcInit();
    if (USART && !sdc) {
        printString("SD card init failed!\r\n");
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

            if (bavg < BAT_LOW_MV) {
                if (USART) printString("Battery low\r\n");
            } else {
                if (ens) {
                    doEns();
                }
                if (radio && bme == 0 && pas) { // TODO include SD card?
                    doMeas(sdc);

                    // all seems successfully initialized and running
                    wdt_reset();
                }
            }

            // wait for USART tx to be done (before going to sleep)
            if (USART) wait_usart_tx_done();

            // save some power
            sleep_mode();
        }

    }

    return 0;
}
