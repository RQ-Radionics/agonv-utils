# Makefile raíz - porta agon-utils a ESP32-MOS
#
# Uso:
#   make                      → compila todos los comandos (default: esp32p4)
#   make TARGET=esp32s3       → compila para ESP32-S3 (Xtensa)
#   make echo                 → compila solo echo.bin
#   make clean
#
# Prerequisito: toolchain instalado via ESP-IDF
#   source ~/esp/esp-idf/export.sh

TARGET ?= esp32p4

COMMANDS = echo grep head tail wc strings bootlogo

.PHONY: all clean $(COMMANDS)

all: $(COMMANDS)

$(COMMANDS):
	$(MAKE) -C $@ TARGET=$(TARGET)

clean:
	for cmd in $(COMMANDS); do $(MAKE) -C $$cmd clean; done
