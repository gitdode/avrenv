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

#define LINE_BUF    512

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <serial port> <log file>\n", argv[0]);

        return EXIT_SUCCESS;
    }

    char *devfile = argv[1];
    char *logfile = argv[2];

    int fd = serial_open(devfile);
    if (fd == -1) {
        return EXIT_FAILURE;
    }

    FILE *log = fopen(logfile, "a");
    if (log == NULL) {
        error(EXIT_FAILURE, errno,
              "Error: log file '%s' could not be opened for writing",
              logfile);
    }

    char buf[LINE_BUF];
    int len, ret;
    while ((len = serial_read(fd, buf, sizeof (buf))) > 0) {
        printf("%s", buf);
        ret = fprintf(log, "%s", buf);
        if (ret < 0) {
            error(0, errno,
                  "Error: failed to write to log file '%s'",
                  logfile);
        }
        fflush(log);

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

    ret = fclose(log);
    if (ret != 0) {
        error(EXIT_FAILURE, errno,
              "Error: log file '%s' could not be closed",
              logfile);
    }

    return EXIT_SUCCESS;
}

