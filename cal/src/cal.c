/*
 * cal.c - Port de agon-utilities/cal para ESP32-MOS
 *
 * Original: https://github.com/lennart-benschop/agon-utilities (MIT)
 * Port: ESP32-MOS (freestanding, API MOS via mos_libc.h)
 *
 * Uso desde el shell MOS:
 *   cal 2025
 *   cal 3 2025
 *   cal -s 2025
 */

#define MOS_LIBC_IMPL
#include "mos_libc.h"

static int start_sunday;

static const char *month_names[] = {
    "January","February","March","April","May","June",
    "July","August","September","October","November","December"
};

static const char days[] =
    "     1  2  3  4  5  6  7  8  9"
    " 10 11 12 13 14 15 16 17 18 19"
    " 20 21 22 23 24 25 26 27 28 29"
    " 30 31";

static const char daynames1[] = " Mo Tu We Th Fr Sa Su";
static const char daynames2[] = " Su Mo Tu We Th Fr Sa";

static unsigned int month_lengths[] = {31,28,31,30,31,30,31,31,30,31,30,31};

static char month_buf1[6*22];
static char month_buf2[6*22];
static char month_buf3[6*22];

static void memset_local(char *p, int c, size_t n)
{
    while (n--) *p++ = (char)c;
}

static void memcpy_local(char *dst, const char *src, size_t n)
{
    while (n--) *dst++ = *src++;
}

static void fill_month(char *buf, unsigned int startday, unsigned int month_len)
{
    unsigned int i;
    char *p;
    for (i = 0; i < 6; i++)
        memset_local(buf + i*22, 32, 21);
    p = buf + startday*3;
    for (i = 0; i < month_len*3; i += 3) {
        memcpy_local(p, days + i + 3, 3);
        p += 3;
        if (*p == 0) p++;
    }
}

static unsigned int month_length(unsigned int m, unsigned int year)
{
    unsigned int ml = month_lengths[m-1];
    if (m == 2) {
        if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
            ml++;
    }
    return ml;
}

__attribute__((section(".text.entry")))
int _start(int argc, char **argv, t_mos_api *mos)
{
    mos_libc_init(mos);

    int nopts = 0;
    unsigned int i, j, y;
    unsigned int year, month;

    for (;;) {
        if (argc + nopts < 2) {
            printf("Usage: cal [-s] [month] <year>\r\n");
            return 1;
        }
        if (strcmp(argv[1+nopts], "-s") == 0) {
            start_sunday = 1;
            nopts++;
        } else {
            break;
        }
    }

    if (argc == 2 + nopts) {
        month = 0;
        year = (unsigned int)atoi(argv[nopts+1]);
    } else {
        month = (unsigned int)atoi(argv[nopts+1]);
        if (month < 1 || month > 12) {
            printf("Invalid month\r\n");
            return 1;
        }
        year = (unsigned int)atoi(argv[nopts+2]);
    }

    y = year - 1;
    y = y + y/4 - y/100 + y/400 + (unsigned int)start_sunday;

    if (month == 0) {
        printf("%36u\r\n", year);
        for (i = 0; i < 12; i += 3) {
            unsigned int k1 = strlen(month_names[i]);
            unsigned int k2 = strlen(month_names[i+1]);
            unsigned int k3 = strlen(month_names[i+2]);
            /* centrar nombres de mes en campo de 21 caracteres */
            unsigned int pad1 = 11 + k1/2;
            unsigned int pad2 = 21 - k1/2 + k2/2;
            unsigned int pad3 = 21 - k2/2 + k3/2;
            for (j = 0; j < pad1; j++) printf(" ");
            printf("%s", month_names[i]);
            for (j = 0; j < pad2; j++) printf(" ");
            printf("%s", month_names[i+1]);
            for (j = 0; j < pad3; j++) printf(" ");
            printf("%s\r\n", month_names[i+2]);

            printf("%s  %s  %s\r\n",
                start_sunday ? daynames2 : daynames1,
                start_sunday ? daynames2 : daynames1,
                start_sunday ? daynames2 : daynames1);

            fill_month(month_buf1, y%7, month_length(i+1, year));
            y += month_length(i+1, year);
            fill_month(month_buf2, y%7, month_length(i+2, year));
            y += month_length(i+2, year);
            fill_month(month_buf3, y%7, month_length(i+3, year));
            y += month_length(i+3, year);

            for (j = 0; j < 6; j++) {
                /* month_buf lines son 21 chars sin null — imprimir byte a byte */
                int k;
                for (k = 0; k < 21; k++) printf("%c", month_buf1[22*j+k]);
                printf("  ");
                for (k = 0; k < 21; k++) printf("%c", month_buf2[22*j+k]);
                printf("  ");
                for (k = 0; k < 21; k++) printf("%c", month_buf3[22*j+k]);
                printf("\r\n");
            }
            if (i < 9) printf("\r\n");
        }
    } else {
        unsigned int pad = 9 + strlen(month_names[month-1])/2;
        for (j = 0; j < pad; j++) printf(" ");
        printf("%s %u\r\n", month_names[month-1], year);
        printf("%s\r\n", start_sunday ? daynames2 : daynames1);
        for (j = 1; j < month; j++) y += month_length(j, year);
        fill_month(month_buf1, y%7, month_length(month, year));
        for (j = 0; j < 6; j++) {
            int k;
            for (k = 0; k < 21; k++) printf("%c", month_buf1[22*j+k]);
            printf("\r\n");
        }
    }

    return 0;
}
