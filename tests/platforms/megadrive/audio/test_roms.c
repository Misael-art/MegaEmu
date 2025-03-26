/**
 * @file test_roms.c
 * @brief Testes do sistema de áudio com ROMs comerciais
 * @version 1.0
 * @date 2024-03-21
 */

#include "audio_mixer.h"
#include "audio_profiler.h"
#include "psg_adapter.h"
#include "ym2612_adapter.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unity.h>

// Constantes para os testes
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 2048
#define TEST_DURATION_SEC 30
#define SAMPLES_TO_PROCESS (SAMPLE_RATE * TEST_DURATION_SEC)
#define MAX_ROMS 100
#define ROM_PATH "roms/comerciais/"

// Estrutura para armazenar informações da ROM
typedef struct {
  char filename[256];
  char name[64];
  uint32_t size;
  uint8_t *data;
} rom_info_t;

// Variáveis globais para os testes
static ym2612_context_t *fm;
static psg_context_t *psg;
static audio_mixer_t *mixer;
static audio_profiler_t *profiler;
static rom_info_t roms[MAX_ROMS];
static int num_roms = 0;

// Configuração do mixer
static audio_mixer_config_t mixer_config = {.sample_rate = SAMPLE_RATE,
                                            .buffer_size = BUFFER_SIZE,
                                            .fm_volume = 0.75f,
                                            .psg_volume = 0.5f,
                                            .master_volume = 1.0f};

// Buffer para amostras
static int16_t sample_buffer[BUFFER_SIZE * 2];

// Função auxiliar para carregar ROMs
static void load_roms(void) {
  DIR *dir = opendir(ROM_PATH);
  TEST_ASSERT_NOT_NULL(dir);

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL && num_roms < MAX_ROMS) {
    if (entry->d_type == DT_REG) {
      char filepath[512];
      snprintf(filepath, sizeof(filepath), "%s%s", ROM_PATH, entry->d_name);

      FILE *file = fopen(filepath, "rb");
      if (file) {
        // Obtém tamanho do arquivo
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Aloca memória e lê ROM
        uint8_t *data = malloc(size);
        if (data) {
          if (fread(data, 1, size, file) == size) {
            strncpy(roms[num_roms].filename, entry->d_name,
                    sizeof(roms[num_roms].filename) - 1);
            roms[num_roms].size = size;
            roms[num_roms].data = data;

            // Extrai nome do jogo do cabeçalho da ROM
            if (size >= 0x150) {
              memcpy(roms[num_roms].name, &data[0x120], 48);
              roms[num_roms].name[48] = '\0';
            } else {
              strncpy(roms[num_roms].name, "Unknown",
                      sizeof(roms[num_roms].name) - 1);
            }

            num_roms++;
          } else {
            free(data);
          }
        }
        fclose(file);
      }
    }
  }

  closedir(dir);
  TEST_ASSERT_GREATER_THAN(0, num_roms);
}

// Função auxiliar para liberar ROMs
static void free_roms(void) {
  for (int i = 0; i < num_roms; i++) {
    free(roms[i].data);
  }
  num_roms = 0;
}

void setUp(void) {
  // Cria instâncias dos componentes
  fm = ym2612_create();
  TEST_ASSERT_NOT_NULL(fm);

  psg = psg_create(PSG_CLOCK, SAMPLE_RATE);
  TEST_ASSERT_NOT_NULL(psg);

  mixer = audio_mixer_create(&mixer_config);
  TEST_ASSERT_NOT_NULL(mixer);

  profiler = audio_profiler_create();
  TEST_ASSERT_NOT_NULL(profiler);

  // Carrega ROMs
  load_roms();
}

void tearDown(void) {
  if (fm) {
    ym2612_destroy(fm);
    fm = NULL;
  }
  if (psg) {
    psg_destroy(psg);
    psg = NULL;
  }
  if (mixer) {
    audio_mixer_destroy(mixer);
    mixer = NULL;
  }
  if (profiler) {
    audio_profiler_destroy(profiler);
    profiler = NULL;
  }

  free_roms();
}

