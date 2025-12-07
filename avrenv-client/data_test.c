/*
 * File:   data_test.c
 * Author: torsten.roemer@luniks.net
 *
 * AceUnit is awesome!
 *
 * Created on 07.12.2025, 0:18
 */

#include <assert.h>
#include <stddef.h>

#include "data.h"

void test_read_data() {
    char *line = "26064,4,47,1,3262,2,2359,42,1004,123,1,8,50426084,3273993,22,35\n";
    EnvData data = {0};

    int len = read_data(&data, line);

    assert(len == FIELD_LEN + 1);
    assert(data.time == 26064);
    assert(data.dur == 4);
    assert(data.rssi == 47);
    assert(data.crc == 1);
    assert(data.voltage == 3262);
    assert(data.power == 2);
    assert(data.temperature == 2359);
    assert(data.humidity == 42);
    assert(data.pressure == 1004);
    assert(data.gasres == 123);
    assert(data.fix == 1);
    assert(data.sat == 8);
    assert(data.lat == 50426084);
    assert(data.lon == 3273993);
    assert(data.alt == 22);
    assert(data.speed == 35);
}
