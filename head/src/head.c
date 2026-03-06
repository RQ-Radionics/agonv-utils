/*
 * head.c - Port de agon-utils/head para ESP32-MOS
 *
 * Original: https://github.com/vascocosta/agon-utils (MIT)
 * Port: ESP32-MOS (freestanding, API MOS via mos_libc.h)
 *
 * Uso desde el shell MOS:
 *   head archivo.txt
 *   head -n 5 archivo.txt
 */

#define MOS_LIBC_IMPL
#include "mos_libc.h"

static void show_usage(const char *prog_name)
{
	printf("Usage: %s [-hn] filename\r\n", prog_name);
	printf("-h show this help message\r\n");
	printf("-n print the first n lines (default: 10)\r\n");
}

static void show_lines(FILE *file, int lines)
{
	char buf[1024];
	int lc = 0;

	while (fgets(buf, sizeof(buf), file) != NULL && lc < lines)
	{
		printf("%s", buf);
		lc++;
	}
}

__attribute__((section(".text.entry")))
int _start(int argc, char **argv, t_mos_api *mos)
{
	mos_libc_init(mos);

	FILE *file = NULL;
	char *filename = NULL;
	int parsed_lines = 0;
	int lines = 10;

	for (int i = 1; i != argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0)
		{
			show_usage(argv[0]);
			return 0;
		}
		else if (strncmp(argv[i], "-n", 2) == 0)
		{
			parsed_lines = atoi(argv[++i]);

			if (parsed_lines <= 0)
			{
				printf("The number of lines must be positive\r\n");
				return 1;
			}

			lines = parsed_lines;
		}
		else if (!filename)
		{
			filename = argv[i];
		}
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

	show_lines(file, lines);
	fclose(file);

	return 0;
}
