/*
 * wc.c - Port de agon-utils/wc para ESP32-MOS
 *
 * Original: https://github.com/vascocosta/agon-utils (MIT)
 * Port: ESP32-MOS (freestanding, API MOS via mos_libc.h)
 *
 * Uso desde el shell MOS:
 *   wc archivo.txt
 *   wc -l archivo.txt
 *   wc -w archivo.txt
 *   wc -c archivo.txt
 */

#define MOS_LIBC_IMPL
#include "mos_libc.h"

static void show_usage(const char *prog_name)
{
	printf("Usage: %s [-chlw] filename\r\n", prog_name);
	printf("-c print the characters count\r\n");
	printf("-h show this help message\r\n");
	printf("-l print the lines count\r\n");
	printf("-w print the words count\r\n");
}

static unsigned count_words(const char *str)
{
	int state = 0;
	unsigned wc = 0;

	while (*str)
	{
		if (*str == ' ' || *str == '\n' || *str == '\t')
			state = 0;
		else if (state == 0) {
			state = 1;
			wc++;
		}
		str++;
	}

	return wc;
}

static void show_counts(FILE *file, int do_lines, int do_words, int do_chars)
{
	char buf[1024];
	size_t bytes_read;
	int lc = 0, wc = 0, cc = 0;

	while ((bytes_read = fread(buf, 1, sizeof(buf), file)) > 0)
	{
		for (size_t i = 0; i < bytes_read; i++)
		{
			if (buf[i] == '\n')
				lc++;
			else if (buf[i] != '\r')
				cc++;
		}
		buf[bytes_read] = '\0';
		wc += (int)count_words(buf);
	}

	if (!(do_lines || do_words || do_chars))
	{
		printf("%d %d %d", lc, wc, cc);
	}
	else
	{
		if (do_lines) printf("%d ", lc);
		if (do_words) printf("%d ", wc);
		if (do_chars) printf("%d", cc);
	}

	printf("\r\n");
}

__attribute__((section(".text.entry")))
int _start(int argc, char **argv, t_mos_api *mos)
{
	mos_libc_init(mos);

	FILE *file = NULL;
	char *filename = NULL;
	int do_lines = 0, do_words = 0, do_chars = 0;

	for (int i = 1; i != argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0)
		{
			show_usage(argv[0]);
			return 0;
		}
		else if (strcmp(argv[i], "-l") == 0)
			do_lines = 1;
		else if (strcmp(argv[i], "-w") == 0)
			do_words = 1;
		else if (strcmp(argv[i], "-c") == 0)
			do_chars = 1;
		else if (!filename)
			filename = argv[i];
		else
		{
			show_usage(argv[0]);
			return 0;
		}
	}

	if (filename == NULL)
	{
		show_usage(argv[0]);
		return 0;
	}

	if (!(file = fopen(filename, "r")))
	{
		printf("Error opening file\r\n");
		return 1;
	}

	show_counts(file, do_lines, do_words, do_chars);
	fclose(file);

	return 0;
}
