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

json_object* to_json(EnvData *data) {
    static json_object *root;
    root = json_object_new_object();
    if (!root) {
        return NULL;
    }

    json_object_object_add(root, "time",
            json_object_new_int(data->time));
    json_object_object_add(root, "dur",
            json_object_new_int(data->dur));
    json_object_object_add(root, "rssi",
            json_object_new_int(data->rssi));
    json_object_object_add(root, "crc",
            json_object_new_int(data->crc));
    json_object_object_add(root, "voltage",
            json_object_new_int(data->voltage));
    json_object_object_add(root, "power",
            json_object_new_int(data->power));
    json_object_object_add(root, "temperature",
            json_object_new_uint64(data->temperature));
    json_object_object_add(root, "humidity",
            json_object_new_int(data->humidity));
    json_object_object_add(root, "pressure",
            json_object_new_int(data->pressure));
    json_object_object_add(root, "gasres",
            json_object_new_int(data->gasres));
    json_object_object_add(root, "fix",
            json_object_new_int(data->fix));
    json_object_object_add(root, "sat",
            json_object_new_int(data->sat));
    json_object_object_add(root, "lat",
            json_object_new_int(data->lat));
    json_object_object_add(root, "lon",
            json_object_new_int(data->lon));
    json_object_object_add(root, "alt",
            json_object_new_uint64(data->alt));
    json_object_object_add(root, "speed",
            json_object_new_int(data->speed));

    return root;
}

int read_data(EnvData *data, const char *line) {
    char *copy = strdup(line);
    if (copy == NULL) {
        error(EXIT_FAILURE, errno,
                "Insufficient memory to copy line of data");
    }
    
    char *freeme = copy;
    char *token;
    int i = 0;
    while ((token = strsep(&copy, ",\n"))) {
        read_field(data, i, token);
        i++;
    }

    free(freeme);
    freeme = NULL;

    return i;
}
