/*
 * usart.h
 *
 *  Created on: 16.05.2025
 *      Author: torsten.roemer@luniks.net
 */

#ifndef USART_H_
#define USART_H_

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "pins.h"
#include "utils.h"

#define USART_LENGTH 128

#ifndef BAUDRATE
#define BAUDRATE 9600
#endif

/* Asynchronous Normal mode: S = 16 */
#define UBRR ((64 * F_CPU) / (16UL * BAUDRATE))

/* Waits until transmit complete */
#define wait_usart_tx_done() loop_until_bit_is_set(USART1_STATUS, USART_TXCIF_bp)

/**
 * Sets the baudrate and enables the transmitter and receiver.
 */
void initUSART(void);

/**
 * Returns true if a CR or LF terminated line of data was received via USART.
 * 
 * @return data was received
 */
bool isUSARTReceived(void);

/**
 * Appends the data received via USART to the given string with the given
 * length.
 */
void getUSARTData(char *data, size_t length);

/**
 * Prints the given string via USART.
 */
void printString(const char *data);

/**
 * Prints the given integer including CR + LF via USART.
 */
void printInt(int16_t data);

/**
 * Prints the given unsigned integer in hex notation including CR + LF 
 * via USART.
 */
void printHex(uint16_t data);

/**
 * Prints the given unsigned integer in binary notation including CR + LF
 * via USART.
 */
void printByte(uint8_t data);

#endif /* USART_H_ */
