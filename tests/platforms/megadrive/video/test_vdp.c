/**
 * @file test_vdp.c
 * @brief Testes unitários para o adaptador VDP do Mega Drive
 * @version 1.0
 * @date 2024-03-21
 */

#include "platforms/megadrive/video/vdp_adapter.h"
#include "test/test_framework.h"
#include <stdlib.h>
#include <string.h>

// Mocks e stubs
static uint8_t mock_memory[0x10000];
static uint8_t mock_memory_read(uint32_t addr, void *user_data) {
  (void)user_data; // Não utilizado
  return mock_memory[addr & 0xFFFF];
}

// Funções auxiliares
static void setup_vdp(emu_video_interface_t **video,
                      megadrive_vdp_context_t **context) {
  *video = megadrive_vdp_adapter_create();
  TEST_ASSERT_NOT_NULL(*video);

  *context = megadrive_vdp_get_context(*video);
  TEST_ASSERT_NOT_NULL(*context);

  // Configura callback de memória
  vdp_dma_set_memory_callback(*context, mock_memory_read, NULL);

  // Inicializa com configuração padrão
  emu_video_config_t config = {
      .width = 320, .height = 224, .buffer_format = EMU_VIDEO_FORMAT_INDEXED8};
  TEST_ASSERT_EQUAL(0, (*video)->init((*video)->context, &config));
}

static void teardown_vdp(emu_video_interface_t *video) {
  if (video) {
    megadrive_vdp_adapter_destroy(video);
  }
}

// Testes de inicialização
TEST_CASE(test_vdp_create) {
  emu_video_interface_t *video = NULL;
  megadrive_vdp_context_t *context = NULL;

  // Testa criação
  setup_vdp(&video, &context);
  TEST_ASSERT_NOT_NULL(video);
  TEST_ASSERT_NOT_NULL(context);

  // Verifica valores iniciais
  TEST_ASSERT_EQUAL(MD_VDP_MODE_H32_V28, context->mode);
  TEST_ASSERT_EQUAL(0, context->status);
  TEST_ASSERT_EQUAL(0, context->address);
  TEST_ASSERT_EQUAL(0, context->code);
  TEST_ASSERT_TRUE(context->first_byte);

  teardown_vdp(video);
}

// Testes de registradores
TEST_CASE(test_vdp_registers) {
  emu_video_interface_t *video = NULL;
  megadrive_vdp_context_t *context = NULL;
  setup_vdp(&video, &context);

  // Testa escrita/leitura de registradores
  vdp_write_register(context, 0x00, 0x04); // Modo normal
  TEST_ASSERT_EQUAL(0x04, vdp_read_register(context, 0x00));

  vdp_write_register(context, 0x01, 0x44); // Display on, V30
  TEST_ASSERT_EQUAL(0x44, vdp_read_register(context, 0x01));
  TEST_ASSERT_EQUAL(MD_VDP_MODE_H32_V30, context->mode);

  vdp_write_register(context, 0x0C, 0x81); // H40 mode
  TEST_ASSERT_EQUAL(0x81, vdp_read_register(context, 0x0C));
  TEST_ASSERT_EQUAL(MD_VDP_MODE_H40_V30, context->mode);

  teardown_vdp(video);
}

// Testes de DMA
TEST_CASE(test_vdp_dma) {
  emu_video_interface_t *video = NULL;
  megadrive_vdp_context_t *context = NULL;
  setup_vdp(&video, &context);

  // Prepara dados de teste
  for (int i = 0; i < 256; i++) {
    mock_memory[i] = i;
  }

  // Testa DMA fill
  vdp_dma_start(context, 0x80, 0x0000, 0xAA, 128);
  TEST_ASSERT_TRUE(vdp_dma_is_active(context));
  vdp_dma_execute(context);
  TEST_ASSERT_FALSE(vdp_dma_is_active(context));

  for (int i = 0; i < 128; i++) {
    TEST_ASSERT_EQUAL(0xAA, context->vram[i]);
  }

  // Testa DMA copy
  memset(context->vram, 0, MD_VDP_VRAM_SIZE);
  for (int i = 0; i < 128; i++) {
    context->vram[i] = i;
  }

  vdp_dma_start(context, 0xC0, 0x0200, 0x0000, 128);
  TEST_ASSERT_TRUE(vdp_dma_is_active(context));
  vdp_dma_execute(context);
  TEST_ASSERT_FALSE(vdp_dma_is_active(context));

  for (int i = 0; i < 128; i++) {
    TEST_ASSERT_EQUAL(i, context->vram[0x200 + i]);
  }

  // Testa DMA da memória para VRAM
  memset(context->vram, 0, MD_VDP_VRAM_SIZE);
  vdp_dma_start(context, 0x40, 0x0000, 0x0000, 128);
  TEST_ASSERT_TRUE(vdp_dma_is_active(context));
  vdp_dma_execute(context);
  TEST_ASSERT_FALSE(vdp_dma_is_active(context));

  for (int i = 0; i < 128; i++) {
    TEST_ASSERT_EQUAL(mock_memory[i], context->vram[i]);
  }

  teardown_vdp(video);
}

