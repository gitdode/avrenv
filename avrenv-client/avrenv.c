/*
 * File:   avrenv.c
 * Author: torsten.roemer@luniks.net
 *
 * Client program for avrenv; reads serial data from the receiver,
 * writes it to a log file and sends it to the web service.
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
#include <signal.h>

#include "serial.h"
#include "data.h"
#include "rest.h"

/* Max. expected length of line of data from receiver */
#define LINE_LEN    512

/* Log file stream */
static FILE *log;

/* Auth token */
static Token token;

/* Indicates a SIGINT */
static bool sigint = false;

/**
 * Initiates an orderly exit incl. full cleanup.
 *
 * @param signo signal number
 */
static void cleanup(int signo) {
    sigint = true;
    puts("Gracefully stopping, please wait...");

    return;
}

int main(int argc, char **argv) {
    if (argc != 5) {
        printf("Usage: %s <serial port> <log file> <username> <password>\n",
                argv[0]);

        return EXIT_SUCCESS;
    }

    struct sigaction sig_handler = {.sa_handler = cleanup};
    sigaction(SIGINT, &sig_handler, NULL);

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

    log = fopen(logfile, "a");
    if (log == NULL) {
        curl_cleanup();
        error(EXIT_FAILURE, errno,
                "Log file '%s' could not be opened for writing",
                logfile);
    }

    EnvData env = {0};
    char data[LINE_LEN] = {0};
    int len, ret, code = 0;
    while ((len = serial_read(fd, data, sizeof (data))) > 0) {
        if (sigint) break;

        // -1 empty field from (ignored) newline
        int fld = read_data(&env, data) - 1;
        if (fld == FIELD_LEN) {
            printf("%s", data);
            ret = fprintf(log, "%s", data);
            if (ret < 0) {
                error(0, errno,
                        "Failed to write to log file '%s'",
                        logfile);
            }
            fflush(log);

            time_t now = time(NULL);
            printf("Token expires in %ld s\n", token.exp - now);
            if (token.exp - 30 < now) {
                code = get_token(username, password, &token);
                printf("Get token: HTTP %d\n", code);
            }
            if (code == 200) {
                code = post_data(SERVER_URL, token.access, &env);
                printf("Send data: HTTP %d\n", code);
            }
        } else {
            printf("Unexpected number of data fields: %d (%d)\n",
                    fld, FIELD_LEN);
        }
    }

    curl_cleanup();
    free((void *) token.access);
    ret = fclose(log);
    if (ret != 0) {
        error(EXIT_FAILURE, errno,
                "Log file '%s' could not be closed",
                logfile);
    }

    return EXIT_SUCCESS;
}

