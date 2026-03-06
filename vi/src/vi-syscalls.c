/*
 * vi-syscalls.c - Newlib syscall stubs for vi on ESP32-MOS
 *
 * Newlib's stdio (fopen, fread, fwrite, malloc, printf…) uses these
 * low-level syscalls. We redirect them to the MOS API jump table.
 *
 * File descriptors 0-2 (stdin/stdout/stderr) → MOS console.
 * File descriptors 3+ → MOS file handles (fd = fh + 3).
 */

#include "mos_api_table.h"
#include <sys/stat.h>   /* struct stat */
#include <sys/types.h>  /* off_t */
#include <stdint.h>
#include <reent.h>      /* struct _reent, __getreent */

/* Global MOS API pointer — set by _start() */
t_mos_api *g_mos;

/* Map: fd → MOS file handle. fd 0,1,2 = console (fh unused). */
#define MAX_FD 16
static uint8_t _fd_to_fh[MAX_FD];  /* 0 = unused */

static int _alloc_fd(uint8_t fh)
{
    for (int i = 3; i < MAX_FD; i++) {
        if (!_fd_to_fh[i]) { _fd_to_fh[i] = fh; return i; }
    }
    return -1;
}

/* ── Syscall implementations ─────────────────────────────────────────── */

int _open(const char *path, int flags, int mode)
{
    (void)mode;
    /* flags: O_RDONLY=0, O_WRONLY=1, O_RDWR=2, O_CREAT|O_TRUNC=0x241 etc. */
    const char *mosmode;
    if (flags == 0)         mosmode = "rb";
    else if (flags & 0x200) mosmode = "wb";  /* O_CREAT */
    else                    mosmode = "rb";
    uint8_t fh = g_mos->fopen(path, mosmode);
    if (!fh) return -1;
    int fd = _alloc_fd(fh);
    if (fd < 0) { g_mos->fclose(fh); return -1; }
    return fd;
}

int _close(int fd)
{
    if (fd < 3 || fd >= MAX_FD) return 0;
    if (!_fd_to_fh[fd]) return -1;
    int r = g_mos->fclose(_fd_to_fh[fd]);
    _fd_to_fh[fd] = 0;
    return r ? -1 : 0;
}

int _read(int fd, char *buf, int len)
{
    if (fd < 3) return 0;  /* no stdin read */
    if (fd >= MAX_FD || !_fd_to_fh[fd]) return -1;
    return (int)g_mos->fread(buf, 1, (uint32_t)len, _fd_to_fh[fd]);
}

int _write(int fd, const char *buf, int len)
{
    if (fd == 1 || fd == 2) {
        /* console output */
        for (int i = 0; i < len; i++) g_mos->putch((uint8_t)buf[i]);
        return len;
    }
    if (fd >= MAX_FD || !_fd_to_fh[fd]) return -1;
    return (int)g_mos->fwrite(buf, 1, (uint32_t)len, _fd_to_fh[fd]);
}

off_t _lseek(int fd, off_t offset, int whence)
{
    if (fd < 3 || fd >= MAX_FD || !_fd_to_fh[fd]) return -1;
    g_mos->flseek(_fd_to_fh[fd], (long)offset, whence);
    return (off_t)g_mos->ftell(_fd_to_fh[fd]);
}

int _fstat(int fd, struct stat *st)
{
    (void)fd;
    /* Return minimal stat — enough to satisfy newlib FILE buffering */
    if (st) {
        for (int i = 0; i < (int)sizeof(struct stat); i++)
            ((char *)st)[i] = 0;
        st->st_mode = 0100644;  /* regular file */
        st->st_blksize = 512;
    }
    return 0;
}

int _isatty(int fd)
{
    return (fd == 0 || fd == 1 || fd == 2) ? 1 : 0;
}

/* _sbrk: heap for newlib malloc.
   MOS has its own malloc, but newlib also uses _sbrk internally
   for its own allocator. We provide a small static heap for newlib's
   internal needs (FILE buffers, etc.). vi.c uses malloc() from newlib
   which we redirect to MOS below. */
static char _heap[4096];
static char *_heap_ptr = _heap;

void *_sbrk(int incr)
{
    char *prev = _heap_ptr;
    if (_heap_ptr + incr > _heap + sizeof(_heap)) return (void *)-1;
    _heap_ptr += incr;
    return prev;
}

/* ── Override malloc/free/realloc to use MOS directly ────────────────── */
/* Newlib defines malloc/free/realloc in terms of _sbrk. We override them
   here to use MOS's allocator instead, which has access to PSRAM. */

/* MOS malloc doesn't record size. Use a 4-byte header for realloc support. */
void *malloc(size_t size)
{
    uint8_t *raw = (uint8_t *)g_mos->malloc(size + 4);
    if (!raw) return (void *)0;
    raw[0] = (uint8_t)(size & 0xFF);
    raw[1] = (uint8_t)((size >> 8) & 0xFF);
    raw[2] = (uint8_t)((size >> 16) & 0xFF);
    raw[3] = (uint8_t)((size >> 24) & 0xFF);
    return raw + 4;
}

void free(void *ptr)
{
    if (!ptr) return;
    g_mos->free((uint8_t *)ptr - 4);
}

void *calloc(size_t n, size_t sz)
{
    size_t total = n * sz;
    void *p = malloc(total);
    if (p) { char *c = (char *)p; for (size_t i = 0; i < total; i++) c[i] = 0; }
    return p;
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr) return malloc(size);
    uint8_t *raw = (uint8_t *)ptr - 4;
    size_t old = (size_t)raw[0] | ((size_t)raw[1]<<8) |
                 ((size_t)raw[2]<<16) | ((size_t)raw[3]<<24);
    uint8_t *nraw = (uint8_t *)g_mos->malloc(size + 4);
    if (!nraw) return (void *)0;
    nraw[0] = (uint8_t)(size & 0xFF);
    nraw[1] = (uint8_t)((size >> 8) & 0xFF);
    nraw[2] = (uint8_t)((size >> 16) & 0xFF);
    nraw[3] = (uint8_t)((size >> 24) & 0xFF);
    size_t copy = old < size ? old : size;
    uint8_t *src = (uint8_t *)ptr, *dst = nraw + 4;
    for (size_t i = 0; i < copy; i++) dst[i] = src[i];
    g_mos->free(raw);
    return dst;
}

/* ── errno (needed by newlib) ─────────────────────────────────────────── */
int errno = 0;

/* ── fflush_all stub (no buffering on MOS) ────────────────────────────── */
void fflush_all(void) { /* nop */ }

/* ── system(): no shell execution on MOS ─────────────────────────────── */
int system(const char *command) { (void)command; return -1; }

/* ── Newlib reentrancy support ────────────────────────────────────────── */
/* Newlib's stdio uses a per-task reentrancy structure. We run single-
   threaded on MOS, so provide a single global one. */
static struct _reent _global_reent = _REENT_INIT(_global_reent);

struct _reent *__getreent(void)
{
    return &_global_reent;
}

/* ── Process stubs (needed by abort/raise via newlib) ─────────────────── */
void _exit(int code) { g_mos->exit(code); }

int _kill(int pid, int sig)  { (void)pid; (void)sig; return -1; }
int _getpid(void)            { return 1; }

/* ── _start: MOS entry point ─────────────────────────────────────────── */
extern int vi_main(int argc, char **argv);

void _start(int argc, char **argv, t_mos_api *mos)
{
    g_mos = mos;
    int rc = vi_main(argc, argv);
    mos->exit(rc);
}
