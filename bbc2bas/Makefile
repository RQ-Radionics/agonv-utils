NAME    = bbc2bas
CMD_DIR = $(CURDIR)
SRCS    = $(CMD_DIR)/src/bbc2bas.c \
          $(CMD_DIR)/src/bbc_detokenize.c

include ../sdk/Makefile

# bbc2bas uses #ifdef ESP32_MOS guards instead of mos_libc.h
override CFLAGS += -DESP32_MOS
