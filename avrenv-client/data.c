/*
 * File:   data.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 21.11.2025, 20:24
 */

#include "data.h"

/**
 * Converts the given token with the given index and writes it
 * to the respective member of the structure.
 *
 * @param data structure
 * @param index field index
 * @param token field
 */
static void read_field(EnvData *data, int index, char *token) {
    switch (index) {
        case 0:
            data->time = atol(token);
            break;
        case 1:
            data->dur = atol(token);
            break;
        case 2:
            data->rssi = atol(token);
            break;
        case 3:
            data->crc = atol(token);
            break;
        case 4:
            data->voltage = atol(token);
            break;
        case 5:
            data->power = atol(token);
            break;
        case 6:
            data->temperature = atol(token);
            break;
        case 7:
            data->humidity = atol(token);
            break;
        case 8:
            data->pressure = atol(token);
            break;
        case 9:
            data->gasres = atol(token);
            break;
        case 10:
            data->fix = atol(token);
            break;
        case 11:
            data->sat = atol(token);
            break;
        case 12:
            data->lat = atol(token);
            break;
        case 13:
            data->lon = atol(token);
            break;
        case 14:
            data->alt = atol(token);
            break;
        case 15:
            data->speed = atol(token);
            break;
        default: break;
    }
}

int read_data(EnvData *data, char *line) {
    char *copy = strdup(line);
    char *freeme = copy;
    char *token;
    int i = 0;
    while ((token = strsep(&copy, ",\n"))) {
        read_field(data, i, token);
        i++;
    }

    free(freeme);

    return i;
}
