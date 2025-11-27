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
 * POSTs given data to the given URL and sets given HTTP return code
 * on success.
 * 
 * @param url URL to post data to
 * @param data data to be posted
 * @param code HTTP return code
 * @return 0 on success
 */
int curl_post(const char *url, const char *data, long *code);

#endif /* REST_H */
