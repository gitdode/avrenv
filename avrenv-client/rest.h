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
#include <time.h>
#include <errno.h>
#include <error.h>

#include <curl/curl.h>

#include "data.h"

/* Timeout for POSTing data in seconds */
#define POST_TIMEOUT        10UL

/* Content-Type: application/json */
#define CONTENT_TYPE_JSON   "Content-Type: application/json"

/* Content-Type: application/x-www-form-urlencoded */
#define CONTENT_TYPE_FURL   "Content-Type: application/x-www-form-urlencoded"

/* Authorization header */
#define HEADER_BEARER_AUTH   "Authorization: Bearer "

/* Token endpoint of the auth server */
#define TOKEN_URL   "http://keycloak.luniks.net/realms/luniks/protocol/openid-connect/token"

/* Format for direct access grant login request form data */
#define TOKEN_REQ   "grant_type=password&client_id=public&username=%s&password=%s"

/* REST endpoint to send data from receiver to */
#define SERVER_URL  "http://baloon.luniks.net/data"

/* Request object */
typedef struct {
    /* Data to be POSTed */
    const char *data;
    /* Content type */
    const char *type;
    /* Access token */
    const char *access;
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

/* Token with expiration time */
typedef struct {
    /* Access token */
    char *access;
    /* Expiration time */
    time_t exp;
} Token;

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
 * Response.data must be freed after use.
 * 
 * @param url URL to post request to
 * @param req request
 * @param resp response
 * @return 0 on success
 */
int curl_post(const char *url, Request *req, Response *resp);

/**
 * Gets an access token from the auth server and updates the given
 * token on success.
 * 
 * Token.access should be freed after final use.
 *
 * @param username
 * @param password
 * @return HTTP response code
 */
int get_token(char *username, char *password, Token *token);

/**
 * Converts given data from receiver to a Json object and POSTs it
 * to the given URL.
 *
 * @param url server url
 * @param sessionid login session id
 * @param env EnvData
 */
void post_data(const char *url, const char *sessionid, EnvData *env);

#endif /* REST_H */
