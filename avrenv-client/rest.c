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

int curl_post(const char *url, const char *data, long *code) {
    __attribute__ ((cleanup(free_slist))) struct curl_slist *chunk = NULL;
    __attribute__ ((cleanup(easy_cleanup))) CURL *curl;
    static CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        chunk = curl_slist_append(chunk, CONTENT_TYPE_JSON);

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

        res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        if (res != CURLE_OK) {
            fprintf(stderr, "Setting POST data failed: %s\n",
                    curl_easy_strerror(res));
            return (int)res;
        }

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Sending data failed: %s\n",
                    curl_easy_strerror(res));
            return (int)res;
        }

        res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, code);
        if (res != CURLE_OK) {
            fprintf(stderr, "Getting info failed: %s\n",
                    curl_easy_strerror(res));
            return (int)res;
        }
    }

    return (int)res;
}