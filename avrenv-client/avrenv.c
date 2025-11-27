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
#include "rest.h"

/* Max. expected length of line of data from receiver */
#define LINE_LEN    512

/* REST endpoint to send data from receiver to */
#define SERVER_URL  "http://localhost:8080/data"

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <serial port> <log file>\n", argv[0]);

        return EXIT_SUCCESS;
    }

    int cinit = curl_init();
    if (cinit) {
        return EXIT_FAILURE;
    }

    char *devfile = argv[1];
    char *logfile = argv[2];

    int fd = serial_open(devfile);
    if (fd == -1) {
        curl_cleanup();

        return EXIT_FAILURE;
    }

    FILE *log = fopen(logfile, "a");
    if (log == NULL) {
        curl_cleanup();
        error(EXIT_FAILURE, errno,
              "Log file '%s' could not be opened for writing",
              logfile);
    }

    char buf[LINE_LEN] = {0};
    int len, ret;
    while ((len = serial_read(fd, buf, sizeof (buf))) > 0) {
        printf("%s", buf);
        ret = fprintf(log, "%s", buf);
        if (ret < 0) {
            error(0, errno,
                  "Failed to write to log file '%s'",
                  logfile);
        }
        fflush(log);

        EnvData data = {0};
        int fld = read_data(&data, buf);
        if (fld == FIELD_LEN + 1) {
            printf("T: %u, D: %hhu, RSSI: %hhu, CRC: %hhu, P: %hhu, V: %hu, "
                   "T: %hd, H: %hhu, P: %hu, G: %hu, F: %hhu, S: %hhu, "
                   "L: %u, L: %u, A: %hd, S: %hu\n",
                    data.time, data.dur, data.rssi, data.crc, data.power,
                    data.voltage, data.temperature, data.humidity,
                    data.pressure, data.gasres, data.fix, data.sat, data.lat,
                    data.lon, data.alt, data.speed);

            char json[512];
            snprintf(json, sizeof (json), "{\"time\": \"%u\"}", data.time);
            long code;
            int res = curl_post(SERVER_URL, json, &code);
            if (res == 0) {
                printf("Sent data: HTTP %ld\n", code);
            }
        }
    }

    curl_cleanup();

    ret = fclose(log);
    if (ret != 0) {
        error(EXIT_FAILURE, errno,
              "Log file '%s' could not be closed",
              logfile);
    }

    return EXIT_SUCCESS;
}

