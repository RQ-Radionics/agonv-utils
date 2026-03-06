/*
 * concat.c - Port de agon-utilities/concat para ESP32-MOS
 *
 * Original: https://github.com/lennart-benschop/agon-utilities (MIT)
 * Port: ESP32-MOS (freestanding, API MOS via mos_libc.h)
 *
 * Concatena archivos al stdout filtrando los CR (\r).
 *
 * Uso desde el shell MOS:
 *   concat file1.txt file2.txt
 */

#define MOS_LIBC_IMPL
#include "mos_libc.h"

#define BUF_SIZE 1024
static unsigned char buf[BUF_SIZE];

__attribute__((section(".text.entry")))
int _start(int argc, char **argv, t_mos_api *mos)
{
    mos_libc_init(mos);

    FILE *f;
    int i, j;
    int nbytes;

    if (argc < 2) {
        printf("Usage: concat <file>+\r\n");
        return 1;
    }

    for (i = 1; i < argc; i++) {
        f = fopen(argv[i], "r");
        if (f == NULL) {
            printf("Cannot open file %s\r\n", argv[i]);
            continue;
        }
        while ((nbytes = (int)fread(buf, 1, BUF_SIZE, f)) > 0) {
            for (j = 0; j < nbytes; j++) {
                if (buf[j] != '\r')
                    printf("%c", (char)buf[j]);
            }
        }
        fclose(f);
    }

    return 0;
}
