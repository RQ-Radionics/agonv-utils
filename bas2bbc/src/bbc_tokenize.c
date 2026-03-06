/*
 * bbc_tokenize.c - Tokenize ASCII BBC BASIC lines
 *
 * Adapted from BBCSDL/src/bbmain.c (R.T. Russell):
 *   tokit(), lexan(), encode()
 */

#include <string.h>
#ifndef ESP32_MOS
#include <stdio.h>
#endif
#include "bbc_tokens.h"

/* liston controls case-sensitivity: BIT3 = allow lowercase keywords */
static unsigned char liston = BIT3;

/* Encode line number into pseudo binary form */
static char *encode(unsigned short lino, char *ebx)
{
	unsigned char al = lino & 0xC0;
	unsigned char ah = (lino >> 8) & 0xC0;
	lino = (lino & 0x3F3F) | 0x4040;
	*ebx++ = (char)TLINO;
	*ebx++ = ((al | (ah >> 2)) >> 2) ^ 0x54;
	*ebx++ = lino & 0xFF;
	*ebx++ = lino >> 8;
	return ebx;
}

/* Search for a keyword in the keywds[] table.
 * Return token if found, unchanged character if not found.
 * If found, advance pointer past keyword. */
static signed char tokit(char **pesi, const signed char *ebx)
{
	signed char al, ah;
	while (1)
	{
		char *esi = *pesi;
		signed char tok = *ebx++;
		al = *esi++;
		ah = *ebx++;
		if (al < ah) break;
		if ((al == ah) || ((liston & BIT3) && ((al - 0x20) == ah)))
		{
			signed char lc = al - ah;
			do
			{
				al = *esi++;
				ah = *ebx++;
				if ((ah == '(') || (ah == '$'))
					lc = 0;
			}
			while (((al - lc) == ah) && (ah > ' '));
			if ((al == '.') || (ah < ' ') || ((ah == ' ') && !range1(al)))
			{
				if (al == '.')
					*pesi = esi - 1;
				else
					*pesi = esi - 2;
				return tok;
			}
		}
		while (*ebx >= ' ') ebx++;
	}
	return **pesi;
}

/* Lexical analysis: tokenize a line of text.
 * esi: input text (CR-terminated)
 * ebx: output buffer
 * mode: initial mode (BIT0 = left/statement mode)
 * Returns pointer past last byte written (0x0D). */
static char *lexan(char *esi, char *ebx, unsigned char mode)
{
	signed char al;
	while (1)
	{
		al = *esi;
		if (al == 0x0D || al == '\0') break;
		if (!range1(al)) mode &= ~(BIT3|BIT5);
		if ((al != ' ') && (al != ','))
		{
			if ((al >= 'g') || (al == '@') || (al == '_') || (al == '`') ||
			    ((al >= 'G') && (((liston & BIT3) == 0) || (al < 'a'))))
				mode &= ~BIT3; /* not in hex */
			if (al == '"') mode ^= BIT7;
			if (mode & BIT4)
			{
				mode &= ~BIT4;
				unsigned int lino = 0;
				char *s = esi;
				if (al != '+') {
					while (*s == ' ') s++;
					while (*s >= '0' && *s <= '9')
						lino = lino * 10 + (*s++ - '0');
				}
				esi = s;
				if (lino)
				{
					mode |= BIT4;
					encode(lino, ebx);
					ebx += 4;
				}
				continue;
			}
			if (mode <= 1)
			{
				if (mode == 1) /* left mode */
				{
					mode = 0; /* right mode */
					if (al == '*')
						mode |= BIT6;
					else if ((al >= 'A') && (al <= 'z'))
						al = tokit(&esi, keywds);
					if (al == TDATA)
						mode |= BIT6;
					else if ((al >= (signed char)TOKLO) && (al <= (signed char)TOKHI))
						al += OFFSIT;
				}
				else if ((al >= 'A') && (al <= 'z'))
					al = tokit(&esi, keywds);
				if (al == TREM) mode |= BIT6; /* quit tokenising */
				if ((al == TFN) || (al == TPROC) || range2(al)) mode |= BIT5;
				if (al == '&') mode |= BIT3; /* in hex */
				if (strchr((const char *)list1, al)) mode |= BIT4; /* accept line number */
				if (strchr((const char *)list2, al)) mode |= BIT0; /* enter left mode */
			}
		}
		*ebx++ = al;
		esi++;
	}
	*ebx++ = 0x0D;
	return ebx;
}

/*
 * tokenize_line - Tokenize one ASCII line into BBC BASIC format.
 *
 * input:  ASCII text of the line (without line number, NUL-terminated)
 * output: buffer to write tokenized line (must be >= 256 bytes)
 * lineno: line number (1-65279)
 *
 * Output format:
 *   [length] [lineno_lo] [lineno_hi] [tokens...] [0x0D]
 *
 * Returns total length of the tokenized line (including header and 0x0D).
 */
int tokenize_line(const char *input, unsigned char *output, int lineno)
{
	char tmpbuf[512];
	char cr_input[512];

	/* Make a CR-terminated copy of the input for lexan */
	int len = (int)strlen(input);
	if (len > (int)sizeof(cr_input) - 2) len = (int)sizeof(cr_input) - 2;
	memcpy(cr_input, input, len);
	cr_input[len] = 0x0D;
	cr_input[len + 1] = '\0';

	/* Tokenize into tmpbuf */
	char *end = lexan(cr_input, tmpbuf, BIT0); /* BIT0 = start in left mode */
	int toklen = (int)(end - tmpbuf);

	/* Build output: [length] [lineno_lo] [lineno_hi] [tokens...0x0D] */
	int total = 3 + toklen; /* 1 byte length + 2 bytes lineno + tokenized content */
	output[0] = (unsigned char)total;
	output[1] = (unsigned char)(lineno & 0xFF);
	output[2] = (unsigned char)((lineno >> 8) & 0xFF);
	memcpy(output + 3, tmpbuf, toklen);

	return total;
}
