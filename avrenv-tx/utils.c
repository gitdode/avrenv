/*
 * File:   utils.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 02.11.2025, 20:11
 */

#include "utils.h"

// https://stackoverflow.com/a/57112610/709426
uint16_t xstrtoi(const char *str) {
    uint16_t res = 0;
    char c;

    while ((c = *str++)) {
        char v = ((c & 0xf) + (c >> 6)) | ((c >> 3) & 0x8);
        res = (res << 4) | (uint16_t) v;
    }

    return res;
}
