/*
 * avr64ea.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 * Playground to get into the newer AVRs.
 *
 * Created on: 16.05.2025
 *     Author: torsten.roemer@luniks.net
 *
 */

#include <stdbool.h>
#include <stdlib.h>
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

/* Timebase used for timing internal delays */
#define TIMEBASE_VALUE ((uint8_t) ceil(F_CPU * 0.000001))

/* 0°C in Kelvin */
#define TMP_0C      273.15

/* Specifications of the NTC Thermistor 0.1°C */
#define TH_RESI     100000
#define TH_TEMP     298.15
#define TH_BETA     3892
/* Serial resistance */
#define TH_SERI     100000

#ifndef LORA
    #define LORA    0
#endif

#define USART       1

/* Generates a software event on the given channel */
#define generate_event(channel) EVSYS_SWEVENTA |= (1 << channel)

/* Periodic interrupt timer interrupt count */
static volatile uint32_t pitints = 0;

/* Timer/Counter Type A 0 interrupt count */
static volatile uint32_t tca0ints = 0;

/* Read only data in program memory visible in RAM address space */
const char rostr[] = "This is a string in .rodata in program memory\r\n";

/** Averaged battery voltage in millivolts */
static uint16_t bavg;

/* Periodic interrupt timer interrupt */
ISR(RTC_PIT_vect) {
    // clear flag or interrupt remains active
    RTC_PITINTFLAGS |= RTC_PI_bm;
    pitints++;
}

/* Timer/Counter Type A 0 overflow/underflow interrupt */
ISR(TCA0_OVF_vect) {
    // clear flag or interrupt remains active
    TCA0_SINGLE_INTFLAGS |= TCA_SINGLE_OVF_bm;
    tca0ints++;
}

/* ADC0 result ready interrupt... */
/*
ISR(ADC0_RESRDY_vect) {
    // can use ISR or just check flags
    // flag also cleared when reading result
    ADC0_INTFLAGS |= ADC_RESRDY_bm;
}
 */

/* ...or just empty */
EMPTY_INTERRUPT(ADC0_RESRDY_vect);

/**
 * Radio module DIO0 and DIO4 (FSK)/DIO1 (LoRa) interrupt.
 */
