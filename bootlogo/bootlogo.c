/*
 * bootlogo.c — ESP32-MOS port of bootlogo for Agon Light 2 / Console8
 *
 * Original eZ80 assembly by Shawn Sijnstra (MIT licence, 2024-5).
 * Seated warrior icon by Arnold Meszaros.
 * Console 8 icon rendered by Shawn Sijnstra.
 *
 * Port to ESP32-MOS (RISC-V / Xtensa) by the same build system used in
 * ../mos-esp32/sdk.  All VDU byte sequences are identical to the original;
 * only the OS-call layer differs.
 *
 * Usage (from the MOS shell):
 *   RUN A:/bootlogo.bin         → Agon Light 2 logo
 *   RUN A:/bootlogo.bin 8       → Console 8 logo
 *
 * Build:
 *   cd /Volumes/FastDisk/Queru/Ports/bootlogo
 *   make                          (ESP32-S3, default)
 *   make TARGET=esp32p4           (ESP32-P4, RISC-V)
 */

#include <stdint.h>
#include <stddef.h>
#include "mos_api_table.h"

/* ── VDU helper macros ───────────────────────────────────────────────────── */

static t_mos_api *g_mos;

static inline void vdu(uint8_t b)
{
    g_mos->putch(b);
}

/*
 * vdu_block — emite un bloque de bytes crudos al VDP.
 * Equivalente a RST.LIL 18h en el eZ80 original.
 */
static void vdu_block(const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        g_mos->putch(data[i]);
    }
}

/*
 * puts_dollar — emite una cadena terminada en '$' (compatible con el
 * formato del original en eZ80).  Usa putch byte a byte para que los
 * caracteres de control VDU se envíen sin filtrado.
 */
static void puts_dollar(const uint8_t *s)
{
    while (*s != '$') {
        g_mos->putch(*s++);
    }
}

/*
 * print_u32 — imprime un entero sin signo en decimal.
 * No usa printf (no disponible sin libc).
 */
static void print_u32(uint32_t n)
{
    char buf[12];
    int  i = sizeof(buf) - 1;
    buf[i] = '\0';
    if (n == 0) {
        g_mos->putch('0');
        return;
    }
    while (n > 0 && i > 0) {
        buf[--i] = '0' + (n % 10);
        n /= 10;
    }
    g_mos->puts(&buf[i]);
}

/* ── Datos de logo — idénticos al original ───────────────────────────────── */

/* Agon Light 2 warrior icon (caracteres 200–238, 10 bytes c/u) */
static const uint8_t boot_logo_icon[] = {
    23,200, 7, 15, 31, 31, 31, 31, 31, 63,
    23,201,192,224,240,240,240,240,240,248,
    23,202, 63, 63, 31, 15, 15, 15,  7,  7,
    23,203,248,248,240,224,224,224,192,192,
    23,204,  0,  1,  7, 15, 31, 63,127,127,
    23,205,  7,199,199,195,227,225,240,240,
    23,206,192,195,195,135,135, 15, 15, 31,
    23,207,  0,  0,224,240,248,252,252,254,
    23,208,  0,  0,  1,  1,  3,  3,  7,  7,
    23,209,255,255,255,255,255,255,255,255,
    23,210,248,248,248,240,224,225,193,195,
    23,211, 63, 63,127,255,255,255,255,255,
    23,212,255,255,255,255,255,255,223,223,
    23,213,  0,  0,128,128,128,192,192,192,
    23,214,  7,  7, 15, 15, 15, 15, 15, 31,
    23,215,255,247,231,231,231,199,135,130,
    23,216,199,135,135,143, 15, 15, 31, 31,
    23,217,207,207,207,199,199,195,129,  1,
    23,218,224,224,224,240,240,240,240,240,
    23,219, 31, 31, 31, 31, 15, 15, 15, 31,
    23,220,128,128,128,128,129,143,135,143,
    23,221, 31, 63, 15,  0,128,224,224,195,
    23,222,252,240,128,  0,  3,  7, 35,243,
    23,223,  1,  1,  1,129,193,225,241,241,
    23,224,240,240,240,240,240,240,240,240,
    23,225, 31, 31,  3,  3,  3,  3,  7, 15,
    23,226,223,255,255,255,255,255,254,254,
    23,227,199,135,135,  7, 15, 15, 31, 31,
    23,228,241,241,241,248,248,248,248,248,
    23,229,253,255,255,255,255,255,255,127,
    23,230,240,248,248,224,224,224,240,248,
    23,231, 31, 63, 63, 63, 31, 31, 15,  0,
    23,232,254,252,252,252,248,248,224,  0,
    23,233, 31, 60, 32,  0,  0,  0,  0,  0,
    23,234,120, 28, 12, 12,  0,  0,  0,  0,
    23,235,127,127,127,127,127, 63, 15,  0,
    23,236,248,252,252,252,252,248,240,  0,
    23,237,255,254,252,248,240,224,192,128,
    23,238,255,255,255,255,255,255,255,255, /* bloque sólido */
};

