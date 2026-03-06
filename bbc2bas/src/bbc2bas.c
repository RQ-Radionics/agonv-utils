/*
 * bbc2bas.c - Convert tokenized BBC BASIC .bbc file to ASCII .bas file
 *
 * Host build: standard stdio
 * ESP32-MOS build: uses MOS API jump table
 */

#include <string.h>

#ifdef ESP32_MOS
  #include "mos_api_table.h"
  static t_mos_api *g_mos;
  typedef uint8_t FILEHANDLE;
  #define INVALID_FH    0
  #define io_open(p,m)  g_mos->fopen(p,m)
  #define io_close(fh)  g_mos->fclose(fh)
  #define io_puts(s)    g_mos->puts(s)
  #define io_read(b,s,n,fh)  g_mos->fread(b,s,n,fh)
  #define io_write(b,s,n,fh) g_mos->fwrite(b,s,n,fh)
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  typedef FILE* FILEHANDLE;
  #define INVALID_FH    NULL
  #define io_open(p,m)  fopen(p,m)
  #define io_close(fh)  fclose(fh)
  #define io_puts(s)    fputs(s,stdout)
  #define io_read(b,s,n,fh)  fread(b,s,n,fh)
  #define io_write(b,s,n,fh) fwrite(b,s,n,fh)
#endif

/* From bbc_detokenize.c */
extern int detokenize_line(const unsigned char *input, char *output, int bufsize);

#ifdef ESP32_MOS
int _start(int argc, char **argv, t_mos_api *mos)
{
    g_mos = mos;
#else
int main(int argc, char **argv)
{
#endif
    if (argc < 3) {
        io_puts("Usage: bbc2bas input.bbc output.bas\n");
        return 1;
    }

    FILEHANDLE fin = io_open(argv[1], "rb");
    if (fin == INVALID_FH) {
        io_puts("Error: cannot open input file\n");
        return 1;
    }

    FILEHANDLE fout = io_open(argv[2], "w");
    if (fout == INVALID_FH) {
        io_puts("Error: cannot open output file\n");
        io_close(fin);
        return 1;
    }

    unsigned char linebuf[256];
    char textbuf[1024];

    while (1)
    {
        /* Read length byte */
        if (io_read(linebuf, 1, 1, fin) != 1) break;
        unsigned char linelen = linebuf[0];

        /* End of program marker */
        if (linelen == 0) break;

        /* Read rest of the line (linelen - 1 bytes since we already read the length) */
        if (io_read(linebuf + 1, 1, linelen - 1, fin) != (size_t)(linelen - 1)) break;

        /* Detokenize */
        int textlen = detokenize_line(linebuf, textbuf, sizeof(textbuf));
        (void)textlen;

        /* Write text line + newline */
        io_write(textbuf, 1, strlen(textbuf), fout);
        io_write("\n", 1, 1, fout);
    }

    io_close(fin);
    io_close(fout);
    return 0;
}
