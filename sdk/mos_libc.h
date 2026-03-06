/*
 * mos_libc.h - Capa de compatibilidad libc -> MOS API para ESP32-MOS
 *
 * Permite compilar código C estándar (printf, fopen, malloc, etc.)
 * en modo -nostdlib usando la tabla de saltos MOS.
 *
 * Uso:
 *   #define MOS_LIBC_IMPL   (en UN solo .c, antes del include)
 *   #include "mos_libc.h"
 *
 *   En el resto de archivos (sin MOS_LIBC_IMPL):
 *   #include "mos_libc.h"
 *
 * El _start recibe el puntero MOS y debe llamar a mos_libc_init(mos)
 * antes de cualquier llamada a printf/fopen/etc.
 */

#ifndef MOS_LIBC_H
#define MOS_LIBC_H

#include "mos_api_table.h"

/* ── Tipos libc mínimos ───────────────────────────────────────────────── */
typedef unsigned char  uint8_t;
typedef unsigned int   size_t;
typedef long           off_t;

#define NULL  ((void *)0)
#define EOF   (-1)

/* FILE handle: envuelve el fh de MOS (uint8_t) */
typedef struct { uint8_t fh; } FILE;

/* Streams estándar pre-declarados */
extern FILE *stdout;
extern FILE *stderr;
extern FILE *stdin;

/* ── API pública ──────────────────────────────────────────────────────── */

/* Llamar desde _start antes de usar cualquier función libc */
void mos_libc_init(t_mos_api *mos);

/* I/O de consola */
int  printf(const char *fmt, ...);
int  fprintf(FILE *stream, const char *fmt, ...);
int  puts(const char *s);
int  fputc(int c, FILE *stream);
size_t fwrite(const void *buf, size_t sz, size_t n, FILE *stream);

/* Archivos */
FILE   *fopen(const char *path, const char *mode);
int     fclose(FILE *f);
int     fgetc(FILE *f);
char   *fgets(char *buf, int n, FILE *f);
int     feof(FILE *f);
size_t  fread(void *buf, size_t sz, size_t n, FILE *f);
long    ftell(FILE *f);
int     fseek(FILE *f, long offset, int whence);

/* Memoria */
void *malloc(size_t size);
void  free(void *ptr);
void *calloc(size_t n, size_t sz);

/* String utils (implementados aquí para no depender de libc) */
int    atoi(const char *s);
size_t strlen(const char *s);
int    strcmp(const char *a, const char *b);
int    strncmp(const char *a, const char *b, size_t n);
char  *strcpy(char *dst, const char *src);
char  *strncpy(char *dst, const char *src, size_t n);
char  *strstr(const char *hay, const char *needle);
int    tolower(int c);
int    isprint(int c);

/* fseek whence */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* ── Implementación (solo cuando MOS_LIBC_IMPL está definido) ─────────── */
#ifdef MOS_LIBC_IMPL

#include <stdarg.h>

/* Puntero global a la tabla MOS */
static t_mos_api *_mos;

/* Streams estándar: stdout/stderr apuntan a consola (fh=0), stdin no usado */
static FILE _stdout_f = {0};
static FILE _stderr_f = {0};
FILE *stdout = &_stdout_f;
FILE *stderr = &_stderr_f;
FILE *stdin  = NULL;

void mos_libc_init(t_mos_api *mos)
{
    _mos = mos;
}

/* ── String utils ─────────────────────────────────────────────────────── */

size_t strlen(const char *s)
{
    size_t n = 0;
    while (*s++) n++;
    return n;
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n)
{
    while (n-- && *a && *a == *b) { a++; b++; }
    if (!n) return 0;  /* n fue 0 tras el decremento: iguales hasta n */
    /* Tras el bucle: o se agotó n (ya retornado), o diferencia */
    return (unsigned char)*a - (unsigned char)*b;
}

char *strcpy(char *dst, const char *src)
{
    char *d = dst;
    while ((*d++ = *src++));
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
    char *d = dst;
    while (n-- && (*d++ = *src++));
    while (n-- > 0) *d++ = '\0';
    return dst;
}

char *strstr(const char *hay, const char *needle)
{
    if (!*needle) return (char *)hay;
    for (; *hay; hay++) {
        const char *h = hay, *n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return (char *)hay;
    }
    return NULL;
}

