/*
 * echo.c - Port de agon-utils/echo para ESP32-MOS
 *
 * Original: https://github.com/vascocosta/agon-utils (MIT)
 * Port: ESP32-MOS (freestanding, API MOS via mos_libc.h)
 *
 * Uso desde el shell MOS:
 *   echo hello world
 *   echo -n sin_newline
 */

#define MOS_LIBC_IMPL
#include "mos_libc.h"

static void show_usage(const char *prog_name)
{
	printf("Usage: %s [-hn] string\r\n", prog_name);
	printf("-h show this help message\r\n");
	printf("-n supress trailing newline\r\n");
}

__attribute__((section(".text.entry")))
int _start(int argc, char **argv, t_mos_api *mos)
{
	mos_libc_init(mos);

	int newline = 1;

	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-h") == 0)
			{
				show_usage(argv[0]);
				return 0;
			}

			if (strcmp(argv[i], "-n") == 0)
			{
				newline = 0;
				continue;
			}

			printf("%s", argv[i]);

			if (i < argc - 1)
				printf(" ");
		}
	}

	if (newline)
		printf("\r\n");

	return 0;
}
