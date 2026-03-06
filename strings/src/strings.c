/*
 * strings.c - Port de agon-utils/strings para ESP32-MOS
 *
 * Original: https://github.com/vascocosta/agon-utils (MIT)
 * Port: ESP32-MOS (freestanding, API MOS via mos_libc.h)
 *
 * Uso desde el shell MOS:
 *   strings archivo.bin
 *   strings -n 6 archivo.bin
 */

#define MOS_LIBC_IMPL
#include "mos_libc.h"

static void show_usage(const char *prog_name)
{
	printf("Usage: %s [-hn min-len] filename\r\n", prog_name);
	printf("-h show this help message\r\n");
	printf("-n strings at least min-len long (default: 4)\r\n");
}

static void show_str(size_t str_len, FILE *file)
{
	char ch;
	char buf[1024];
	size_t cur_len = 0;

	while (fread(&ch, 1, 1, file) > 0)
	{
		if ((unsigned char)ch > 31 && (unsigned char)ch < 128)
		{
			if (cur_len < sizeof(buf) - 1)
				buf[cur_len++] = ch;
		}
		else if (cur_len >= str_len)
		{
			buf[cur_len] = '\0';
			printf("%s\r\n", buf);
			cur_len = 0;
		}
		else
		{
			cur_len = 0;
		}
	}

	/* Imprimir última cadena pendiente si cumple longitud */
	if (cur_len >= str_len)
	{
		buf[cur_len] = '\0';
		printf("%s\r\n", buf);
	}
}

__attribute__((section(".text.entry")))
int _start(int argc, char **argv, t_mos_api *mos)
{
	mos_libc_init(mos);

	FILE *file = NULL;
	char *filename = NULL;
	int parsed_len = 0;
	size_t str_len = 4;

	for (int i = 1; i != argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0)
		{
			show_usage(argv[0]);
			return 0;
		}
		else if (strncmp(argv[i], "-n", 2) == 0)
		{
			parsed_len = atoi(argv[++i]);

			if (parsed_len <= 0)
			{
				printf("The min-len must be positive\r\n");
				return 1;
			}

			str_len = (size_t)parsed_len;
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

	show_str(str_len, file);
	fclose(file);

	return 0;
}
