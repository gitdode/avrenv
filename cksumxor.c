/*
 * cksumxor.c
 *
 * Calculates the checksum of the given string by XORing all characters.
 *
 * Created on: 26.10.2025
 *     Author: torsten.roemer@luniks.net
 *
 */

#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <string>\r\n", argv[0]);

        return 0;
    }

    uint16_t cksum = 0;
    char *string = argv[1];
    for (; *string != '\0'; string++) {
        cksum ^= *string;
    }

    printf("%X\r\n", cksum);

    return 0;
}
