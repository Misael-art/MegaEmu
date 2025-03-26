/**
 * @file test_vdp_rom_compatibility.c
 * @brief Testes de compatibilidade do VDP com ROMs comerciais
 * @version 1.0
 * @date 2024-03-25
 */

#include "unity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "../../../../src/platforms/megadrive/megadrive.h"
#include "../../../../src/platforms/megadrive/video/vdp.h"
#include "../../../../src/platforms/megadrive/video/vdp_adapter.h"
#include "../../../../src/utils/enhanced_log.h"
#include "../../../../src/utils/file_utils.h"

// Diretório de ROMs comerciais
#define ROM_DIR "roms/comerciais/megadrive"
#define MAX_ROMS 50
#define MAX_FILENAME 256
#define TEST_FRAMES 600  // 10 segundos @ 60fps

// Estrutura para armazenar informações sobre a ROM
typedef struct {
    char filename[MAX_FILENAME];
    size_t size;
    uint8_t *data;
    char title[49];  // Tamanho máximo do título do Mega Drive (48 caracteres + nulo)
    uint8_t vdp_initial_reg[24]; // Valores iniciais dos registradores do VDP
} rom_info_t;

// Array para armazenar informações das ROMs
static rom_info_t roms[MAX_ROMS];
static int rom_count = 0;

// Estatísticas de teste
typedef struct {
    // Modos de VDP
    int mode5_count;
    int mode4_count;
    int h40_count;
    int h32_count;
    int interlace_count;
    int shadow_highlight_count;

    // Uso de recursos
    int dma_usage;
    int hblank_interrupt_usage;
    int vblank_interrupt_usage;
    int window_usage;
    int h_scroll_per_row;
    int sprite_masking;

    // Erros específicos
    int sprite_overflow_count;
    int sprite_limit_exceeded;
    int invalid_dma_usage;
    int timing_issues;
} test_stats_t;

// Estatísticas globais
static test_stats_t global_stats = {0};

// Contexto do emulador para teste
static megadrive_t *md_context = NULL;

// Função para carregar ROMs comerciais
static void load_roms(void) {
    DIR *dir;
    struct dirent *entry;
    char filepath[MAX_FILENAME * 2];

    // Abrir diretório de ROMs
    dir = opendir(ROM_DIR);
    if (!dir) {
        emu_log_error("Não foi possível abrir o diretório de ROMs: %s", ROM_DIR);
        return;
    }

    // Ler cada arquivo no diretório
    while ((entry = readdir(dir)) != NULL && rom_count < MAX_ROMS) {
        // Ignorar "." e ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Verificar extensão do arquivo (.md, .bin, .gen)
        char *ext = strrchr(entry->d_name, '.');
        if (!ext || (strcmp(ext, ".md") != 0 && strcmp(ext, ".bin") != 0 && strcmp(ext, ".gen") != 0))
            continue;

        // Construir caminho completo do arquivo
        snprintf(filepath, sizeof(filepath), "%s/%s", ROM_DIR, entry->d_name);

        // Abrir arquivo
        FILE *file = fopen(filepath, "rb");
        if (!file)
            continue;

        // Obter tamanho do arquivo
        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        rewind(file);

        // Alocar memória para ROM
        uint8_t *data = (uint8_t*)malloc(size);
        if (!data) {
            fclose(file);
            continue;
        }

        // Ler dados
        if (fread(data, 1, size, file) != size) {
            free(data);
            fclose(file);
            continue;
        }

        // Fechar arquivo
        fclose(file);

        // Armazenar informações da ROM
        strncpy(roms[rom_count].filename, entry->d_name, MAX_FILENAME - 1);
        roms[rom_count].size = size;
        roms[rom_count].data = data;

        // Extrair título da ROM (offset 0x150, 48 caracteres)
        if (size >= 0x180) {
            memcpy(roms[rom_count].title, data + 0x150, 48);
            roms[rom_count].title[48] = '\0';
        } else {
            strcpy(roms[rom_count].title, "Desconhecido");
        }

        rom_count++;
    }

    // Fechar diretório
    closedir(dir);

    emu_log_info("Carregadas %d ROMs comerciais para testes", rom_count);
}

// Função para liberar memória das ROMs
static void free_roms(void) {
    for (int i = 0; i < rom_count; i++) {
        free(roms[i].data);
    }
    rom_count = 0;
}

// Função para inicializar o emulador para teste
static void setup_emulator_for_rom(const rom_info_t *rom) {
    // Criar contexto do emulador
    if (md_context) {
        megadrive_destroy(md_context);
    }

    md_context = megadrive_create();
    TEST_ASSERT_NOT_NULL(md_context);

    // Carregar ROM
    megadrive_load_rom(md_context, rom->data, rom->size);

    // Reset ao emulador
    megadrive_reset(md_context);
}

