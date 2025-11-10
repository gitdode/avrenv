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

#define BLOCKSIZE   512

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
    uint32_t alt;
    /* Speed over ground in knots * 100 */
    uint16_t speed;
} SatData;

/*
 * Converts a raw file written by avrenv transmitter to a GPX file.
 */
int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s <avrenv raw file> <gpx file>", argv[0]);

        return EXIT_SUCCESS;
    }

    FILE *envf = fopen(argv[1], "r");
    FILE *gpxf = fopen(argv[2], "w+");

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
        while ((token = strsep(&str, ","))) {
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

        div_t lat_div = div(data.lat * 10, 10000000);
        div_t lon_div = div(data.lon * 10, 10000000);

        struct tm tm = *localtime(&(time_t){time(NULL)});
        strptime(data.utc, "%H%M%S", &tm);
        char tbuf[24];
        strftime(tbuf, sizeof (tbuf), "%Y-%m-%dT%H:%M:%SZ", &tm);

        if (data.fix != 0) {
            fprintf(gpxf, "      <trkpt lat=\"%u.%u\" lon=\"%u.%u\">\n",
                    lat_div.quot, (lat_div.rem + 30) / 60,
                    lon_div.quot, (lon_div.rem + 30) / 60);
            fprintf(gpxf, "        <ele>%u</ele>\n", (data.alt + 5) / 10);
            fprintf(gpxf, "        <time>%s</time>\n", tbuf);
            fprintf(gpxf, "      </trkpt>\n");
        }
    }

    fputs("    </trkseg>\n", gpxf);
    fputs("  </trk>\n", gpxf);
    fputs("</gpx>\n", gpxf);

    fclose(envf);
    fflush(gpxf);
    fclose(gpxf);

    return (EXIT_SUCCESS);
}

