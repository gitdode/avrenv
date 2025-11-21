/*
 * File:   avrenv.c
 * Author: torsten.roemer@luniks.net
 *
 * Client program for avrenv; reads serial data from the receiver
 * and sends it to the web service.
 *
 * Created on 21.11.2025, 18:10
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <error.h>

#include "serial.h"
#include "data.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <serial port>\n", argv[0]);

        return EXIT_SUCCESS;
    }

    char *dev = argv[1];
    int fd = serial_open(dev);
    if (fd == -1) {
        return EXIT_FAILURE;
    }

    char buf[512];
    int len;
    while ((len = serial_read(fd, buf, sizeof (buf))) > 0) {
        printf("%s", buf);

        EnvData data = {0};
        int fld = read_data(&data, buf);
        if (fld != FIELD_LEN) {
            printf("T: %u, D: %hhu, RSSI: %hhu, CRC: %hhu, P: %hhu, V: %hu, "
                   "T: %hd, H: %hhu, P: %hu, G: %hu, F: %hhu, S: %hhu, "
                   "L: %u, L: %u, A: %hd, S: %hu\n",
                    data.time, data.dur, data.rssi, data.crc, data.power,
                    data.voltage, data.temperature, data.humidity,
                    data.pressure, data.gasres, data.fix, data.sat, data.lat,
                    data.lon, data.alt, data.speed);
        }
    }

    return EXIT_SUCCESS;
}

