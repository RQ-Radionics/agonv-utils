#ifndef VI_MOS_H
#define VI_MOS_H

/*
 * vi-mos.h - Platform layer for vi on ESP32-MOS
 *
 * Strategy: vi.c uses the toolchain's newlib stdio.h (FILE*, fopen, etc.)
 * which calls POSIX syscalls (_open, _write, etc.). We implement those
 * syscalls in vi-syscalls.c to redirect to MOS API.
 *
 * This header provides only the vi-specific platform abstractions.
 */

#include "mos_api_table.h"
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#define VI_VER "Agon VI v1.06 is based on Busybox VI (ESP32-MOS port)"

/* ── Global MOS pointer ───────────────────────────────────────────────── */
/* Defined in vi-syscalls.c, used throughout */
extern t_mos_api *g_mos;

/* ── Key codes ────────────────────────────────────────────────────────── */
#define KEYCODE_UP       0x995
#define KEYCODE_DOWN     0x997
#define KEYCODE_RIGHT    0x996
#define KEYCODE_LEFT     0x998
#define KEYCODE_HOME     0x999
#define KEYCODE_END      0x99a
#define KEYCODE_PAGEUP   0x99b
#define KEYCODE_PAGEDOWN 0x99c
#define KEYCODE_DELETE   0x99d
#define KEYCODE_INSERT   0x99e

/* vkeycode values from mos_sysvars_block.h */
#define _VK_HOME     133
#define _VK_END      135
#define _VK_PAGEUP   146
#define _VK_PAGEDOWN 148
#define _VK_DELETE   130
#define _VK_INSERT   129

/* ── Feature flags ────────────────────────────────────────────────────── */
#define ENABLE_FEATURE_ALLOW_EXEC        1
#define ENABLE_FEATURE_VI_SEARCH         1
#define ENABLE_FEATURE_VI_YANKMARK       1
#define ENABLE_FEATURE_VI_DOT_CMD        1
#define ENABLE_FEATURE_VI_UNDO           1
#define ENABLE_FEATURE_VI_COLON          1
#define ENABLE_FEATURE_VI_READONLY       0
#define ENABLE_FEATURE_VI_ASK_TERMINAL   0
#define IF_FEATURE_VI_ASK_TERMINAL(x)    0
#define isbackspace(c) ((c) == 0x7f)

/* ── Platform functions ───────────────────────────────────────────────── */

static inline void platform_init(void)  { /* nothing */ }
static inline void platform_deinit(void) { /* nothing */ }

static inline int read_key(void)
{
    int ch = g_mos->getkey();

    switch ((unsigned char)ch) {
    case 0xC1: return KEYCODE_UP;
    case 0xC2: return KEYCODE_DOWN;
    case 0xC3: return KEYCODE_RIGHT;
    case 0xC4: return KEYCODE_LEFT;
    default:   break;
    }

    t_mos_sysvars *sv = g_mos->sysvars();
    switch (sv->vkeycode) {
    case _VK_HOME:     return KEYCODE_HOME;
    case _VK_END:      return KEYCODE_END;
    case _VK_PAGEUP:   return KEYCODE_PAGEUP;
    case _VK_PAGEDOWN: return KEYCODE_PAGEDOWN;
    case _VK_DELETE:   return KEYCODE_DELETE;
    case _VK_INSERT:   return KEYCODE_INSERT;
    default: break;
    }

    return ch & 0xff;
}

static inline int get_scr_cols(void)
{
    g_mos->vdp_request_mode();
    g_mos->vdp_sync();
    return g_mos->sysvars()->scrCols;
}

static inline int get_scr_rows(void)
{
    return g_mos->sysvars()->scrRows;
}

static inline void goto_xy(int x, int y)
{
    g_mos->putch(31);
    g_mos->putch((uint8_t)x);
    g_mos->putch((uint8_t)y);
}

/* system() is implemented in vi-syscalls.c — not static to avoid
   conflict with stdlib.h's non-static declaration */

/* ── Extra functions vi.c needs that newlib doesn't provide ──────────── */

/* putch: used by vi.c for direct character output */
static inline void putch(int c)
{
    g_mos->putch((uint8_t)c);
}

/* strdup / strndup: not in newlib's string.h for freestanding builds */
#include <stdlib.h>  /* for malloc */
static inline char *strdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (p) { for (size_t i = 0; i < n; i++) p[i] = s[i]; }
    return p;
}

static inline char *strndup(const char *s, size_t n)
{
    size_t len = 0;
    while (len < n && s[len]) len++;
    char *p = (char *)malloc(len + 1);
    if (p) { for (size_t i = 0; i < len; i++) p[i] = s[i]; p[len] = '\0'; }
    return p;
}

/* isblank: ctype.h may define it as a macro (glibc/newlib do).
   Undefine any macro version so vi.c's own static definition at line 2197
   compiles without a "static declaration follows non-static" error. */
#undef isblank

#endif /* VI_MOS_H */
