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

#define TEST_DATA "26064,4,47,1,3262,2,2359,42,1004,123,1,8,50426084,3273993,22,35\n"

void test_to_json() {
    EnvData env = {0};
    int len = read_data(&env, TEST_DATA);

    assert(len == FIELD_LEN + 1);

    json_object *json = to_json(&env);
    json_object *time = json_object_object_get(json, "time");
    json_object *dur = json_object_object_get(json, "dur");
    json_object *rssi = json_object_object_get(json, "rssi");
    json_object *crc = json_object_object_get(json, "crc");
    json_object *voltage = json_object_object_get(json, "voltage");
    json_object *power = json_object_object_get(json, "power");
    json_object *temperature = json_object_object_get(json, "temperature");
    json_object *humidity = json_object_object_get(json, "humidity");
    json_object *pressure = json_object_object_get(json, "pressure");
    json_object *gasres = json_object_object_get(json, "gasres");
    json_object *fix = json_object_object_get(json, "fix");
    json_object *sat = json_object_object_get(json, "sat");
    json_object *lat = json_object_object_get(json, "lat");
    json_object *lon = json_object_object_get(json, "lon");
    json_object *alt = json_object_object_get(json, "alt");
    json_object *speed = json_object_object_get(json, "speed");

    assert(json_object_get_int(time) == 26064);
    assert(json_object_get_int(dur) == 4);
    assert(json_object_get_int(rssi) == 47);
    assert(json_object_get_int(crc) == 1);
    assert(json_object_get_int(voltage) == 3262);
    assert(json_object_get_int(power) == 2);
    assert(json_object_get_int(temperature) == 2359);
    assert(json_object_get_int(humidity) == 42);
    assert(json_object_get_int(pressure) == 1004);
    assert(json_object_get_int(gasres) == 123);
    assert(json_object_get_int(fix) == 1);
    assert(json_object_get_int(sat) == 8);
    assert(json_object_get_int(lat) == 50426084);
    assert(json_object_get_int(lon) == 3273993);
    assert(json_object_get_int(alt) == 22);
    assert(json_object_get_int(speed) == 35);

    int freed = json_object_put(json);

    assert(freed == 1);
}

void test_read_data() {
    EnvData env = {0};

    int len = read_data(&env, TEST_DATA);

    assert(len == FIELD_LEN + 1);
    assert(env.time == 26064);
    assert(env.dur == 4);
    assert(env.rssi == 47);
    assert(env.crc == 1);
    assert(env.voltage == 3262);
    assert(env.power == 2);
    assert(env.temperature == 2359);
    assert(env.humidity == 42);
    assert(env.pressure == 1004);
    assert(env.gasres == 123);
    assert(env.fix == 1);
    assert(env.sat == 8);
    assert(env.lat == 50426084);
    assert(env.lon == 3273993);
    assert(env.alt == 22);
    assert(env.speed == 35);
}
