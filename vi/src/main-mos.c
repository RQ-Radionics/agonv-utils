/*
 * main-mos.c - ESP32-MOS entry point for vi
 *
 * vi.c is compiled with -Dmain=vi_main so its main() becomes vi_main().
 * This file provides the actual _start() required by the MOS ABI.
 *
 * MOS_LIBC_IMPL is defined in vi.c (via vi-mos.h). This file only uses
 * the public declarations from mos_libc.h.
 */

/* Do NOT define MOS_LIBC_IMPL here — vi.c owns the implementation */
#include "mos_libc.h"

/* Declared in vi-mos.h / vi.c translation unit */
extern t_mos_api *g_mos;
extern int vi_main(int argc, char **argv);

void _start(int argc, char **argv, t_mos_api *mos)
{
    g_mos = mos;
    mos_libc_init(mos);
    int rc = vi_main(argc, argv);
    mos->exit(rc);
}
