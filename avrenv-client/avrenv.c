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

int main(int argc, char **argv) {
    if (argc != 5) {
        printf("Usage: %s <serial port> <log file> <username> <password>\n",
               argv[0]);

        return EXIT_SUCCESS;
    }

    int cinit = curl_init();
    if (cinit) {
        return EXIT_FAILURE;
    }

    char *devfile = argv[1];
    char *logfile = argv[2];
    char *username = argv[3];
    char *password = argv[4];

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

    Token token = {.access = malloc(0), .exp = (time_t){time(NULL)}};
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

        time_t now = time(NULL);
        printf("Token expires in: %ld s\n", token.exp - now);
        if (token.exp - 30 < now) {
            token = *get_token(username, password, &token);
        }
        post_data(SERVER_URL, token.access, buf);
    }

    free((void *)token.access);
    token.access = NULL;
    curl_cleanup();

    ret = fclose(log);
    if (ret != 0) {
        error(EXIT_FAILURE, errno,
                "Log file '%s' could not be closed",
                logfile);
    }

    return EXIT_SUCCESS;
}

