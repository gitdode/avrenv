/* 
 * File:   serial.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 21.11.2025, 19:42
 */

#ifndef SERIAL_H
#define SERIAL_H

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <error.h>
#include <fcntl.h>
#include <termios.h>

/* Baudrate used by the receiver */
#define BAUDRATE B9600

/**
 * Opens the serial port with the given device file (i.e. /dev/ttyUSB0) 
 * read only and returns the file descriptor or -1 if an error occured.
 * Currently uses fixed settings, like baud rate etc.
 * 
 * @param dev serial port device path
 * @return file descriptor or -1 on error
 */
int serial_open(char *dev);

/**
 * Reads up to len - 1 characters from the given file descriptor 
 * into the given buffer until a line end and adds a '\0' terminator.
 * 
 * @param fd file descriptor
 * @param buf buffer
 * @param len length of given buffer
 * @return characters read
 */
int serial_read(int fd, char *buf, size_t len);

#endif /* SERIAL_H */

