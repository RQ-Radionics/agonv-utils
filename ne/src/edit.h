/* Nano-style editor
   Copyright 2025, L.C. Benschop, Vught, The Netherlands.
   MIT license

   Ported to ESP32-MOS (agonv-utils) 2025.
*/

#include "mos_libc.h"

/* Virtual key codes from mos_sysvars_block.h vkeycode field.
   Same numeric values as original AgDev VKEY_* constants. */
#define VKEY_UP       150
#define VKEY_DOWN     152
#define VKEY_LEFT     154
#define VKEY_RIGHT    156
#define VKEY_PAGEUP   146
#define VKEY_PAGEDOWN 148
#define VKEY_DELETE   130
#define VKEY_HOME     133
#define VKEY_END      135

/* The MOS global pointer, set in _start */
extern t_mos_api *g_mos;

/* putch helper — used throughout conio.c / editcore.c */
#define VDUWrite(c) g_mos->putch((uint8_t)(c))

#define EDIT_BUF_SIZE (400*1024)
#define CUT_BUF_SIZE  (32*1024)
#define MAX_NAME_LENGTH 128

struct _EditState {
  unsigned char *mem_start;
  unsigned char *text_start;
  unsigned char *top_line;
  unsigned char *gap_start;
  unsigned char *gap_end;
  unsigned char *text_end;
  unsigned char *cut_end;
  unsigned char *mem_end;
  unsigned int lineno;
  unsigned int total_lines;
  unsigned int cut_lines;
  unsigned char is_changed;
  unsigned char curline_pos;
  unsigned char curline_len;
  unsigned char cursor_col_max;
  unsigned char cursor_col;
  unsigned char cursor_row;
  unsigned char tab_stop;
  unsigned char scr_rows;
  unsigned char crlf_flag;
  unsigned char fgcolour;
  unsigned char bgcolour;
  unsigned char scr_cols;
};

extern struct _EditState EDT;

#define FILENAME_OFFS     0
#define BACKFILENAME_OFFS (FILENAME_OFFS+MAX_NAME_LENGTH+1)
#define SEARCHSTRING_OFFS (BACKFILENAME_OFFS+MAX_NAME_LENGTH+1)
#define LINENOSTRING_OFFS (SEARCHSTRING_OFFS+41)
#define VAR_END_OFFS      (LINENOSTRING_OFFS + 5)

#define SCR_COLS (EDT.scr_cols)

void EDT_EditCore(void);
unsigned int  EDT_GetKey(void);
void EDT_InitScreen(void);
void EDT_ExitScreen(void);
void EDT_LoadFile(unsigned char *name);
bool EDT_SaveFile(unsigned char *name);
void EDT_LoadConfig(char *name);
void EDT_InvVideo(void);
void EDT_TrueVideo(void);
void EDT_SetCursor(int x, int y);
void EDT_ClrEOL(void);
void EDT_ReadLine(unsigned char* buf, int len);
unsigned char * EDT_RenderLine(unsigned char *p, bool is_current);
void EDT_RenderCurrentLine(void);
void EDT_LeaveCurrentLine(void);
void EDT_ShowScreen(void);
void EDT_ShowCursor(void);
void EDT_ShowBottom(void);
void EDT_AdjustTop(bool always_redraw);

void EDT_BufStartLine(void);
void EDT_BufEndLine(void);
int EDT_BufLenCurLine(void);
void EDT_BufInsertChar(unsigned char c);
void EDT_BufInsertNL(void);
void EDT_BufDeleteChar(void);
void EDT_BufJoinLines(void);
void EDT_BufNextLine(void);
void EDT_BufPrevLine(void);
void EDT_BufNextChar(void);
void EDT_BufPrevChar(void);
void EDT_BufAdjustCol(void);
void EDT_BufDeleteLine(void);
bool EDT_BufCopyLine(void);
void EDT_BufPaste(void);