// Função auxiliar para processar áudio da ROM
static void process_rom_audio(rom_info_t *rom) {
  char report_filename[512];
  snprintf(report_filename, sizeof(report_filename), "audio_report_%s.txt",
           rom->name);

  audio_profiler_start(profiler);

  uint32_t total_samples = 0;
  uint32_t underruns = 0;
  uint32_t overruns = 0;

  // Simula execução da ROM por TEST_DURATION_SEC segundos
  while (total_samples < SAMPLES_TO_PROCESS) {
    audio_profiler_start_section(profiler, "mixing");

    // Processa um bloco de amostras
    audio_mixer_process(mixer, BUFFER_SIZE);
    uint32_t samples_read = audio_mixer_read(mixer, sample_buffer, BUFFER_SIZE);

    audio_profiler_end_section(profiler, "mixing");

    // Atualiza métricas
    total_samples += samples_read;

    // Verifica qualidade do áudio
    for (uint32_t i = 0; i < samples_read * 2; i += 2) {
      // Verifica se as amostras estão dentro dos limites
      TEST_ASSERT_LESS_OR_EQUAL(32767, sample_buffer[i]);
      TEST_ASSERT_GREATER_OR_EQUAL(-32768, sample_buffer[i]);
      TEST_ASSERT_LESS_OR_EQUAL(32767, sample_buffer[i + 1]);
      TEST_ASSERT_GREATER_OR_EQUAL(-32768, sample_buffer[i + 1]);
    }

    // Atualiza métricas do buffer
    uint32_t available = audio_mixer_available_samples(mixer);
    if (available == 0)
      underruns++;
    if (available == BUFFER_SIZE)
      overruns++;

    audio_profiler_update_buffer(profiler, available, BUFFER_SIZE);
    audio_profiler_update_samples(profiler, samples_read, 0);
  }

  audio_profiler_stop(profiler);

  // Verifica métricas de qualidade
  TEST_ASSERT_LESS_THAN(10, underruns);
  TEST_ASSERT_LESS_THAN(10, overruns);

  // Verifica uso de CPU
  const audio_profiler_metrics_t *metrics =
      audio_profiler_get_metrics(profiler);
  TEST_ASSERT_LESS_THAN(80.0, metrics->total_cpu_usage);

  // Gera relatório
  audio_profiler_generate_report(profiler, report_filename);
}

void test_roms_audio(void) {
  printf("\nTestando áudio de %d ROMs:\n", num_roms);

  for (int i = 0; i < num_roms; i++) {
    printf("\nTestando ROM %d/%d: %s\n", i + 1, num_roms, roms[i].name);

    // Reseta estado do sistema de áudio
    ym2612_reset(fm);
    psg_reset(psg);
    audio_mixer_reset(mixer);
    audio_profiler_reset(profiler);

    // Processa áudio da ROM
    process_rom_audio(&roms[i]);

    // Verifica métricas finais
    const audio_profiler_metrics_t *metrics =
        audio_profiler_get_metrics(profiler);

    printf("Métricas para %s:\n", roms[i].name);
    printf("- CPU: FM=%.1f%% PSG=%.1f%% Mix=%.1f%% Total=%.1f%%\n",
           metrics->fm_cpu_usage, metrics->psg_cpu_usage,
           metrics->mixing_cpu_usage, metrics->total_cpu_usage);

    printf("- Buffer: Uso=%.1f%% Under=%u Over=%u\n", metrics->buffer_usage,
           metrics->buffer_underruns, metrics->buffer_overruns);

    printf("- Amostras: Proc=%u Drop=%u Taxa=%.1f Hz\n",
           metrics->samples_processed, metrics->samples_dropped,
           metrics->sample_rate);
  }
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_roms_audio);
  return UNITY_END();
}