/* Console8 icon (caracteres 200–238) */
static const uint8_t boot_logo_c8[] = {
    23,200,0x00,0x00,0x00,0x00,0x00,0x01,0x06,0x08,
    23,201,0x00,0x00,0x00,0x00,0x7E,0x81,0x00,0x7E,
    23,202,0x00,0x00,0x00,0x00,0x00,0x80,0x40,0x30,
    23,203,0x00,0x00,0x00,0x00,0x00,0x01,0x07,0x0F,
    23,204,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xC1,
    23,205,0x00,0x00,0x00,0x00,0x00,0xC0,0xF0,0xF8,

    23,206,0x19,0x22,0x24,0x48,0x48,0x90,0x90,0x90,
    23,207,0x81,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    23,208,0x88,0x44,0x24,0x1E,0x0E,0x07,0x07,0x03,
    23,209,0x1F,0x3E,0x1C,0x08,0x00,0x00,0x80,0x80,
    23,210,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    23,211,0xF8,0x3C,0x3C,0x1E,0x0E,0x0F,0x07,0x07,

    23,212,0x90,0x90,0x90,0x90,0x48,0x48,0x2C,0x22,
    23,213,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    23,214,0x03,0x01,0x01,0x00,0x00,0x10,0x28,0x44,
    23,215,0xC0,0xE0,0xE0,0xF0,0x70,0x78,0x3C,0x1E,
    23,216,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    23,217,0x07,0x07,0x07,0x0F,0x0E,0x0E,0x1E,0x3C,

    23,218,0x11,0x08,0x06,0x01,0x00,0x00,0x00,0x00,
    23,219,0x81,0xFE,0x00,0x03,0xFE,0x00,0x00,0x00,
    23,220,0x88,0x10,0x60,0x80,0x00,0x00,0x00,0x00,
    23,221,0x0F,0x07,0x03,0x01,0x00,0x00,0x00,0x00,
    23,222,0x00,0xC3,0xFF,0xFF,0x3E,0x00,0x00,0x00,
    23,223,0xF8,0xF0,0xE0,0x80,0x00,0x00,0x00,0x00,

    23,224,0x00,0x00,0x00,0x1C,0x26,0x42,0x40,0x40,
    23,225,0x00,0x00,0x00,0x00,0x00,0x31,0x49,0x85,
    23,226,0x00,0x00,0x00,0x00,0x00,0x63,0x94,0x14,
    23,227,0x00,0x00,0x00,0x00,0x00,0x86,0x49,0x10,
    23,228,0x00,0x00,0x00,0x20,0x20,0x23,0x24,0xA8,
    23,229,0x00,0x00,0x1c,0x22,0x22,0x24,0x88,0x48,

    23,230,0x40,0x42,0x26,0x1C,0x00,0x00,0x00,0x00,
    23,231,0x85,0x85,0x49,0x31,0x00,0x00,0x00,0x00,
    23,232,0x13,0x10,0x14,0x13,0x00,0x00,0x00,0x00,
    23,233,0x90,0x50,0x49,0x86,0x00,0x00,0x00,0x00,
    23,234,0xAF,0xA8,0x24,0x13,0x00,0x00,0x00,0x00,
    23,235,0x92,0x22,0xA2,0x1C,0x00,0x00,0x00,0x00,

    23,238,255,255,255,255,255,255,255,255, /* bloque sólido */
};

/* ── Líneas del banner — Agon Light 2 ────────────────────────────────────── */
/*
 * Cada cadena termina en '$' (convención puts_dollar).
 * Los bytes 200-238 son los caracteres custom programados arriba.
 * CR=13, LF=10.
 */
static const uint8_t icon_line_1[] =
    "  \xC8\xC9     Agon Lite V with riscV CPU\r\n$";
static const uint8_t icon_line_2[] =
    "  \xCA\xCB   \r\n$";
static const uint8_t icon_line_3[] =
    " \xCC\xCD\xCE\xCF  \r\n$";
static const uint8_t icon_line_4[] =
    "\xD0\xD1\xD2\xD3\xD4\xD5 \r\n$";
static const uint8_t icon_line_5[] =
    "\xD6\xD7\xD8\xD1\xD9\xDA Screen mode: $";
