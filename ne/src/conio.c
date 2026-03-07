/* Nano-style editor - Console I/O layer
   Copyright 2025, L.C. Benschop, Vught, The Netherlands.
   MIT license

   Ported to ESP32-MOS (agonv-utils) 2025.
   Replaces AgDev/VDP library calls with direct MOS API calls via g_mos.
*/

#include "edit.h"

/* Convenience macro: send a single byte to VDP */
#define putch(c) g_mos->putch((uint8_t)(c))

/* ── Key input ──────────────────────────────────────────────────────────── */

/*
 * EDT_GetKey: block until a key is pressed.
 *
 * The ESP32-MOS firmware maps arrow keys to the following ASCII codes:
 *   Up    → 0xC1   Down  → 0xC2   Right → 0xC3   Left  → 0xC4
 * (sent from mos_vdp_internal.cpp lines 2464-2477)
 *
 * All other special keys arrive via vkeycode in sysvars.
 * We translate arrow key ASCII values to 0x100+VKEY_* so that
 * editcore.c's switch statement handles them uniformly.
 */
unsigned int EDT_GetKey(void)
{
    int ch = g_mos->getkey();   /* blocking */

    /* Arrow keys come as special ASCII from VDP firmware */
    switch ((unsigned char)ch) {
    case 0xC1: return 0x100 + VKEY_UP;
    case 0xC2: return 0x100 + VKEY_DOWN;
    case 0xC3: return 0x100 + VKEY_RIGHT;
    case 0xC4: return 0x100 + VKEY_LEFT;
    default:   break;
    }

    /* Check vkeycode for other special keys */
    {
        t_mos_sysvars *sv = g_mos->sysvars();
        unsigned char vk = sv->vkeycode;
        if (vk == VKEY_PAGEUP   || vk == VKEY_PAGEDOWN ||
            vk == VKEY_DELETE   || vk == VKEY_HOME      ||
            vk == VKEY_END) {
            return (unsigned int)vk + 0x100;
        }
    }

    return (unsigned int)ch & 0xff;
}

/* ── Screen initialisation ─────────────────────────────────────────────── */

void EDT_InitScreen(void)
{
    /* Disable control key processing (VDU 23,0,0x98,0) */
    putch(23); putch(0); putch(0x98); putch(0);
    /* Paged mode off (FF / ctrl-O) */
    putch(15);
    /* Text viewport off */
    putch(26);

    /* Read screen dimensions from sysvars */
    g_mos->vdp_request_mode();      /* trigger mode-info update */
    g_mos->vdp_sync();
    {
        t_mos_sysvars *sv = g_mos->sysvars();
        EDT.scr_rows = sv->scrRows;
        EDT.scr_cols = sv->scrCols;
    }

    /* If fewer than 80 columns, switch to mode 3 (80×30) */
    if (EDT.scr_cols < 80) {
        putch(22); putch(3);
        g_mos->vdp_request_mode();
        g_mos->vdp_sync();
        t_mos_sysvars *sv = g_mos->sysvars();
        EDT.scr_rows = sv->scrRows;
        EDT.scr_cols = sv->scrCols;
    }

    /* Fixed colours: foreground = 15 (white), background = 0 (black).
       vdp_return_palette_entry_index() is not available in ESP32-MOS API. */
    EDT.fgcolour = 15;
    EDT.bgcolour = 0;
}

void EDT_ExitScreen(void)
{
    /* Re-enable control key processing (VDU 23,0,0x98,1) */
    putch(23); putch(0); putch(0x98); putch(1);
}

/* ── Colour helpers ────────────────────────────────────────────────────── */

void EDT_InvVideo(void)
{
    putch(17); putch(EDT.bgcolour);
    putch(17); putch(128 + EDT.fgcolour);
}

void EDT_TrueVideo(void)
{
    putch(17); putch(EDT.fgcolour);
    putch(17); putch(128 + EDT.bgcolour);
}

/* ── Cursor positioning ────────────────────────────────────────────────── */

void EDT_SetCursor(int x, int y)
{
    putch(31); putch((unsigned char)x); putch((unsigned char)y);
}

/* Return current text cursor position via sysvars.
   vdp_sync() flushes any pending VDP responses so cursorX/Y are fresh. */
static void VDUGetCursor(int *x, int *y)
{
    g_mos->vdp_sync();
    t_mos_sysvars *sv = g_mos->sysvars();
    *x = sv->cursorX;
    *y = sv->cursorY;
}

/* ── Line I/O ──────────────────────────────────────────────────────────── */

void EDT_ClrEOL(void)
{
    int x, y, n;
    VDUGetCursor(&x, &y);
    n = SCR_COLS - x;
    if (y == EDT.scr_rows - 1)
        n--;
    for (int i = 0; i < n; i++) putch(32);
}

void EDT_ReadLine(unsigned char *buf, int len)
{
    int i = 0;
    int c;
    /* Echo any existing content */
    for (; buf[i] != 0; i++)
        putch(buf[i]);
    /* Read new characters */
    for (;;) {
        c = EDT_GetKey();
        switch (c) {
        case -1:
            return;
        case 13:
            return;
        case 127:
            if (i > 0) {
                putch(127);
                i--;
                buf[i] = 0;
            }
            break;
        default:
            if (i < len && c >= 32) {
                putch(c);
                buf[i++] = (unsigned char)c;
                buf[i]   = 0;
            }
        }
    }
}

