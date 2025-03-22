# Makefile para ferramentas de teste do Mega_Emu
# Autor: Equipe Mega_Emu
# Data: 21/03/2025

# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -I../src
LDFLAGS = -lSDL2 -lm

# Diretórios
SRC_DIR = ../src
TOOLS_DIR = .
BUILD_DIR = ./build

# Fontes do sistema de áudio
AUDIO_SOURCES = \
	$(SRC_DIR)/platforms/megadrive/audio/ym2612.c \
	$(SRC_DIR)/platforms/megadrive/audio/sn76489.c \
	$(SRC_DIR)/platforms/megadrive/audio/audio_system.c \
	$(SRC_DIR)/common/logging.c

# Ferramentas
AUDIO_TEST = $(BUILD_DIR)/audio_test

# Alvos
all: directories $(AUDIO_TEST)

directories:
	mkdir -p $(BUILD_DIR)

# Compilação do teste de áudio
$(AUDIO_TEST): $(TOOLS_DIR)/audio_test.c $(AUDIO_SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Executar teste de áudio
run_audio_test: $(AUDIO_TEST)
	$(AUDIO_TEST)

# Limpar arquivos gerados
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all directories run_audio_test clean