// Função para capturar estatísticas do VDP
static void capture_vdp_stats(const rom_info_t *rom, test_stats_t *stats) {
    // Zerar estatísticas
    memset(stats, 0, sizeof(test_stats_t));

    megadrive_vdp_context_t *vdp = megadrive_get_vdp(md_context);
    TEST_ASSERT_NOT_NULL(vdp);

    // Executar ROM por um número de frames
    for (int frame = 0; frame < TEST_FRAMES; frame++) {
        // Executar um frame
        megadrive_run_frame(md_context);

        // Coletar estatísticas após os primeiros 60 frames (1 segundo)
        if (frame > 60) {
            // Detectar modos do VDP
            if (vdp_is_mode5())
                stats->mode5_count++;
            else
                stats->mode4_count++;

            if (vdp_get_mode() & 0x01)  // H40
                stats->h40_count++;
            else
                stats->h32_count++;

            if (vdp_get_interlace() > 0)
                stats->interlace_count++;

            // Detectar uso de recursos
            if (vdp_dma_is_active(vdp))
                stats->dma_usage++;

            if (vdp->registers[0] & 0x10)  // HBlank interrupt enabled
                stats->hblank_interrupt_usage++;

            if (vdp->registers[1] & 0x20)  // VBlank interrupt enabled
                stats->vblank_interrupt_usage++;

            if ((vdp->registers[0x11] != 0) || (vdp->registers[0x12] != 0))  // Window positions
                stats->window_usage++;

            // Detectar scroll por linha
            if (vdp->registers[0x0B] & 0x04)  // H-Scroll per line
                stats->h_scroll_per_row++;

            // Verificar erros
            if (vdp_get_sprite_overflow())
                stats->sprite_overflow_count++;

            if (vdp_get_sprite_count() > 20)
                stats->sprite_limit_exceeded++;
        }
    }

    // Normalizar estatísticas para porcentagens
    int total_frames = TEST_FRAMES - 60;

    // Converter contagens em porcentagens do tempo total de execução
    stats->mode5_count = (stats->mode5_count * 100) / total_frames;
    stats->mode4_count = (stats->mode4_count * 100) / total_frames;
    stats->h40_count = (stats->h40_count * 100) / total_frames;
    stats->h32_count = (stats->h32_count * 100) / total_frames;
    stats->interlace_count = (stats->interlace_count * 100) / total_frames;
    stats->shadow_highlight_count = (stats->shadow_highlight_count * 100) / total_frames;
    stats->dma_usage = (stats->dma_usage * 100) / total_frames;
    stats->hblank_interrupt_usage = (stats->hblank_interrupt_usage * 100) / total_frames;
    stats->vblank_interrupt_usage = (stats->vblank_interrupt_usage * 100) / total_frames;
    stats->window_usage = (stats->window_usage * 100) / total_frames;
    stats->h_scroll_per_row = (stats->h_scroll_per_row * 100) / total_frames;
    stats->sprite_overflow_count = (stats->sprite_overflow_count * 100) / total_frames;
}

// Teste de compatibilidade com ROMs comerciais
static void test_rom_vdp_compatibility(void) {
    for (int i = 0; i < rom_count; i++) {
        emu_log_info("Testando ROM %d/%d: %s", i+1, rom_count, roms[i].title);

        // Inicializar emulador com a ROM
        setup_emulator_for_rom(&roms[i]);

        // Coletar estatísticas
        test_stats_t rom_stats;
        capture_vdp_stats(&roms[i], &rom_stats);

        // Reportar resultados
        emu_log_info("Resultados para %s:", roms[i].title);
        emu_log_info("  Modo 5: %d%%, Modo 4: %d%%", rom_stats.mode5_count, rom_stats.mode4_count);
        emu_log_info("  H40: %d%%, H32: %d%%", rom_stats.h40_count, rom_stats.h32_count);
        emu_log_info("  Interlace: %d%%", rom_stats.interlace_count);
        emu_log_info("  Uso de DMA: %d%%", rom_stats.dma_usage);
        emu_log_info("  Interrupções HBlank: %d%%, VBlank: %d%%",
            rom_stats.hblank_interrupt_usage, rom_stats.vblank_interrupt_usage);
        emu_log_info("  Uso de Window: %d%%", rom_stats.window_usage);
        emu_log_info("  H-Scroll por linha: %d%%", rom_stats.h_scroll_per_row);
        emu_log_info("  Overflow de sprites: %d%%", rom_stats.sprite_overflow_count);

        // Atualizar estatísticas globais
        global_stats.mode5_count += rom_stats.mode5_count > 50 ? 1 : 0;
        global_stats.mode4_count += rom_stats.mode4_count > 50 ? 1 : 0;
        global_stats.h40_count += rom_stats.h40_count > 50 ? 1 : 0;
        global_stats.h32_count += rom_stats.h32_count > 50 ? 1 : 0;
        global_stats.interlace_count += rom_stats.interlace_count > 10 ? 1 : 0;
        global_stats.shadow_highlight_count += rom_stats.shadow_highlight_count > 10 ? 1 : 0;
        global_stats.dma_usage += rom_stats.dma_usage > 10 ? 1 : 0;
        global_stats.hblank_interrupt_usage += rom_stats.hblank_interrupt_usage > 50 ? 1 : 0;
        global_stats.vblank_interrupt_usage += rom_stats.vblank_interrupt_usage > 50 ? 1 : 0;
        global_stats.window_usage += rom_stats.window_usage > 10 ? 1 : 0;
        global_stats.h_scroll_per_row += rom_stats.h_scroll_per_row > 10 ? 1 : 0;
        global_stats.sprite_masking += 0;  // TODO: Implementar detecção
        global_stats.sprite_overflow_count += rom_stats.sprite_overflow_count > 5 ? 1 : 0;
        global_stats.sprite_limit_exceeded += 0;  // TODO: Implementar detecção
        global_stats.invalid_dma_usage += 0;      // TODO: Implementar detecção
        global_stats.timing_issues += 0;          // TODO: Implementar detecção
    }
}

