/*
 * File:   rest.c
 * Author: torsten.roemer@luniks.net
 *
 * libcurl is awesome!
 *
 * Created on 26.11.2025, 00:00
 */

#include "rest.h"

int curl_init(void) {
    static CURLcode res;

    res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != CURLE_OK) {
        fprintf(stderr, "Initializing curl failed: %s\n",
                curl_easy_strerror(res));
        return (int)res;
    }

    return (int)res;
}

void curl_cleanup(void) {
    curl_global_cleanup();
}

/* Cleans up the curl handle when curl_post() returns */
static void easy_cleanup(CURL **curl) {
    curl_easy_cleanup(*curl);
}

/* Frees (all) the slist when curl_post() returns */
static void free_slist(struct curl_slist **chunk) {
    curl_slist_free_all(*chunk);
}

/* Frees the auth header when curl_post() returns */
static void free_auth(char **auth) {
    free(*auth);
}

/**
 * Writer callback providing response data and length.
 *
 * @param data response data owned by libcurl
 * @param size length is size * nmemb
 * @param nmemb length is size * nmemb
 * @param respptr pointer to Response object
 * @return response length
 */
static size_t writer(char *data, size_t size, size_t nmemb, void *respptr) {
    size_t length = size * nmemb + 1;
    Response *resp = (Response *)respptr;
    resp->data = malloc(length);
    if (resp->data) {
        memcpy(resp->data, data, length - 1);
    }
    resp->data[length - 1] = '\0';
    resp->length = length;

    return size * nmemb;
}

int curl_post(const char *url, Request *req, Response *resp) {
    __attribute__ ((cleanup(free_slist))) struct curl_slist *chunk = NULL;
    __attribute__ ((cleanup(easy_cleanup))) CURL *curl;
    __attribute__ ((cleanup(free_auth))) char *auth = NULL;
    static CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        chunk = curl_slist_append(chunk, req->type);
        if (req->token) {
            int len = strlen(BEARER_AUTH_HEADER) + strlen(req->token) + 1;
            auth = malloc(len);
            strlcpy(auth, BEARER_AUTH_HEADER, len);
            strlcat(auth, req->token, len);
            chunk = curl_slist_append(chunk, auth);
        }

        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        if (res != CURLE_OK) {
            fprintf(stderr, "Setting headers failed: %s\n",
                    curl_easy_strerror(res));
            return (int)res;
        }

        res = curl_easy_setopt(curl, CURLOPT_URL, url);
        if (res != CURLE_OK) {
            fprintf(stderr, "Setting URL to '%s' failed: %s\n",
                    url, curl_easy_strerror(res));
            return (int)res;
        }

        res = curl_easy_setopt(curl, CURLOPT_TIMEOUT, POST_TIMEOUT);
        if (res != CURLE_OK) {
            fprintf(stderr, "Setting timeout failed: %s\n",
                    curl_easy_strerror(res));
            return (int)res;
        }

        res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->data);
        if (res != CURLE_OK) {
            fprintf(stderr, "Setting POST data failed: %s\n",
                    curl_easy_strerror(res));
            return (int)res;
        }

        res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
        if (res != CURLE_OK) {
            fprintf(stderr, "Setting writer failed: %s\n",
                    curl_easy_strerror(res));
            return (int)res;
        }

        res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);
        if (res != CURLE_OK) {
            fprintf(stderr, "Setting write data failed: %s\n",
                    curl_easy_strerror(res));
            return (int)res;
        }

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Sending data failed: %s\n",
                    curl_easy_strerror(res));
            return (int)res;
        }

        res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp->code);
        if (res != CURLE_OK) {
            fprintf(stderr, "Getting info failed: %s\n",
                    curl_easy_strerror(res));
            return (int)res;
        }

    }

    return (int)res;
}