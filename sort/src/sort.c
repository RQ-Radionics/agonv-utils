/*
 * sort.c - Port de agon-utilities/sort para ESP32-MOS
 *
 * Original: https://github.com/lennart-benschop/agon-utilities (MIT)
 * Port: ESP32-MOS (freestanding, API MOS via mos_libc.h)
 *
 * Uso desde el shell MOS:
 *   sort archivo.txt
 *   sort -f archivo.txt    (case insensitive)
 *   sort -r archivo.txt    (reverse)
 */

#define MOS_LIBC_IMPL
#include "mos_libc.h"

#define MAX_LINES 2000

static int nocase;
static int reverse;

static int my_strcasecmp(const char *p, const char *q)
{
    char c1, c2;
    for (;;) {
        c1 = *p++;
        c2 = *q++;
        if (c1 >= 'a' && c1 <= 'z') c1 = (char)(c1 - 0x20);
        if (c2 >= 'a' && c2 <= 'z') c2 = (char)(c2 - 0x20);
        if (c1 != c2) return c1 - c2;
        if ((c1 | c2) == 0) return 0;
    }
}

static int line_compare(const void *a, const void *b)
{
    const char * const *l1 = (const char * const *)a;
    const char * const *l2 = (const char * const *)b;
    int v;
    if (nocase)
        v = my_strcasecmp(*l1, *l2);
    else
        v = strcmp(*l1, *l2);
    if (reverse) v = -v;
    return v;
}

static char buf[1024];
static char linebuf[256];
static int file_idx;
static int buf_filled;
static FILE *f;

static int nextchar(void)
{
    if (file_idx == buf_filled) {
        buf_filled = (int)fread(buf, 1, 1024, f);
        if (buf_filled == 0) return -1;
        file_idx = 0;
    }
    return (unsigned char)buf[file_idx++];
}

static int nextline(void)
{
    int linelength = 0;
    int c;
    for (;;) {
        c = nextchar();
        if (c == -1) {
            linebuf[linelength] = '\0';
            return linelength != 0;
        } else if (c == '\n' || c == '\r') {
            /* skip trailing \r after \n */
            linebuf[linelength] = '\0';
            return 1;
        } else if (linelength == 255) {
            linebuf[linelength] = '\0';
            return 1;
        } else {
            linebuf[linelength++] = (char)c;
        }
    }
}

static char *lines[MAX_LINES];
static unsigned int nlines = 0;

__attribute__((section(".text.entry")))
int _start(int argc, char **argv, t_mos_api *mos)
{
    mos_libc_init(mos);

    int nopts = 0;
    unsigned int i;

    for (;;) {
        if (argc < nopts + 2) {
            printf("Usage: sort [-f] [-r] <file>\r\n");
            return 1;
        }
        if (strcmp(argv[nopts+1], "-f") == 0) {
            nocase = 1;
            nopts++;
        } else if (strcmp(argv[nopts+1], "-r") == 0) {
            reverse = 1;
            nopts++;
        } else {
            break;
        }
    }

    f = fopen(argv[1+nopts], "r");
    if (!f) {
        printf("Cannot open file %s\r\n", argv[1+nopts]);
        return 1;
    }

    file_idx = 0;
    buf_filled = 0;
    nlines = 0;

    while (nlines < MAX_LINES && nextline()) {
        lines[nlines] = strdup(linebuf);
        if (!lines[nlines]) break;
        nlines++;
    }
    fclose(f);

    qsort(lines, nlines, sizeof(char *), line_compare);

    for (i = 0; i < nlines; i++) {
        printf("%s\r\n", lines[i]);
        free(lines[i]);
    }

    return 0;
}