ISR(PORTD_PORT_vect) {
    if (PORTD_INTFLAGS & PORT_INT_2_bm) {
        PORTD_INTFLAGS |= PORT_INT_2_bm;
        // DIO0
        rfmIrq();
    }
    if (PORTD_INTFLAGS & PORT_INT_3_bm) {
        PORTD_INTFLAGS |= PORT_INT_3_bm;
        // DIO4 (FSK)/DIO1 (LoRa)
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

    // set MOSI and SCK as output pin
    PORTA_DIRSET = (1 << MOSI_PA4);
    PORTA_DIRSET = (1 << SCK_PA6);
    // enable input on MISO pin
    PORTA_PIN5CTRL = PORT_ISC_INTDISABLE_gc;
    // enabling pull-up on MISO pin saves quite some power in sleep mode
    // disabling digital input buffer before going to sleep has the same effect
    PORTA_PIN5CTRL |= PORT_PULLUPEN_bm;

    // PC2 powers the thermistor (output pin)
    PORTC_DIRSET = (1 << TH_PWR_PC2);

    // PD0 is radio module reset pin (output pin)
    PORTD_DIRSET = (1 << RFM_RST_PD0);

    // PD1 is radio module CS pin (output pin + pullup)
    PORTD_DIRSET = (1 << RFM_CS_PD1);
    PORTD_PIN1CTRL |= PORT_PULLUPEN_bm;

    // PD4 is BME68x CS pin (output pin + pullup)
    PORTD_DIRSET = (1 << BME_CS_PD4);
    PORTD_PIN2CTRL |= PORT_PULLUPEN_bm;
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
    // set periodic interrupt period in RTC clock cycles, enable PIT
    RTC_PITCTRLA |= RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm;
}

/* Initializes Timer/Counter Type A 0 */
static void initTimerA(void) {
    // set timer period/TOP value
    TCA0_SINGLE_PER = F_CPU / 1024;
    // timer clock select, enable timer
    TCA0_SINGLE_CTRLA |= TCA_SINGLE_CLKSEL_DIV1024_gc | TCA_SINGLE_ENABLE_bm;
    // enable overflow/underflow interrupt
    TCA0_SINGLE_INTCTRL |= (1 << TCA_SINGLE_OVF_bp);
    // keep on running in standby sleep mode
    TCA0_SINGLE_CTRLA |= TCA_SINGLE_RUNSTDBY_bm;
}

/* Initializes Timer/Counter Type B 0 */
static void initTimerB(void) {
    // positive edge on event input, enable timer
    TCB0_CTRLA |= TCB_CLKSEL_EVENT_gc | TCB_ENABLE_bm;
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
    // enable result ready interrupt
    ADC0_INTCTRL |= ADC_RESRDY_bm;
}

/* Initializes the SPI */
static void initSPI(void) {
    // disable client select line, SS pin can be used as regular I/O pin
    SPI0_CTRLB |= SPI_SSD_bm;
    // SPI master mode, SPI enable
    SPI0_CTRLA |= SPI_MASTER_bm | SPI_ENABLE_bm;
}

/* Initializes the Event System */
static void initEVSYS(void) {
    // connect Timer/Counter Type B 0 as event user to CHANNEL0
    EVSYS_USERTCB0COUNT |= EVSYS_CHANNEL_0_bm;
}

/* Initializes pin interrupts */
static void initInts(void) {
    // PD2 sense rising edge
    PORTD_PIN2CTRL = PORT_ISC_RISING_gc;
    // PD3 sense rising edge
    PORTD_PIN3CTRL = PORT_ISC_RISING_gc;
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
 * Measures the temperature and returns it in °C * 10. Powers on the thermistor
 * before AD conversion and again off after, saving about 22µA during sleep mode.
 *
 * @return temperature in °C * 10
 */
static int16_t measureTemp(void) {
    PORTC_OUTSET = (1 << TH_PWR_PC2);
    uint32_t adc = convert(ADC_REFSEL_VDD_gc, ADC_MUXPOS_AIN23_gc, 16);
    PORTC_OUTCLR = (1 << TH_PWR_PC2);

    // resistance of the thermistor
    float resTh = (4096.0 / fmax(1, adc) - 1) * TH_SERI;
    // temperature in °C * 10
    float temp = 10.0 / (1.0 / TH_BETA * log(resTh / TH_RESI) + 1.0 / TH_TEMP) - TMP_0C * 10;

    return temp;
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

int main(void) {

    initPins();
    initClock();
    initRTC();
    // initTimerA();
    initTimerB();
    initADC();
    initUSART();
    initSPI();
    initEVSYS();
    initInts();

    if (USART) {
        printString("Hello AVR64EA!\r\n");
        char rev[16];
        snprintf(rev, sizeof (rev), "Rev. %c%d\r\n",
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

    struct bme68x_dev dev;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heater_conf;
    int8_t bme688 = initBME68x(&dev, &conf, &heater_conf);
    if (bme688 != 0 && USART) {
        printString("BME688 init failed!\r\n");
        printInt(bme688);
    }

    // enable global interrupts
    sei();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    if (USART) printString(rostr);

    while (true) {
        if (pitints % 8 == 0) {
            uint16_t bat = measureBat();
            if (bavg == 0) bavg = bat;
            bavg = (bavg + bat) >> 1;

            if (bavg < 2100) {
                if (USART) printString("Battery low\r\n");
            } else {
                uint16_t temp = measureTemp();
                uint8_t power = rfmGetOutputPower();

                if (USART) {
                    // print temperature measured with thermistor for reference
                    div_t tmp = div(temp, 10);
                    char buf[18];
                    snprintf(buf, sizeof (buf), "%4d.%d°C\r\n", tmp.quot, abs(tmp.rem));
                    printString(buf);
                    // print battery voltage
                    snprintf(buf, sizeof (buf), "%d mV\r\n", bavg);
                    printString(buf);
                }

                if (bme688 == 0) {
                    struct bme68x_data data;
                    bme68xMeasure(&dev, &conf, &heater_conf, &data);

                    div_t tdiv = div(data.temperature, 100);
                    uint8_t humidity = divRoundNearest(data.humidity, 1000);
                    uint16_t pressure = divRoundNearest(data.pressure, 100);
                    uint32_t gas_res = data.gas_resistance / 100; // hOhm

                    if (radio) {
                        uint8_t payload[] = {
                            data.temperature >> 8,
                            data.temperature,
                            humidity,
                            pressure >> 8,
                            pressure,
                            // 18 bit are probably more than sufficient plus
                            // a simple "DC free" mechanism avoiding some
                            // consecutive 0's
                            // data.gas_resistance >> 24,
                            (gas_res >> 16) | 0xa8,
                            gas_res >> 8,
                            gas_res,
                            power,
                            bavg >> 8,
                            bavg
                        };
                        rfmWake();
                        rfmTransmitPayload(payload, sizeof (payload), 0x24);
                        rfmSleep();
                    }

                    if (USART) {
                        // highly sophisticated IAQ algorithm
                        char *aqi = "excellent";
                        if (data.gas_resistance < 45000L) aqi = "good";
                        if (data.gas_resistance < 35000L) aqi = "moderate";
                        if (data.gas_resistance < 25000L) aqi = "poor";
                        if (data.gas_resistance < 15000L) aqi = "unhealthy";

                        // print measurements of the BME688
                        char buf[80];
                        snprintf(buf, sizeof (buf), "%c%d.%d°C, %d%%, %d hPa, %ld Ohm (%s), 0x%02x\r\n",
                                data.temperature < 0 ? '-' : ' ', abs(tdiv.quot), abs(tdiv.rem),
                                humidity,
                                pressure,
                                data.gas_resistance,
                                aqi,
                                data.status);
                        printString(buf);
                    }
                }
            }

            // generate an event on channel 0 incrementing timer B0 count
            generate_event(0);
            if (USART) printInt(TCB0_CNT); // 16-Bit

            // wait for USART tx to be done (before going to sleep)
            wait_usart_tx_done();
        }

        // save some power
        sleep_mode();
    }

    return 0;
}
