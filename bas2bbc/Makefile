NAME    = bas2bbc
CMD_DIR = $(CURDIR)
SRCS    = $(CMD_DIR)/src/bas2bbc.c \
          $(CMD_DIR)/src/bbc_tokenize.c

include ../sdk/Makefile

# bas2bbc uses #ifdef ESP32_MOS guards instead of mos_libc.h
override CFLAGS += -DESP32_MOS
