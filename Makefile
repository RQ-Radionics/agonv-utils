# Makefile raíz - porta agon-utils a ESP32-MOS
#
# Uso:
#   make                      → compila todos los comandos (default: esp32p4)
#   make TARGET=esp32s3       → compila para ESP32-S3 (Xtensa)
#   make echo                 → compila solo echo.bin
#   make install              → copia .bin a ../agon-lite-v/data/<TARGET>/
#   make clean
#
# Prerequisito: toolchain instalado via ESP-IDF
#   source ~/esp/esp-idf/export.sh

TARGET ?= esp32p4

COMMANDS  = echo grep head tail wc strings bootlogo cal concat sort ne bas2bbc bbc2bas
DATA_DIR  = $(abspath ../agon-lite-v/data/$(TARGET)/bin)

.PHONY: all clean install $(COMMANDS)

all: $(COMMANDS)

$(COMMANDS):
	$(MAKE) -C $@ TARGET=$(TARGET)

install: all
	@mkdir -p $(DATA_DIR)
	@for cmd in $(COMMANDS); do \
	    bin=$$cmd/bin/$$cmd.bin; \
	    if [ -f $$bin ]; then \
	        cp $$bin $(DATA_DIR)/$$cmd.bin; \
	        echo "  → $(DATA_DIR)/$$cmd.bin"; \
	    fi; \
	done

clean:
	for cmd in $(COMMANDS); do $(MAKE) -C $$cmd clean; done