int tolower(int c)
{
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

int isprint(int c)
{
    return (c >= 32 && c < 127);
}

int atoi(const char *s)
{
    int sign = 1, val = 0;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    while (*s >= '0' && *s <= '9') val = val * 10 + (*s++ - '0');
    return sign * val;
}

/* ── printf mínimo ────────────────────────────────────────────────────── */

/* Escribe un entero decimal sin signo en buf, retorna longitud */
static int _uitoa(unsigned int v, char *buf)
{
    if (v == 0) { buf[0] = '0'; return 1; }
    char tmp[12]; int n = 0, len;
    while (v) { tmp[n++] = '0' + (v % 10); v /= 10; }
    for (len = 0; len < n; len++) buf[len] = tmp[n - 1 - len];
    return len;
}

/* Escribe un hex sin signo en buf */
static int _uxtoa(unsigned int v, char *buf, int upper)
{
    const char *hex = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[9]; int n = 0, len;
    if (v == 0) { buf[0] = '0'; return 1; }
    while (v) { tmp[n++] = hex[v & 0xF]; v >>= 4; }
    for (len = 0; len < n; len++) buf[len] = tmp[n - 1 - len];
    return len;
}

static void _out_str(const char *s, int to_stderr)
{
    (void)to_stderr;  /* consola única en MOS */
    _mos->puts(s);
}

static void _out_ch(char c)
{
    _mos->putch((uint8_t)c);
}

static int _vprintf_impl(const char *fmt, va_list ap)
{
    int count = 0;
    char buf[24];
    int len;

    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            _out_ch(*fmt);
            count++;
            continue;
        }
        fmt++;
        /* flags mínimos: %-s, %0Nd no implementados, suficiente para utils */
        switch (*fmt) {
        case 'd': case 'i': {
            int v = va_arg(ap, int);
            if (v < 0) { _out_ch('-'); count++; v = -v; }
            len = _uitoa((unsigned int)v, buf);
            buf[len] = '\0';
            _out_str(buf, 0);
            count += len;
            break;
        }
        case 'u': {
            unsigned int v = va_arg(ap, unsigned int);
            len = _uitoa(v, buf);
            buf[len] = '\0';
            _out_str(buf, 0);
            count += len;
            break;
        }
        case 'x': {
            unsigned int v = va_arg(ap, unsigned int);
            len = _uxtoa(v, buf, 0);
            buf[len] = '\0';
            _out_str(buf, 0);
            count += len;
            break;
        }
        case 'X': {
            unsigned int v = va_arg(ap, unsigned int);
            len = _uxtoa(v, buf, 1);
            buf[len] = '\0';
            _out_str(buf, 0);
            count += len;
            break;
        }
        case 's': {
            const char *s = va_arg(ap, const char *);
            if (!s) s = "(null)";
            _out_str(s, 0);
            count += (int)strlen(s);
            break;
        }
        case 'c': {
            char c = (char)va_arg(ap, int);
            _out_ch(c);
            count++;
            break;
        }
        case '%':
            _out_ch('%');
            count++;
            break;
        default:
            _out_ch('%');
            _out_ch(*fmt);
            count += 2;
            break;
        }
    }
    return count;
}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = _vprintf_impl(fmt, ap);
    va_end(ap);
    return r;
}

int fprintf(FILE *stream, const char *fmt, ...)
{
    (void)stream;  /* stdout y stderr van a consola */
    va_list ap;
    va_start(ap, fmt);
    int r = _vprintf_impl(fmt, ap);
    va_end(ap);
    return r;
}

int puts(const char *s)
{
    _mos->puts(s);
    _mos->puts("\r\n");
    return 0;
}

int fputc(int c, FILE *stream)
{
    (void)stream;
    _mos->putch((uint8_t)c);
    return c;
}

int fwrite_console(const void *buf, size_t n)
{
    const char *p = (const char *)buf;
    for (size_t i = 0; i < n; i++) _mos->putch((uint8_t)p[i]);
    return (int)n;
}

/* ── Archivos ─────────────────────────────────────────────────────────── */

/* Pool estático de FILE handles (MOS soporta pocos ficheros abiertos) */
#define MAX_FILES 8
static FILE _file_pool[MAX_FILES];
static int  _file_used[MAX_FILES];

FILE *fopen(const char *path, const char *mode)
{
    uint8_t fh = _mos->fopen(path, mode);
    if (!fh) return NULL;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!_file_used[i]) {
            _file_pool[i].fh = fh;
            _file_used[i] = 1;
            return &_file_pool[i];
        }
    }
    /* sin hueco: cerrar y retornar NULL */
    _mos->fclose(fh);
    return NULL;
}

int fclose(FILE *f)
{
    if (!f) return EOF;
    int r = _mos->fclose(f->fh);
    /* liberar slot */
    for (int i = 0; i < MAX_FILES; i++) {
        if (&_file_pool[i] == f) { _file_used[i] = 0; break; }
    }
    return r ? EOF : 0;
}

int fgetc(FILE *f)
{
    if (!f) return EOF;
    return _mos->fgetc(f->fh);
}

char *fgets(char *buf, int n, FILE *f)
{
    if (!f || n <= 0) return NULL;
    int i = 0;
    while (i < n - 1) {
        int c = _mos->fgetc(f->fh);
        if (c == EOF) break;
        buf[i++] = (char)c;
        if (c == '\n') break;
    }
    if (i == 0) return NULL;
    buf[i] = '\0';
    return buf;
}

int feof(FILE *f)
{
    if (!f) return 1;
    return _mos->feof(f->fh);
}

size_t fread(void *buf, size_t sz, size_t n, FILE *f)
{
    if (!f) return 0;
    return _mos->fread(buf, sz, n, f->fh);
}

size_t fwrite(const void *buf, size_t sz, size_t n, FILE *stream)
{
    if (!stream) return 0;
    /* stdout/stderr → consola */
    if (stream == stdout || stream == stderr) {
        fwrite_console(buf, sz * n);
        return n;
    }
    return _mos->fwrite(buf, sz, n, stream->fh);
}

long ftell(FILE *f)
{
    if (!f) return -1L;
    return _mos->ftell(f->fh);
}

int fseek(FILE *f, long offset, int whence)
{
    if (!f) return -1;
    return _mos->flseek(f->fh, offset, whence);
}

/* ── Memoria ──────────────────────────────────────────────────────────── */

void *malloc(size_t size)
{
    return _mos->malloc(size);
}

void free(void *ptr)
{
    _mos->free(ptr);
}

void *calloc(size_t n, size_t sz)
{
    size_t total = n * sz;
    void *p = _mos->malloc(total);
    if (p) {
        char *c = (char *)p;
        for (size_t i = 0; i < total; i++) c[i] = 0;
    }
    return p;
}

#endif /* MOS_LIBC_IMPL */
#endif /* MOS_LIBC_H */
