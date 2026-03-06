NAME    = vi
CMD_DIR = $(CURDIR)

# vi.c uses newlib stdio.h directly. vi-syscalls.c provides:
#   - Newlib syscall stubs (_open, _write, _sbrk, etc.) → MOS API
#   - malloc/free/realloc → MOS allocator
#   - _start() → MOS entry point
# vi.c is compiled with -Dmain=vi_main so _start() in vi-syscalls.c
# is the real entry point.
SRCS    = $(CMD_DIR)/src/vi.c \
          $(CMD_DIR)/src/vi-syscalls.c

include ../sdk/Makefile

# Add src/ to include path so our vi-agon.h shim shadows any other.
# -Dmain=vi_main renames main() in vi.c.
# Remove -fno-builtin so newlib's string functions work normally.
override CFLAGS += -I$(CMD_DIR)/src -Dmain=vi_main
override CFLAGS := $(filter-out -fno-builtin,$(CFLAGS))
# vi uses newlib stdio — do NOT use -nostdlib's own string stubs
# Keep -nostdlib but allow libc.a to provide stdio/string machinery
