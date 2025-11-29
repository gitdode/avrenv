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

/* Token endpoint of the auth server */
#define TOKEN_URL   "http://localhost:8090/realms/luniks/protocol/openid-connect/token"

/* Format for direct access grant login request form data */
#define TOKEN_REQ   "grant_type=password&client_id=public&username=%s&password=%s"

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
 * Gets and returns a token from the auth server or NULL if that failed.
 *
 * It is the caller's responsibility to free the returned token from memory.
 *
 * @param username
 * @param password
 * @return access token
 */
static char *get_token(char *username, char *password) {
    char *token = NULL;

    char form[256];
    snprintf(form, sizeof (form), TOKEN_REQ, username, password);
    Request req = {.data = form, .type = CONTENT_TYPE_FURL,
                   .token = NULL};
    Response resp = {.code = 0, .data = NULL, .length = 0};
    int res = curl_post(TOKEN_URL, &req, &resp);
    if (res == 0) {
        if (resp.code == 200) {
            __attribute__ ((cleanup(json_cleanup))) json_object *json;
            json = json_tokener_parse(resp.data);
            json_object *jtoken = json_object_object_get(json, "access_token");
            const char *itoken = json_object_get_string(jtoken);
            token = strdup(itoken);
        } else {
            puts(resp.data);
        }
    }
    free(resp.data);

    return token;
}

/**
 * Converts given data from receiver to a Json object and POSTs it
 * to the given URL.
 *
 * @param url server url
 * @param token bearer token
 * @param data Json data
 */
static void post_data(const char *url, const char *token, const char *data) {
    EnvData env = {0};

    // -1 empty field from (ignored) newline
    int fld = read_data(&env, data) - 1;
    if (fld == FIELD_LEN) {
        __attribute__ ((cleanup(json_cleanup))) json_object *json;
        json = to_json(&env);
        if (json) {
            const char *jsonstr = json_object_to_json_string(json);
            Request req = {.data = jsonstr, .type = CONTENT_TYPE_JSON,
                           .token = token};
            Response resp = {.code = 0, .data = NULL, .length = 0};
            int res = curl_post(url, &req, &resp);
            if (res == 0) {
                printf("HTTP %ld\n", resp.code);
                if (resp.data) puts(resp.data);
            }
            free(resp.data);
        }
    } else {
        printf("Unexpected number of data fields: %d (%d)\n",
                fld, FIELD_LEN);
    }
}

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

        char *token = get_token(username, password);
        if (token) {
            puts(token);
            post_data(SERVER_URL, token, buf);
        }
        free(token);
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

