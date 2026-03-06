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
#include <stdio.h>      /* setvbuf, _IONBF */

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
    /* flags: O_RDONLY=0, O_WRONLY=1, O_RDWR=2, O_CREAT=0x200, O_TRUNC=0x400 */
    const char *mosmode;
    if (flags == 0)         mosmode = "rb";
    else if (flags & 0x200) mosmode = "wb";  /* O_CREAT → write */
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
    if (!st) return 0;
    for (int i = 0; i < (int)sizeof(struct stat); i++)
        ((char *)st)[i] = 0;
    if (fd == 0 || fd == 1 || fd == 2) {
        st->st_mode = 0020666;  /* character device (tty) */
        return 0;
    }
    st->st_mode    = 0100644;   /* regular file */
    st->st_blksize = 512;
    if (fd >= 3 && fd < MAX_FD && _fd_to_fh[fd]) {
        /* Determine file size: seek-to-end, tell, restore position.
           newlib's fseek(SEEK_END) uses st_size from _fstat rather than
           calling _lseek(SEEK_END) directly, so this must be correct. */
        uint8_t fh  = _fd_to_fh[fd];
        long cur    = g_mos->ftell(fh);
        g_mos->flseek(fh, 0, 2);       /* SEEK_END */
        long size   = g_mos->ftell(fh);
        g_mos->flseek(fh, cur, 0);     /* SEEK_SET — restore */
        st->st_size = (off_t)size;
    }
    return 0;
}

int _isatty(int fd)
{
    return (fd == 0 || fd == 1 || fd == 2) ? 1 : 0;
}

/* _sbrk: provide newlib's allocator with a large heap from PSRAM.
   We allocate a single block from MOS at startup (_start sets _heap/_heap_end)
   and hand it out incrementally. This keeps a single consistent allocator —
   newlib's own malloc/free/realloc — so there are no pointer-provenance
   mismatches when newlib internally frees its own FILE buffers etc. */
#define HEAP_SIZE (256 * 1024)   /* 256 KB from PSRAM for vi's heap */

static char *_heap_base = (void *)0;
static char *_heap_ptr  = (void *)0;
static char *_heap_end  = (void *)0;

void *_sbrk(int incr)
{
    if (!_heap_base) return (void *)-1;   /* not yet initialised */
    char *prev = _heap_ptr;
    if (_heap_ptr + incr > _heap_end) return (void *)-1;
    _heap_ptr += incr;
    return prev;
}

/* ── errno (needed by newlib) ─────────────────────────────────────────── */
int errno = 0;

/* ── fflush_all stub (no buffering on MOS) ────────────────────────────── */
void fflush_all(void) { /* nop */ }

/* ── system(): no shell execution on MOS ─────────────────────────────── */
int system(const char *command) { (void)command; return -1; }

/* ── Newlib reentrancy support ────────────────────────────────────────── */
/* Newlib's stdio uses a per-task reentrancy structure. We run single-
   threaded on MOS, so provide a single global one.
   _impure_data / _impure_ptr are referenced directly by some newlib
   internals (e.g. __srefill_r, __sinit) independently of __getreent().
   We use _impure_data as the one true reent structure and point
   everything at it. */
struct _reent  _impure_data = _REENT_INIT(_impure_data);
struct _reent *_impure_ptr  = &_impure_data;

struct _reent *__getreent(void)
{
    return &_impure_data;
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

    /* Allocate heap from PSRAM via MOS, then wire it to _sbrk.
       This gives newlib's malloc/free/realloc a large coherent heap so
       that newlib-internal frees (FILE buffers, atexit, etc.) are always
       handled by the same allocator that allocated them. */
    _heap_base = (char *)mos->malloc(HEAP_SIZE);
    if (_heap_base) {
        _heap_ptr = _heap_base;
        _heap_end = _heap_base + HEAP_SIZE;
    }

    /* Force stdout unbuffered: vi uses fwrite(stdout) for every character
       drawn on screen. Without this newlib buffers output until the buffer
       fills (4096 bytes), making the screen appear blank during editing. */
    setvbuf(stdout, NULL, _IONBF, 0);

    int rc = vi_main(argc, argv);

    /* Return heap to MOS before exit */
    if (_heap_base) mos->free(_heap_base);

    mos->exit(rc);
}