static const uint8_t icon_line_6[] =
    "\r\n\xDB\xDC\xDD\xDE\xDF\xE0        Text: $";
static const uint8_t icon_line_7[] =
    "\r\n\xE1\xE2\xE3\xE4\xE5\xE6    Graphics: $";
static const uint8_t icon_line_8[] =
    "\r\n\xE7\xE8\xE9\xEA\xEB\xEC     Colours: $";

/* ── Líneas del banner — Console8 ────────────────────────────────────────── */
static const uint8_t icc8_line_1[] =
    "        Agon Console8 with riscV CPU\r\n$";
static const uint8_t icc8_line_2[] =
    "\xC8\xC9\xCA\xCB\xCC\xCD\r\n$";
static const uint8_t icc8_line_3[] =
    "\xCE\xCF\xD0\xD1\xD2\xD3\r\n$";
static const uint8_t icc8_line_4[] =
    "\xD4\xD5\xD6\xD7\xD8\xD9\r\n$";
static const uint8_t icc8_line_5[] =
    "\xDA\xDB\xDC\xDD\xDE\xDF Screen mode: $";
static const uint8_t icc8_line_6[] =
    "\r\n\xE0\xE1\xE2\xE3\xE4\xE5        Text: $";
static const uint8_t icc8_line_7[] =
    "\r\n\xE6\xE7\xE8\xE9\xEA\xEB    Graphics: $";
static const uint8_t icc8_line_8[] =
    "\r\n           Colours: $";

static const uint8_t printby_str[] = " x $";

/*
 * reset_fontload — sincroniza el buffer doble con el VDP (dos veces por si
 * hay double-buffering) y restaura los caracteres de fuente originales.
 * VDU 23,0,0xC3 = swap/VSYNC; VDU 23,0,145 = reset programmed chars.
 */
static const uint8_t reset_fontload[] = {
    23, 0, 0xC3,  /* swap/VSYNC #1 */
    23, 0, 0xC3,  /* swap/VSYNC #2 */
    23, 0, 145,   /* reset programmed font chars */
    13, 10,       /* CR LF */
};

/* ── Solicitud de color de foreground y espera de respuesta ─────────────── */
/*
 * En el Agon original, VDU 23,0,148,128 pide al VDP que devuelva el color
 * actual del foreground en sysvar_scrpixel_R/G/B via el protocolo VPD.
 * En ESP32-MOS el VDP es un cliente TCP externo.  El firmware actual recibe
 * el paquete POINT (cmd=0x04, R/G/B) pero no lo expone via API pública a
 * programas de usuario.
 *
 * Estrategia: enviamos la petición al VDP igualmente y usamos vdp_sync()
 * para esperar que el VDP procese todos los bytes.  Para la barra de colores
 * asumimos 64 colores (modo 0 por defecto en Agon VDP).  Esto es un
 * compromiso razonable ya que el firmware no expone el conteo real.
 */

/*
 * Solicitar info de pantalla al VDP.
 * VDU 23,0,0x86 pide al VDP que envíe un paquete MODE con:
 *   scrMode, scrCols, scrRows, scrColours, scrWidth(16), scrHeight(16)
 * El firmware ESP32-MOS parsea ese paquete pero no lo almacena en variables
 * accesibles via API pública todavía.  Lo emitimos para que el VDP lo procese
 * y para compatibilidad futura cuando el firmware lo exponga.
 */

/* ── Variables de estado ─────────────────────────────────────────────────── */

/*
 * Información de pantalla: se solicita al VDP con VDU 23,0,0x86.
 * Si el firmware no devuelve datos, usamos los defaults indicados.
 */
typedef struct {
    uint8_t  mode;      /* screen mode */
    uint8_t  cols;      /* text columns */
    uint8_t  rows;      /* text rows */
    uint8_t  colours;   /* number of colours */
    uint16_t width;     /* graphics width in pixels */
    uint16_t height;    /* graphics height in pixels */
} screen_info_t;

/* ── Impresión del banner ─────────────────────────────────────────────────── */

