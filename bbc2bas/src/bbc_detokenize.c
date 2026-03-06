/*
 * bbc_detokenize.c - Detokenize BBC BASIC lines to ASCII
 *
 * Adapted from BBCSDL/src/bbmain.c (R.T. Russell):
 *   token(), listline()
 */

#include <string.h>
#include "bbc_tokens.h"

/* Output buffer state for detokenization */
static char *out_ptr;
static char *out_end;

static void out_char(char c)
{
	if (out_ptr < out_end)
		*out_ptr++ = c;
}

static void out_str(const char *s)
{
	while (*s)
		out_char(*s++);
}

/* Format unsigned int into buffer, return pointer to start */
static char *fmt_uint(unsigned int val, char *buf, int bufsz)
{
	char *p = buf + bufsz - 1;
	*p = '\0';
	if (val == 0) {
		*--p = '0';
		return p;
	}
	while (val && p > buf) {
		*--p = '0' + (val % 10);
		val /= 10;
	}
	return p;
}

/* Format unsigned int right-justified in 5 chars */
static void out_lineno(unsigned int val)
{
	char tmp[8];
	char *s = fmt_uint(val, tmp, sizeof(tmp));
	int len = (int)strlen(s);
	for (int i = 0; i < 5 - len; i++)
		out_char(' ');
	out_str(s);
}

/* Output a character or keyword */
static void token(signed char al)
{
	if (al >= ' ')
	{
		out_char(al);
	}
	else
	{
		const signed char *tok = (const signed char *)strchr((const char *)keywds, al);
		if (tok != NULL)
			while (*++tok > ' ')
				out_char(*tok);
	}
}

/*
 * detokenize_line - Convert one tokenized BBC BASIC line to ASCII.
 *
 * input:  pointer to the start of a tokenized line (the length byte)
 *         Format: [length] [lineno_lo] [lineno_hi] [tokens...] [0x0D]
 * output: buffer to write ASCII text (NUL-terminated)
 * bufsize: size of output buffer
 *
 * Returns total length of the ASCII text produced (not including NUL).
 */
int detokenize_line(const unsigned char *input, char *output, int bufsize)
{
	unsigned char linelen = input[0];
	unsigned short lineno = input[1] | (input[2] << 8);
	const signed char *p = (const signed char *)(input + 3);
	const signed char *end_tok = (const signed char *)(input + linelen);

	/* Set up output buffer */
	out_ptr = output;
	out_end = output + bufsize - 1; /* reserve space for NUL */

	/* Write line number + space separator */
	out_lineno(lineno);
	out_char(' ');

	unsigned char mode = BIT0; /* start in left mode */

	while (p < end_tok && *p != 0x0D)
	{
		signed char al = *p++;
		if ((al == '"') && !(mode & 0x60))
			mode ^= BIT7;
		if (mode & (BIT5 | BIT6 | BIT7))
		{
			out_char(al);
		}
		else
		{
			if ((al == '*') && (mode & BIT0))
				mode |= BIT4; /* *command */
			if ((al == TDATA) && (mode & BIT0))
				mode |= BIT5; /* DATA */
			if (al == TREM)
				mode |= BIT6; /* REM */
			if (al != ' ')
				mode &= ~(BIT0 | BIT1); /* right mode, clear EXIT */
			if (al == TEXIT)
				mode |= BIT1; /* EXIT */
			if (strchr((const char *)list2, al))
				mode |= BIT0;
			if (al == (signed char)TLINO)
			{
				unsigned char ah = *(const unsigned char *)p++;
				unsigned short lino;
				lino = ((*(const unsigned char *)p++) ^ ((ah << 2) & 0xC0));
				lino += ((*(const unsigned char *)p++) ^ ((ah << 4) & 0xC0)) * 256;
				char tmp[8];
				out_str(fmt_uint(lino, tmp, sizeof(tmp)));
			}
			else
			{
				token(al);
			}
		}
	}

	*out_ptr = '\0';
	return (int)(out_ptr - output);
}
