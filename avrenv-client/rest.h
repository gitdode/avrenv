/* 
 * File:   rest.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 26.11.2025, 00:00
 */

#ifndef REST_H
#define REST_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <curl/curl.h>

/* Timeout for POSTing data in seconds */
#define POST_TIMEOUT        10UL

/* Content-Type: application/json */
#define CONTENT_TYPE_JSON   "Content-Type: application/json"

/* Content-Type: application/x-www-form-urlencoded */
#define CONTENT_TYPE_FURL   "Content-Type: application/x-www-form-urlencoded"

/* Authorization header */
#define BEARER_AUTH_HEADER  "Authorization: Bearer "

/* Request object */
typedef struct {
    /* Data to be POSTed */
    const char *data;
    /* Content type */
    const char *type;
    /* Bearer token */
    const char *token;
} Request;

/* Response object */
typedef struct {
    /* HTTP status code */
    long code;
    /* Response data */
    char *data;
    /* Response data length */
    size_t length;
} Response;

/**
 * Initializes libcurl globally.
 * 
 * @return 0 on success
 */
int curl_init(void);

/**
 * Cleans up libcurl globally.
 */
void curl_cleanup(void);

/**
 * POSTs the given request to the given URL and sets the response.
 * 
 * @param url URL to post request to
 * @param req request
 * @param resp response
 * @return 0 on success
 */
int curl_post(const char *url, Request *req, Response *resp);

#endif /* REST_H */