// Testes de renderização
TEST_CASE(test_vdp_render) {
  emu_video_interface_t *video = NULL;
  megadrive_vdp_context_t *context = NULL;
  setup_vdp(&video, &context);

  // Configura plano A
  vdp_write_register(context, 0x02, 0x38); // Plano A em 0xE000
  vdp_write_register(context, 0x10, 0x01); // 32x32 tiles

  // Configura um tile de teste
  uint8_t test_tile[] = {
      0x11, 0x11, 0x11, 0x11, // Linha 0: todos pixels = 1
      0x22, 0x22, 0x22, 0x22, // Linha 1: todos pixels = 2
      0x33, 0x33, 0x33, 0x33, // Linha 2: todos pixels = 3
      0x44, 0x44, 0x44, 0x44, // Linha 3: todos pixels = 4
      0x55, 0x55, 0x55, 0x55, // Linha 4: todos pixels = 5
      0x66, 0x66, 0x66, 0x66, // Linha 5: todos pixels = 6
      0x77, 0x77, 0x77, 0x77, // Linha 6: todos pixels = 7
      0x88, 0x88, 0x88, 0x88  // Linha 7: todos pixels = 8
  };

  // Copia tile para VRAM
  memcpy(&context->vram[0], test_tile, sizeof(test_tile));

  // Configura mapeamento do tile
  context->vram[0xE000] = 0x00; // Tile 0
  context->vram[0xE001] = 0x00; // Paleta 0, sem flip

  // Buffer de saída
  uint8_t output[320];

  // Renderiza cada linha do tile
  for (int line = 0; line < 8; line++) {
    vdp_render_line(context, line, output);

    // Verifica pixels da linha
    for (int x = 0; x < 8; x++) {
      TEST_ASSERT_EQUAL(line + 1, output[x]);
    }
  }

  teardown_vdp(video);
}

// Testes de planos
TEST_CASE(test_vdp_planes) {
  emu_video_interface_t *video = NULL;
  megadrive_vdp_context_t *context = NULL;
  setup_vdp(&video, &context);

  // Configura planos
  vdp_write_register(context, 0x02, 0x38); // Plano A em 0xE000
  vdp_write_register(context, 0x04, 0x07); // Plano B em 0x2000
  vdp_write_register(context, 0x10, 0x02); // 64x32 tiles

  // Verifica configuração
  uint16_t base_addr;
  uint8_t width, height;

  vdp_get_plane_info(context, 0, &base_addr, &width, &height);
  TEST_ASSERT_EQUAL(0xE000, base_addr);
  TEST_ASSERT_EQUAL(64, width);
  TEST_ASSERT_EQUAL(32, height);

  vdp_get_plane_info(context, 1, &base_addr, &width, &height);
  TEST_ASSERT_EQUAL(0x2000, base_addr);
  TEST_ASSERT_EQUAL(64, width);
  TEST_ASSERT_EQUAL(32, height);

  teardown_vdp(video);
}

// Testes de sprites
TEST_CASE(test_vdp_sprites) {
  emu_video_interface_t *video = NULL;
  megadrive_vdp_context_t *context = NULL;
  setup_vdp(&video, &context);

  // Configura sprites
  vdp_write_register(context, 0x05, 0x7F); // Tabela de sprites em 0xFC00

  // Verifica configuração
  uint16_t table_addr;
  uint8_t max_sprites;

  vdp_get_sprite_info(context, &table_addr, &max_sprites);
  TEST_ASSERT_EQUAL(0xFC00, table_addr);
  TEST_ASSERT_EQUAL(64, max_sprites); // Modo H32

  // Muda para modo H40
  vdp_write_register(context, 0x0C, 0x81);
  vdp_get_sprite_info(context, &table_addr, &max_sprites);
  TEST_ASSERT_EQUAL(80, max_sprites); // Modo H40

  teardown_vdp(video);
}

// Testes de scroll
TEST_CASE(test_vdp_scroll) {
  emu_video_interface_t *video = NULL;
  megadrive_vdp_context_t *context = NULL;
  setup_vdp(&video, &context);

  // Configura scroll
  vdp_write_register(context, 0x0D, 0x3F); // Base do scroll em 0xFC00
  vdp_write_register(context, 0x0B, 0x03); // Scroll por linha

  // Verifica configuração
  uint16_t hscroll_addr;
  uint8_t hscroll_mode, vscroll_mode;

  vdp_get_scroll_info(context, &hscroll_addr, &hscroll_mode, &vscroll_mode);
  TEST_ASSERT_EQUAL(0xFC00, hscroll_addr);
  TEST_ASSERT_EQUAL(3, hscroll_mode);
  TEST_ASSERT_EQUAL(0, vscroll_mode);

  teardown_vdp(video);
}

// Executa todos os testes
int main(void) {
  TEST_INIT();

  RUN_TEST(test_vdp_create);
  RUN_TEST(test_vdp_registers);
  RUN_TEST(test_vdp_dma);
  RUN_TEST(test_vdp_render);
  RUN_TEST(test_vdp_planes);
  RUN_TEST(test_vdp_sprites);
  RUN_TEST(test_vdp_scroll);

  TEST_REPORT();
  return 0;
}