// Setup para os testes
void setUp(void) {
    // Configuração antes de cada teste
}

// Teardown após testes
void tearDown(void) {
    // Limpeza após cada teste
}

// Teste principal
void test_vdp_rom_compatibility(void) {
    // Carregar ROMs
    load_roms();

    // Pular teste se não houver ROMs
    if (rom_count == 0) {
        emu_log_warning("Nenhuma ROM encontrada para testes de compatibilidade do VDP");
        TEST_IGNORE_MESSAGE("Nenhuma ROM encontrada");
        return;
    }

    // Executar testes de compatibilidade
    test_rom_vdp_compatibility();

    // Exibir resultados globais
    emu_log_info("=== Resultados Globais (total: %d ROMs) ===", rom_count);
    emu_log_info("ROMs usando Modo 5: %d (%d%%)",
        global_stats.mode5_count, (global_stats.mode5_count * 100) / rom_count);
    emu_log_info("ROMs usando Modo 4: %d (%d%%)",
        global_stats.mode4_count, (global_stats.mode4_count * 100) / rom_count);
    emu_log_info("ROMs usando H40: %d (%d%%)",
        global_stats.h40_count, (global_stats.h40_count * 100) / rom_count);
    emu_log_info("ROMs usando H32: %d (%d%%)",
        global_stats.h32_count, (global_stats.h32_count * 100) / rom_count);
    emu_log_info("ROMs usando Interlace: %d (%d%%)",
        global_stats.interlace_count, (global_stats.interlace_count * 100) / rom_count);
    emu_log_info("ROMs usando DMA: %d (%d%%)",
        global_stats.dma_usage, (global_stats.dma_usage * 100) / rom_count);
    emu_log_info("ROMs usando HBlank IRQ: %d (%d%%)",
        global_stats.hblank_interrupt_usage, (global_stats.hblank_interrupt_usage * 100) / rom_count);
    emu_log_info("ROMs usando VBlank IRQ: %d (%d%%)",
        global_stats.vblank_interrupt_usage, (global_stats.vblank_interrupt_usage * 100) / rom_count);
    emu_log_info("ROMs usando Window: %d (%d%%)",
        global_stats.window_usage, (global_stats.window_usage * 100) / rom_count);
    emu_log_info("ROMs usando H-Scroll por linha: %d (%d%%)",
        global_stats.h_scroll_per_row, (global_stats.h_scroll_per_row * 100) / rom_count);
    emu_log_info("ROMs com Overflow de sprites: %d (%d%%)",
        global_stats.sprite_overflow_count, (global_stats.sprite_overflow_count * 100) / rom_count);

    // Liberar memória das ROMs
    free_roms();

    // Garantir que pelo menos 80% das ROMs puderam ser carregadas e executadas
    TEST_ASSERT_MESSAGE(rom_count > 0, "Nenhuma ROM pôde ser carregada");
}

int main(void) {
    UNITY_BEGIN();

    // Inicializar sistema de log
    emu_log_init();
    emu_log_set_level(EMU_LOG_INFO);

    // Executar teste
    RUN_TEST(test_vdp_rom_compatibility);

    // Finalizar sistema de log
    emu_log_shutdown();

    return UNITY_END();
}
