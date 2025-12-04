/*
 * File:   rest.c
 * Author: torsten.roemer@luniks.net
 *
 * libcurl is awesome!
 * json-c is also awesome!
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
    auth = NULL;
}

/* Cleans up the Json object when post_data() returns */
static void json_cleanup(json_object **json) {
    int freed = json_object_put(*json);
    if (freed != 1) {
        puts("json_object was not freed, only the refcount decremented");
    }
}

/**
 * Writer callback providing response data and length.
 *
 * This function is called multiple times for large responses!
 * Thanks to https://everything.curl.dev/examples/getinmem.html
 *
 * @param data response data owned by libcurl
 * @param size length is size * nmemb
 * @param nmemb length is size * nmemb
 * @param respptr pointer to Response object
 * @return response length
 */
static size_t writer(char *data, size_t size, size_t nmemb, void *respptr) {
    size_t length = size * nmemb;
    Response *resp = (Response *) respptr;

    resp->data = realloc(resp->data, resp->length + length + 1);
    if (resp->data == NULL) {
        error(0, ENOMEM, "Insufficient memory to read response data");
        return 0;
    }

    memcpy(&(resp->data[resp->length]), data, length);
    resp->length += length;
    resp->data[resp->length] = 0;

    return length;
}

int curl_post(const char *url, Request *req, Response *resp) {
    __attribute__ ((cleanup(free_slist))) struct curl_slist *chunk = NULL;
    __attribute__ ((cleanup(easy_cleanup))) CURL *curl;
    __attribute__ ((cleanup(free_auth))) char *auth = NULL;
    static CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        chunk = curl_slist_append(chunk, req->type);
        if (req->access) {
            int len = strlen(HEADER_BEARER_AUTH) + strlen(req->access) + 1;
            auth = malloc(len);
            strlcpy(auth, HEADER_BEARER_AUTH, len);
            strlcat(auth, req->access, len);
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

        curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
        if (res != CURLE_OK) {
            fprintf(stderr, "Caching CA cert bundle failed: %s\n",
                    curl_easy_strerror(res));
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

int get_token(char *username, char *password, Token *token) {
    char form[256];
    snprintf(form, sizeof (form), TOKEN_REQ, username, password);
    Request req = {.data = form, .type = CONTENT_TYPE_FURL,
                   .access = NULL};
    Response resp = {.code = 0, .data = NULL, .length = 0};
    int res = curl_post(TOKEN_URL, &req, &resp);
    if (res == 0) {
        if (resp.code == 200) {
            __attribute__ ((cleanup(json_cleanup))) json_object *json;
            json = json_tokener_parse(resp.data);
            json_object *jaccess = json_object_object_get(json, "access_token");
            const char *iaccess = json_object_get_string(jaccess);
            size_t len = strlen(iaccess) + 1;
            token->access = realloc((void *)token->access, len);
            strlcpy(token->access, iaccess, len);
            json_object *jexpires = json_object_object_get(json, "expires_in");
            time_t now = time(NULL);
            token->exp = (time_t)json_object_get_uint64(jexpires) + now;
        } else {
            puts(resp.data);
        }
    }
    free(resp.data);
    resp.data = NULL;

    return (int)resp.code;
}

int post_data(const char *url, const char *token, EnvData *env) {
    __attribute__ ((cleanup(json_cleanup))) json_object *json;
    json = to_json(env);
    int code = 0;
    if (json) {
        const char *jsonstr = json_object_to_json_string(json);
        Request req = {.data = jsonstr, .type = CONTENT_TYPE_JSON,
                       .access = token};
        Response resp = {.code = 0, .data = NULL, .length = 0};
        int res = curl_post(url, &req, &resp);
        if (res == 0) {
            code = resp.code;
        }
        free(resp.data);
        resp.data = NULL;
    }

    return code;
}
