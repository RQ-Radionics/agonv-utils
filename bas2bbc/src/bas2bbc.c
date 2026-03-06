/*
 * bas2bbc.c - Convert ASCII .bas file to tokenized BBC BASIC .bbc file
 *
 * Host build: standard stdio
 * ESP32-MOS build: uses MOS API jump table
 */

#ifdef ESP32_MOS
  #include "mos_api_table.h"
  static t_mos_api *g_mos;
  typedef uint8_t FILEHANDLE;
  #define INVALID_FH    0
  #define io_open(p,m)  g_mos->fopen(p,m)
  #define io_close(fh)  g_mos->fclose(fh)
  #define io_putc(c,fh) g_mos->fputc(c,fh)
  #define io_puts(s)    g_mos->puts(s)
  #define io_write(b,s,n,fh) g_mos->fwrite(b,s,n,fh)
  /* Read a line from file handle into buf. Returns 0 on EOF, 1 on success. */
  static int io_readline(FILEHANDLE fh, char *buf, int maxlen)
  {
      int i = 0;
      while (i < maxlen - 1) {
          int c = g_mos->fgetc(fh);
          if (c < 0 || c == 0xFF) { /* EOF */
              if (i == 0) return 0;
              break;
          }
          if (c == '\n') break;
          if (c == '\r') continue;
          buf[i++] = (char)c;
      }
      buf[i] = '\0';
      return 1;
  }
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  typedef FILE* FILEHANDLE;
  #define INVALID_FH    NULL
  #define io_open(p,m)  fopen(p,m)
  #define io_close(fh)  fclose(fh)
  #define io_putc(c,fh) fputc(c,fh)
  #define io_puts(s)    fputs(s,stdout)
  #define io_write(b,s,n,fh) fwrite(b,s,n,fh)
  /* Read a line from file, stripping CR/LF. Returns 0 on EOF. */
  static int io_readline(FILEHANDLE fh, char *buf, int maxlen)
  {
      if (fgets(buf, maxlen, fh) == NULL) return 0;
      /* Strip trailing CR/LF */
      int len = (int)strlen(buf);
      while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r'))
          buf[--len] = '\0';
      return 1;
  }
#endif

/* From bbc_tokenize.c */
extern int tokenize_line(const char *input, unsigned char *output, int lineno);

#ifdef ESP32_MOS
int _start(int argc, char **argv, t_mos_api *mos)
{
    g_mos = mos;
#else
int main(int argc, char **argv)
{
#endif
    if (argc < 3) {
        io_puts("Usage: bas2bbc input.bas output.bbc\n");
        return 1;
    }

    FILEHANDLE fin = io_open(argv[1], "r");
    if (fin == INVALID_FH) {
        io_puts("Error: cannot open input file\n");
        return 1;
    }

    FILEHANDLE fout = io_open(argv[2], "wb");
    if (fout == INVALID_FH) {
        io_puts("Error: cannot open output file\n");
        io_close(fin);
        return 1;
    }

    char linebuf[512];
    unsigned char tokbuf[512];

    while (io_readline(fin, linebuf, sizeof(linebuf)))
    {
        /* Skip empty lines */
        if (linebuf[0] == '\0') continue;

        /* Parse line number from start of line (skip leading spaces) */
        char *p = linebuf;
        while (*p == ' ' || *p == '\t') p++;
        int lineno = 0;
        int has_lineno = 0;
        while (*p >= '0' && *p <= '9') {
            lineno = lineno * 10 + (*p - '0');
            p++;
            has_lineno = 1;
        }
        /* Skip space after line number */
        if (*p == ' ') p++;

        if (!has_lineno) {
            io_puts("Warning: line without number, skipping: ");
            io_puts(linebuf);
            io_puts("\n");
            continue;
        }

        int toklen = tokenize_line(p, tokbuf, lineno);
        io_write(tokbuf, 1, toklen, fout);
    }

    /* Write end-of-program marker */
    unsigned char end_marker = 0x00;
    io_write(&end_marker, 1, 1, fout);

    io_close(fin);
    io_close(fout);
    return 0;
}
