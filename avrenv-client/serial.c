/*
 * File:   serial.c
 * Author: torsten.roemer@luniks.net
 *
 * Thanks to the nice https://tldp.org/HOWTO/Serial-Programming-HOWTO/index.html
 *
 * Created on 21.11.2025, 19:42
 */

#include <stdlib.h>

#include "serial.h"

int serial_open(char *dev) {
    int fd = open(dev, O_RDONLY | O_NOCTTY);
    if (fd == -1) {
        error(0, errno,
              "Failed to open serial port '%s'",
              dev);

        return -1;
    }

    struct termios serial = {0};
    serial.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    serial.c_iflag = IGNPAR | ICRNL;
    serial.c_oflag = 0;
    serial.c_lflag = ICANON;

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &serial);

    return fd;
}

int serial_read(int fd, char *buf, size_t len) {
    int i = read(fd, buf, len - 1);
    buf[i] = '\0';

    return i;
}