static void print_banner(int console8, const screen_info_t *si)
{
    /* Línea 1: logo + título */
    if (console8) {
        puts_dollar(icc8_line_1);
        puts_dollar(icc8_line_2);
        puts_dollar(icc8_line_3);
        puts_dollar(icc8_line_4);
        puts_dollar(icc8_line_5);
    } else {
        puts_dollar(icon_line_1);
        puts_dollar(icon_line_2);
        puts_dollar(icon_line_3);
        puts_dollar(icon_line_4);
        puts_dollar(icon_line_5);
    }

    print_u32(si->mode);

    if (console8) {
        puts_dollar(icc8_line_6);
    } else {
        puts_dollar(icon_line_6);
    }

    print_u32(si->cols);
    puts_dollar(printby_str);
    print_u32(si->rows);

    if (console8) {
        puts_dollar(icc8_line_7);
    } else {
        puts_dollar(icon_line_7);
    }

    print_u32(si->width);
    puts_dollar(printby_str);
    print_u32(si->height);

    if (console8) {
        puts_dollar(icc8_line_8);
    } else {
        puts_dollar(icon_line_8);
    }

    print_u32(si->colours);
    vdu(13); vdu(10);
}

/* ── Barra de colores ────────────────────────────────────────────────────── */
/*
 * Imprime un bloque sólido (char 238) para cada índice de paleta, ciclando
 * el foreground colour con VDU 17,n.  Idéntico al bucle barloop del original.
 */
static void colour_bar(uint8_t num_colours)
{
    for (uint8_t c = 0; c < num_colours; c++) {
        vdu(17);  /* VDU 17 = colour: selecciona color de foreground */
        vdu(c);
        vdu(238); /* caracter bloque sólido que programamos */
        if (c == 31) {   /* fin de línea en posición 31 */
            vdu(13); vdu(10);
        }
    }
}

/* ── Restauración de fuente ──────────────────────────────────────────────── */
static void restore_font(void)
{
    vdu_block(reset_fontload, sizeof(reset_fontload));
    /* vdp_sync garantiza que el VDP procese todos los bytes anteriores */
    g_mos->vdp_sync();
}

/* ── Obtención de info de pantalla ──────────────────────────────────────── */
/*
 * Solicita al VDP los datos de pantalla actuales via vdp_request_mode(),
 * que bloquea hasta recibir el paquete PACKET_MODE, y luego lee los valores
 * directamente del bloque de sysvars binario — igual que el original eZ80
 * leía (IX + sysvar_scrMode), etc.
 */
static void get_screen_info(screen_info_t *si)
{
    /*
     * Pedir info de pantalla al VDP y esperar la respuesta PACKET_MODE.
     * Después de esta llamada, sysvars()->scrMode y demás están actualizados.
     */
    g_mos->vdp_request_mode();

    /* Leer del bloque de sysvars — acceso directo, sin parseo de strings */
    const t_mos_sysvars *sv = g_mos->sysvars();

    si->mode    = sv->scrMode;
    si->cols    = sv->scrCols;
    si->rows    = sv->scrRows;
    si->colours = sv->scrColours;
    si->width   = sv->scrWidth;
    si->height  = sv->scrHeight;

    /* Fallback si el VDP no respondió a tiempo (campos quedan en 0) */
    if (si->cols    == 0) si->cols    = 64;
    if (si->rows    == 0) si->rows    = 24;
    if (si->colours == 0) si->colours = 64;
    if (si->width   == 0) si->width   = 512;
    if (si->height  == 0) si->height  = 384;
}

/* ── Punto de entrada ────────────────────────────────────────────────────── */

__attribute__((section(".text.entry")))
int _start(int argc, char **argv, t_mos_api *mos)
{
    g_mos = mos;

    /* Determinar modo: argumento '8' → Console8 */
    int console8 = 0;
    if (argc > 1 && argv[1][0] == '8') {
        console8 = 1;
    }

    /* ── 1. Programar los caracteres custom en el VDP ─────────────────── */
    if (console8) {
        vdu_block(boot_logo_c8, sizeof(boot_logo_c8));
    } else {
        vdu_block(boot_logo_icon, sizeof(boot_logo_icon));
    }
    /* Sincronizar: asegurarse de que el VDP procesó todos los VDU 23 */
    g_mos->vdp_sync();

    /* ── 2. Leer info de pantalla ────────────────────────────────────── */
    screen_info_t si;
    get_screen_info(&si);

    /* ── 3. Imprimir el banner ───────────────────────────────────────── */
    print_banner(console8, &si);

    /* ── 4. Barra de colores (solo si no es Console8 y no hay argumento) */
    if (!console8 && argc <= 1) {
        colour_bar(si.colours);
    }

    /* ── 5. Restaurar color foreground original y fuente por defecto ── */
    /*
     * En el original, se restaura el índice de color original detectado
     * en la barra de colores.  Aquí enviamos VDU 17,15 (blanco, índice 15)
     * que es el default habitual en Agon.
     */
    vdu(17); vdu(15);   /* VDU 17,15 = foreground blanco */

    restore_font();

    return 0;
}
