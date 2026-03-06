/* Nano-style editor
   Copyright 2025, L.C. Benschop, Vught, The Netherlands.
   MIT license

   Ported to ESP32-MOS (agonv-utils) 2025.
*/

#define MOS_LIBC_IMPL
#include "edit.h"

/* Global MOS API pointer — used by conio.c via g_mos */
t_mos_api *g_mos;

struct _EditState EDT;

void _start(int argc, char **argv, t_mos_api *mos)
{
    g_mos = mos;
    mos_libc_init(mos);

    EDT.mem_start = malloc(VAR_END_OFFS + EDIT_BUF_SIZE + CUT_BUF_SIZE);
    if (!EDT.mem_start) {
        printf("Not enough RAM\n");
        mos->exit(1);
    }
    EDT.mem_end = EDT.mem_start + VAR_END_OFFS + EDIT_BUF_SIZE + CUT_BUF_SIZE;

    if (argc != 2) {
        printf("Usage: ne <filename>\n");
        mos->exit(1);
    }

    strncpy((char *)EDT.mem_start + FILENAME_OFFS, argv[1], MAX_NAME_LENGTH);
    EDT_EditCore();
    mos->exit(0);
}
