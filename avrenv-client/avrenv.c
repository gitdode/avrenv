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

/* Cleans up the Json object when post_data() returns */
static void json_cleanup(json_object **json) {
    int freed = json_object_put(*json);
    if (freed != 1) {
        puts("json_object was not freed, only the refcount decremented");
    }
}

/**
 * Converts given data from receiver to a Json object and POSTs it
 * to the given URL.
 *
 * @param url
 * @param data
 */
static void post_data(const char *url, const char *data) {
    __attribute__ ((cleanup(json_cleanup))) json_object *json = NULL;
    EnvData env = {0};

    int fld = read_data(&env, data);
    // +1 empty field from newline
    if (fld == FIELD_LEN + 1) {
        json = to_json(&env);
        if (json) {
            const char *jsonstr = json_object_to_json_string(json);
            long code;
            int res = curl_post(url, jsonstr, &code);
            if (res == 0) {
                printf("HTTP %ld\n", code);
            }
        }
    }
}

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

        post_data(SERVER_URL, buf);
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

