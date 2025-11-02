/* 
 * File:   utils.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 07. February 2025, 00:25
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/**
 * Computes the length of the given array.
 */
#define array_length(array) (sizeof(array) / sizeof(array[0]))

/**
 * Returns the greater of both given numbers.
 * https://stackoverflow.com/questions/3437404/min-and-max-in-c
 */
#define max(a, b)            \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

/**
 * Returns the smaller of both given numbers.
 * https://stackoverflow.com/questions/3437404/min-and-max-in-c
 */
#define min(a, b)            \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

/**
 * Divides the given numerator by the given denominator,
 * rounds to the nearest int and returns it.
 * http://stackoverflow.com/a/18067292/709426
 */
#define divRoundNearest(num, den) \
({                                \
    ((num < 0) ^ (den < 0)) ?     \
    ((num - den / 2) / den) :     \
    ((num + den / 2) / den);      \
})

/**
 * Converts given hex string to an int.
 * https://stackoverflow.com/a/57112610/709426
 *
 * @param str hex string
 * @return int value
 */
uint16_t xstrtoi(const char *str);

#endif /* UTILS_H */

