/*
 * grep.c - Port de agon-utils/grep para ESP32-MOS
 *
 * Original: https://github.com/vascocosta/agon-utils (MIT)
 * Port: ESP32-MOS (freestanding, API MOS via mos_libc.h)
 *
 * Uso desde el shell MOS:
 *   grep patron archivo.txt
 *   grep -i patron archivo.txt
 */

#define MOS_LIBC_IMPL
#include "mos_libc.h"

static void show_usage(const char *prog_name)
{
	printf("Usage: %s [-hi] pattern filename\r\n", prog_name);
	printf("-h show this help message\r\n");
	printf("-i case insensitive matching\r\n");
}

static void match_pattern(int insensitive, char *pattern, FILE *file)
{
	char line[1024];

	while (fgets(line, sizeof(line), file))
	{
		char pat_copy[256];
		char line_copy[1024];

		if (insensitive)
		{
			int pi = 0;
			while (pattern[pi] && pi < 255) {
				pat_copy[pi] = (char)tolower((unsigned char)pattern[pi]);
				pi++;
			}
			pat_copy[pi] = '\0';

			int li = 0;
			while (line[li] && li < 1023) {
				line_copy[li] = (char)tolower((unsigned char)line[li]);
				li++;
			}
			line_copy[li] = '\0';

			if (strstr(line_copy, pat_copy))
				printf("%s", line);
		}
		else
		{
			if (strstr(line, pattern))
				printf("%s", line);
		}
	}
}

__attribute__((section(".text.entry")))
int _start(int argc, char **argv, t_mos_api *mos)
{
	mos_libc_init(mos);

	FILE *file;
	int insensitive = 0;
	char *pattern = NULL;
	char *filename = NULL;

	for (int i = 1; i != argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0)
		{
			show_usage(argv[0]);
			return 0;
		}
		else if (strcmp(argv[i], "-i") == 0)
		{
			insensitive = 1;
		}
		else if (!pattern)
		{
			pattern = argv[i];
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

	if (pattern == NULL || filename == NULL)
	{
		show_usage(argv[0]);
		return 0;
	}

	if (!(file = fopen(filename, "r")))
	{
		printf("Error opening file\r\n");
		return 1;
	}

	match_pattern(insensitive, pattern, file);
	fclose(file);

	return 0;
}
