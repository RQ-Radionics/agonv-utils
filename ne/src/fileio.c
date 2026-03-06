/* Nano-style editor - File I/O
   Copyright 2025, L.C. Benschop, Vught, The Netherlands.
   MIT license

   Ported to ESP32-MOS (agonv-utils) 2025.
   mos_del() / mos_ren() replaced with g_mos->del() / g_mos->rename().
*/

#include "edit.h"

void EDT_LoadFile(unsigned char *filename)
{
    FILE *fp;
    int len, curlinelen;
    unsigned char *p;

    fp = fopen((char *)filename, "rb");
    if (fp != NULL) {
        len = (int)fread(EDT.gap_start, 1,
                         (size_t)(EDT.gap_end - EDT.gap_start), fp);
        if (len > 0) {
            p = EDT.gap_start + len;
            if (*(p - 1) != '\n') {
                *--(EDT.gap_end) = '\n';
            }
            curlinelen = 0;
            while (p != EDT.gap_start) {
                char c = (char)*--p;
                if (EDT.gap_start == EDT.gap_end)
                    break;
                if (EDT.crlf_flag == 2 && c == 13)
                    EDT.crlf_flag = 1;
                if (((unsigned char)c >= 32 && c != 127) ||
                    c == '\n' || c == '\t') {
                    if (c == '\n') {
                        curlinelen = 0;
                        EDT.total_lines++;
                    } else {
                        curlinelen++;
                    }
                    if (curlinelen == 254) {
                        *--(EDT.gap_end) = '\n';
                        EDT.total_lines++;
                        if (EDT.gap_start == EDT.gap_end)
                            break;
                        curlinelen = 0;
                    }
                    *--(EDT.gap_end) = (unsigned char)c;
                }
            }
        }
        fclose(fp);
    }
    if (EDT.text_end == EDT.gap_end) {
        *--(EDT.gap_end) = '\n';
        EDT.total_lines++;
    }
}

bool EDT_SaveFile(unsigned char *filename)
{
    FILE *fp;
    char *p;

    /* Build backup filename */
    strcpy((char *)EDT.mem_start + BACKFILENAME_OFFS, (char *)filename);
    p = strrchr((char *)EDT.mem_start + BACKFILENAME_OFFS, '.');
    if (!p) {
        p = (char *)EDT.mem_start + BACKFILENAME_OFFS;
        p += strlen(p);
    }
    if (p - (char *)(EDT.mem_start + BACKFILENAME_OFFS) > MAX_NAME_LENGTH - 4)
        return false;
    strcpy(p, ".bak");

    /* Delete old backup and rename current → backup (ignore errors) */
    g_mos->del((char *)EDT.mem_start + BACKFILENAME_OFFS);
    g_mos->rename((char *)filename,
                  (char *)EDT.mem_start + BACKFILENAME_OFFS);

    /* Create new file */
    fp = fopen((char *)filename, "wb");
    if (fp == NULL) return false;

    if (EDT.crlf_flag != 1) {
        if (EDT.gap_start > EDT.text_start)
            fwrite(EDT.text_start,
                   (size_t)(EDT.gap_start - EDT.text_start), 1, fp);
        if (EDT.text_end > EDT.gap_end)
            fwrite(EDT.gap_end,
                   (size_t)(EDT.text_end - EDT.gap_end), 1, fp);
    } else {
        unsigned char *q;
        for (q = EDT.text_start; q < EDT.gap_start; q++) {
            if (*q == '\n') fwrite("\r", 1, 1, fp);
            fwrite(q, 1, 1, fp);
        }
        for (q = EDT.gap_end; q < EDT.text_end; q++) {
            if (*q == '\n') fwrite("\r", 1, 1, fp);
            fwrite(q, 1, 1, fp);
        }
    }
    fclose(fp);
    return true;
}

/* fgets replacement (avoids any libc fgets issues with EOF detection) */
static char *my_fgets(char *s, unsigned int maxlen, FILE *f)
{
    unsigned int i;
    char c;
    for (i = 0; i < maxlen - 1; ) {
        if (fread(&c, 1, 1, f) <= 0) break;
        if (c == '\r') continue;
        s[i] = c;
        i++;
        if (c == '\n') break;
    }
    s[i] = '\0';
    return (i == 0) ? NULL : s;
}

static unsigned char *parse_word(unsigned char *p)
{
    unsigned char *q;
    while (*p <= 32) {
        if (*p == 0) return p;
        p++;
    }
    q = p;
    while (*q > 32) q++;
    *q = '\0';
    return p;
}

void EDT_LoadConfig(char *name)
{
    FILE *fp;
    unsigned char *p, *q;

    fp = fopen(name, "rb");
    if (fp == NULL) return;

    while (my_fgets((char *)EDT.text_start, 81, fp)) {
        if (EDT.text_start[0] == '\n' || EDT.text_start[0] == '#')
            continue;
        p = EDT.text_start;
        p = parse_word(p);
        q = p + strlen((char *)p) + 1;
        q = parse_word(q);
        if (strcmp((char *)p, "mode") == 0) {
            if (strcmp((char *)q, "3") == 0)      EDT.scr_rows = 30;
            else if (strcmp((char *)q, "0") == 0) EDT.scr_rows = 60;
        } else if (strcmp((char *)p, "tabstop") == 0) {
            if (strcmp((char *)q, "4") == 0)      EDT.tab_stop = 4;
            else if (strcmp((char *)q, "8") == 0) EDT.tab_stop = 8;
        } else if (strcmp((char *)p, "eoln") == 0) {
            if (strcmp((char *)q, "crlf") == 0)       EDT.crlf_flag = 1;
            else if (strcmp((char *)q, "keep") == 0)  EDT.crlf_flag = 2;
            else if (strcmp((char *)q, "lf") == 0)    EDT.crlf_flag = 0;
        }
    }
    fclose(fp);
}
