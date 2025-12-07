/*
 * File:   rest_test.c
 * Author: torsten.roemer@luniks.net
 *
 * AceUnit is awesome!
 *
 * Created on 07.12.2025, 10:32
 */

#include <assert.h>
#include <stddef.h>

#include "rest.h"

#define TEST_TOKEN_URL  "http://localhost:8090/realms/quarkus/protocol/openid-connect/token"
#define TEST_SERVER_URL "http://localhost:8080/data"
#define TEST_SECRET     "secret"
#define TEST_USERNAME   "envtest"
#define TEST_PASSWORD   "password"

void test_get_token_unauth() {
    Token token = {0};

    int code = get_token(TEST_TOKEN_URL, TEST_SECRET,
            "invalid", "credentials", &token);

    assert(code == 401);
    assert(token.exp == 0);
    assert(token.access == NULL);

    free(token.access);
}

void test_get_token() {
    Token token = {0};

    int code = get_token(TEST_TOKEN_URL, TEST_SECRET,
            TEST_USERNAME, TEST_PASSWORD, &token);

    assert(code == 200);
    assert(token.exp >= (time_t){time(NULL)});
    assert(strncmp(token.access, "ey", 2) == 0);

    free(token.access);
}

void test_post_data_unauth() {
    int code = 0;
    EnvData data = {0};

    code = post_data(TEST_SERVER_URL, "invalid token", &data);

    assert(code == 401);
}

void test_post_data() {
    Token token = {0};
    int code = 0;
    EnvData data = {0};

    code = get_token(TEST_TOKEN_URL, TEST_SECRET,
            TEST_USERNAME, TEST_PASSWORD, &token);

    assert(code == 200);

    code = post_data(TEST_SERVER_URL, token.access, &data);

    assert(code == 200);

    free(token.access);
}