/* ── Line rendering ────────────────────────────────────────────────────── */

unsigned char *EDT_RenderLine(unsigned char *p, bool is_current)
{
    unsigned char c;
    bool is_scrolled = false;
    int col = 0;
    unsigned char *q = p;
    int tabstop = EDT.tab_stop;

    if (is_current) {
        /* Determine scroll offset if cursor is past screen width */
        for (;;) {
            if (q == EDT.gap_start) break;
            if (*q == '\n')         break;
            if (*q++ == '\t') {
                col = (col + tabstop) & (-tabstop);
                if (col > SCR_COLS - 1) col = SCR_COLS - 1;
            } else {
                col++;
            }
            if (col == SCR_COLS - 1) {
                col = 1;
                p = q;
                is_scrolled = true;
            }
        }
        EDT.cursor_col = col;
    }

    if (is_scrolled) {
        EDT_InvVideo();
        putch('<');
        EDT_TrueVideo();
        col = 1;
    } else {
        col = 0;
    }

    for (;;) {
        if (p == EDT.gap_start) p = EDT.gap_end;
        c = *p++;
        if (c == '\n') {
            if (col == SCR_COLS) {
                EDT_InvVideo();
                putch('>');
                EDT_TrueVideo();
            } else if (is_current) {
                EDT_ClrEOL();
            } else {
                putch(13); putch(10);
            }
            break;
        } else if (c == '\t' && col < SCR_COLS - 1) {
            do {
                putch(' ');
                col++;
            } while ((col & (tabstop - 1)) && col < SCR_COLS - 1);
        } else if (col < SCR_COLS - 1) {
            putch(c);
            col++;
        } else {
            col = SCR_COLS;
        }
    }
    return p;
}

void EDT_RenderCurrentLine(void)
{
    EDT_SetCursor(0, EDT.cursor_row + 1);
    EDT_RenderLine(EDT.gap_start - EDT.curline_pos, true);
    EDT_ShowCursor();
}

void EDT_LeaveCurrentLine(void)
{
    EDT_SetCursor(0, EDT.cursor_row + 1);
    EDT_RenderLine(EDT.gap_start - EDT.curline_pos, false);
}

void EDT_ShowCursor(void)
{
    EDT_SetCursor(EDT.cursor_col, EDT.cursor_row + 1);
    if (EDT.cursor_col_max < EDT.cursor_col)
        EDT.cursor_col_max = EDT.cursor_col;
}

/* ── Status bar ────────────────────────────────────────────────────────── */

void EDT_ShowBottom(void)
{
    /* Format into a fixed buffer, then output exactly SCR_COLS-1 chars.
       Never write to the last column on the last row — that would cause
       the VDP to auto-wrap and scroll the screen. */
    char buf[256];
    int  maxcols = (int)SCR_COLS - 1;
    if (maxcols <= 0) return;
    snprintf(buf, sizeof(buf),
             "Line %d/%d, %d/%d bytes -- ESC to exit, ^G for help %c Cut %d",
             EDT.lineno,
             EDT.total_lines,
             (int)(EDT.gap_start - EDT.text_start + EDT.text_end - EDT.gap_end),
             (int)(EDT.text_end - EDT.text_start),
             EDT.is_changed ? '*' : ' ',
             EDT.cut_lines);
    EDT_SetCursor(0, EDT.scr_rows - 1);
    EDT_InvVideo();
    int i;
    for (i = 0; i < maxcols && buf[i]; i++) putch((unsigned char)buf[i]);
    for (     ; i < maxcols;           i++) putch(' ');
    EDT_TrueVideo();
}

/* ── Full screen redraw ────────────────────────────────────────────────── */

void EDT_ShowScreen(void)
{
    unsigned char *p = EDT.top_line;
    putch(12);   /* FF: clear screen */
    EDT_InvVideo();
    printf("Nano Extended: %s", EDT.mem_start + FILENAME_OFFS);
    EDT_ClrEOL();
    EDT_TrueVideo();

    for (int i = 0; i < EDT.scr_rows - 2 && p != EDT.text_end; i++)
        p = EDT_RenderLine(p, i == EDT.cursor_row);

    EDT_ShowBottom();
    EDT_ShowCursor();
}

void EDT_AdjustTop(bool always_redraw)
{
    if (EDT.top_line > EDT.gap_start ||
        EDT.cursor_row >= EDT.scr_rows - 2) {
        int lines_to_move = (EDT.scr_rows - 2) / 2;
        EDT.cursor_row = 0;
        EDT.top_line   = EDT.gap_start - EDT.curline_pos;
        while (lines_to_move && EDT.top_line > EDT.text_start) {
            lines_to_move--;
            EDT.cursor_row++;
            EDT.top_line--;
            while (EDT.top_line > EDT.text_start && EDT.top_line[-1] != '\n')
                EDT.top_line--;
        }
        EDT_ShowScreen();
    } else if (always_redraw) {
        EDT_ShowScreen();
    } else {
        EDT_ShowBottom();
        EDT_RenderCurrentLine();
    }
}
