/*
 * File:   txt2gpx.c
 * Author: torsten.roemer@luniks.net
 *
 * Created on 09.11.2025, 00:02
 */

#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <error.h>

#define BLOCKSIZE   512
#define FIELD_LEN   15

/* Data read off the GPS module */
typedef struct {
    /* Timestamp in UTC: hhmmss */
    char *utc;
    /* Latitude in degrees minutes (WGS84) x 10000 */
    uint32_t lat;
    /* Longitude in degrees minutes (WGS84) x 10000 */
    uint32_t lon;
    /* Fix: 0 = not available, 1 = GPS, 2 = differential GPS */
    uint8_t fix;
    /* Number of satellites used */
    uint8_t sat;
    /* Altitude in meters x 10 */
    int32_t alt;
    /* Speed over ground in knots * 100 */
    uint16_t speed;
} SatData;

/**
 * Reads blocks of 512 bytes from 'envf', being a raw image of an SD card of
 * n blocks length, while each block is expected to be a a null-terminated
 * string of comma-separated fields. The fields are converted and used to write
 * basic <trkpt> elements to the GPX file.
 * The WGS84 coordinates are converted from degrees minutes to decimal degrees.
 *
 * @param envf input file stream
 * @param gpxf output file stream
 */
static void convert(FILE *envf, FILE *gpxf) {
    fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n", gpxf);
    fputs("<gpx version=\"1.1\" creator=\"avrenv\">\n", gpxf);
    fputs("  <trk>\n", gpxf);
    fputs("    <trkseg>\n", gpxf);

    uint8_t block[BLOCKSIZE];
    while (fread(block, sizeof (block), 1, envf)) {
        char line[BLOCKSIZE];
        snprintf(line, sizeof (line), "%s", block);

        SatData data = {0};
        uint8_t i = 0;
        char *token;
        char *str = line;
        while ((token = strsep(&str, ",\n"))) {
            switch (i) {
                case 7: data.utc = token;
                    break;
                case 8: data.fix = atol(token);
                    break;
                case 9: data.sat = atol(token);
                    break;
                case 10: data.lat = atol(token);
                    break;
                case 11: data.lon = atol(token);
                    break;
                case 12: data.alt = atol(token);
                    break;
                case 13: data.speed = atol(token);
                    break;
                default: break;
            }
            i++;
        };

        if (i == FIELD_LEN && data.fix != 0) {
            div_t lat_div = div(data.lat, 1000000);
            div_t lon_div = div(data.lon, 1000000);

            struct tm tm = *localtime(&(time_t){time(NULL)});
            strptime(data.utc, "%H%M%S", &tm);
            char tbuf[24];
            strftime(tbuf, sizeof (tbuf), "%Y-%m-%dT%H:%M:%SZ", &tm);

            fprintf(gpxf, "      <trkpt lat=\"%u.%05u\" lon=\"%u.%05u\">\n",
                    lat_div.quot, lat_div.rem / 6,
                    lon_div.quot, lon_div.rem / 6);
            fprintf(gpxf, "        <ele>%d</ele>\n", (data.alt + 5) / 10);
            fprintf(gpxf, "        <time>%s</time>\n", tbuf);
            fprintf(gpxf, "      </trkpt>\n");
        }
    }

    fputs("    </trkseg>\n", gpxf);
    fputs("  </trk>\n", gpxf);
    fputs("</gpx>\n", gpxf);
}

/*
 * Converts a raw file written by avrenv transmitter to a GPX file.
 */
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <avrenv raw file> <gpx file>\n", argv[0]);

        return EXIT_SUCCESS;
    }

    const char *envfile = argv[1];
    const char *gpxfile = argv[2];

    FILE *envf = fopen(envfile, "r");
    if (envf == NULL) {
        error(EXIT_FAILURE, errno,
              "Error: input file '%s' could not be opened for reading",
              envfile);
    }
    FILE *gpxf = fopen(gpxfile, "w+");
    if (gpxf == NULL) {
        error(EXIT_FAILURE, errno,
              "Error: output file '%s' could not be opened for writing",
              gpxfile);
    }

    convert(envf, gpxf);

    if (fclose(envf) != 0) {
        error(EXIT_FAILURE, errno,
              "Error: input file '%s' could not be closed",
              envfile);
    }
    if (fflush(gpxf) != 0 || fclose(gpxf) != 0) {
        error(EXIT_FAILURE, errno,
              "Error: output file '%s' could not be closed",
              gpxfile);
    }

    return EXIT_SUCCESS;
}

