/**
 * @file test_vdp_sprites.c
 * @brief Testes unitários para o sistema de sprites do VDP do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <unity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../../../src/platforms/megadrive/video/vdp.h"
#include "../../../../src/platforms/megadrive/video/vdp_sprites.c" // Incluindo diretamente para testar funções estáticas

// Mocks e stubs necessários
static uint8_t mock_vram[VRAM_SIZE];
static uint16_t mock_cram[CRAM_SIZE];
static uint32_t mock_framebuffer[320 * 240];

void setUp(void)
{
    // Limpar áreas de memória
    memset(mock_vram, 0, sizeof(mock_vram));
    memset(mock_cram, 0, sizeof(mock_cram));
    memset(mock_framebuffer, 0, sizeof(mock_framebuffer));

    // Inicializar o sistema de sprites
    md_vdp_sprite_init();
}

void tearDown(void)
{
    // Nada a fazer aqui por enquanto
}

static void create_test_sprite(int index, int16_t x, int16_t y,
                               uint8_t size_h, uint8_t size_v,
                               uint8_t link, uint8_t palette,
                               uint8_t priority, uint16_t pattern)
{
    uint16_t offset = SPRITE_TABLE_ADDR + (index * 8);

    // Ajustar coordenadas (adicionar 128)
    x += 128;
    y += 128;

    // Escrever dados do sprite na VRAM
    mock_vram[offset] = (y >> 8) & 0xFF;
    mock_vram[offset + 1] = y & 0xFF;

    // Tamanho e link
    mock_vram[offset + 2] = ((link & 0x7F) << 4) | ((size_h & 0x03) << 2) | (size_v & 0x03);

    // Atributos (paleta, prioridade, flip)
    mock_vram[offset + 3] = ((priority & 0x01) << 7) | ((palette & 0x03) << 5);

    // Coordenada X
    mock_vram[offset + 4] = (x >> 8) & 0xFF;
    mock_vram[offset + 5] = x & 0xFF;

    // Índice do padrão
    mock_vram[offset + 6] = (pattern >> 8) & 0xFF;
    mock_vram[offset + 7] = pattern & 0xFF;
}

void test_sprite_init(void)
{
    // Inicializar com valores conhecidos
    sprite_system.sprite_count = 10;
    sprite_system.sprite_overflow = 1;
    sprite_system.sprite_collision = 1;

    // Chamar inicialização
    md_vdp_sprite_init();

    // Verificar se os valores foram resetados
    TEST_ASSERT_EQUAL_INT(0, sprite_system.sprite_count);
    TEST_ASSERT_EQUAL_INT(0, sprite_system.sprite_overflow);
    TEST_ASSERT_EQUAL_INT(0, sprite_system.sprite_collision);

    // Verificar se a tabela de sprites está limpa
    for (int i = 0; i < MAX_SPRITES; i++)
    {
        TEST_ASSERT_EQUAL_INT(0, sprite_system.sprites[i].x);
        TEST_ASSERT_EQUAL_INT(0, sprite_system.sprites[i].y);
        TEST_ASSERT_EQUAL_INT(0, sprite_system.sprites[i].size_h);
        TEST_ASSERT_EQUAL_INT(0, sprite_system.sprites[i].size_v);
    }
}

void test_sprite_reset(void)
{
    // Criar alguns sprites de teste
    create_test_sprite(0, 10, 20, 1, 1, 1, 0, 0, 0x100);
    create_test_sprite(1, 30, 40, 2, 2, 2, 1, 1, 0x200);

    // Processar sprites
    md_vdp_sprite_process_table();

    // Verificar se os sprites foram carregados
    TEST_ASSERT_EQUAL_INT(2, sprite_system.sprite_count);

    // Resetar sistema
    md_vdp_sprite_reset();

    // Verificar se tudo foi limpo
    TEST_ASSERT_EQUAL_INT(0, sprite_system.sprite_count);
    TEST_ASSERT_EQUAL_INT(0, sprite_system.sprite_overflow);
    TEST_ASSERT_EQUAL_INT(0, sprite_system.sprite_collision);

    // Verificar se a tabela de sprites está limpa
    for (int i = 0; i < MAX_SPRITES; i++)
    {
        TEST_ASSERT_EQUAL_INT(0, sprite_system.sprites[i].x);
        TEST_ASSERT_EQUAL_INT(0, sprite_system.sprites[i].y);
        TEST_ASSERT_EQUAL_INT(0, sprite_system.sprites[i].size_h);
        TEST_ASSERT_EQUAL_INT(0, sprite_system.sprites[i].size_v);
    }
}

void test_parse_sprite_table(void)
{
    // Criar alguns sprites de teste com diferentes configurações
    create_test_sprite(0, 10, 20, 1, 1, 1, 0, 0, 0x100); // 8x8 sprite
    create_test_sprite(1, 30, 40, 2, 2, 2, 1, 1, 0x200); // 16x16 sprite
    create_test_sprite(2, 50, 60, 3, 3, 3, 2, 0, 0x300); // 24x24 sprite

    // Processar tabela de sprites
    md_vdp_sprite_process_table();

    // Verificar se os sprites foram carregados corretamente
    TEST_ASSERT_EQUAL_INT(3, sprite_system.sprite_count);

    // Verificar primeiro sprite
    TEST_ASSERT_EQUAL_INT(10, sprite_system.sprites[0].x);
    TEST_ASSERT_EQUAL_INT(20, sprite_system.sprites[0].y);
    TEST_ASSERT_EQUAL_INT(1, sprite_system.sprites[0].size_h);
    TEST_ASSERT_EQUAL_INT(1, sprite_system.sprites[0].size_v);
    TEST_ASSERT_EQUAL_INT(0x100, sprite_system.sprites[0].pattern);

    // Verificar segundo sprite
    TEST_ASSERT_EQUAL_INT(30, sprite_system.sprites[1].x);
    TEST_ASSERT_EQUAL_INT(40, sprite_system.sprites[1].y);
    TEST_ASSERT_EQUAL_INT(2, sprite_system.sprites[1].size_h);
    TEST_ASSERT_EQUAL_INT(2, sprite_system.sprites[1].size_v);
    TEST_ASSERT_EQUAL_INT(0x200, sprite_system.sprites[1].pattern);

    // Verificar terceiro sprite
    TEST_ASSERT_EQUAL_INT(50, sprite_system.sprites[2].x);
    TEST_ASSERT_EQUAL_INT(60, sprite_system.sprites[2].y);
    TEST_ASSERT_EQUAL_INT(3, sprite_system.sprites[2].size_h);
    TEST_ASSERT_EQUAL_INT(3, sprite_system.sprites[2].size_v);
    TEST_ASSERT_EQUAL_INT(0x300, sprite_system.sprites[2].pattern);
}

void test_build_sprite_line_cache(void)
{
    // Criar sprites que se sobrepõem em uma linha específica
    create_test_sprite(0, 10, 50, 1, 1, 1, 0, 0, 0x100); // Sprite na linha 50
    create_test_sprite(1, 20, 50, 1, 1, 2, 1, 1, 0x200); // Sprite na mesma linha
    create_test_sprite(2, 30, 51, 1, 1, 3, 2, 0, 0x300); // Sprite na linha seguinte

    // Processar tabela de sprites
    md_vdp_sprite_process_table();

    // Construir cache para linha 50
    md_vdp_sprite_build_line_cache(50);

    // Verificar sprites na linha
    TEST_ASSERT_EQUAL_INT(2, sprite_system.line_sprite_count);

    // Verificar ordem dos sprites (por prioridade)
    TEST_ASSERT_EQUAL_INT(1, sprite_system.line_sprites[0].priority);
    TEST_ASSERT_EQUAL_INT(0, sprite_system.line_sprites[1].priority);

    // Verificar posições X
    TEST_ASSERT_EQUAL_INT(20, sprite_system.line_sprites[0].x);
    TEST_ASSERT_EQUAL_INT(10, sprite_system.line_sprites[1].x);
}

void test_sprite_collision(void)
{
    // Criar sprites que se sobrepõem
    create_test_sprite(0, 10, 20, 1, 1, 1, 0, 0, 0x100); // 8x8 em (10,20)
    create_test_sprite(1, 15, 20, 1, 1, 2, 1, 1, 0x200); // 8x8 em (15,20)

    // Processar tabela de sprites
    md_vdp_sprite_process_table();

    // Construir cache para linha 20
    md_vdp_sprite_build_line_cache(20);

    // Verificar colisão
    TEST_ASSERT_TRUE(md_vdp_sprite_check_collision(10, 20, 8, 8, 15, 20, 8, 8));

    // Verificar não-colisão
    TEST_ASSERT_FALSE(md_vdp_sprite_check_collision(10, 20, 8, 8, 50, 50, 8, 8));

    // Verificar flag de colisão
    TEST_ASSERT_EQUAL_INT(1, sprite_system.sprite_collision);
}

void test_sprite_rendering(void)
{
    // Criar um sprite simples
    create_test_sprite(0, 10, 20, 1, 1, 0, 0, 0, 0x100);

    // Adicionar padrão do sprite na VRAM
    for (int i = 0; i < 32; i++)
    {
        mock_vram[0x100 + i] = 0xFF; // Padrão sólido
    }

    // Configurar paleta
    mock_cram[16] = 0x0F00; // Vermelho

    // Processar tabela e renderizar
    md_vdp_sprite_process_table();
    md_vdp_sprite_render_line(20);

    // Verificar pixels renderizados
    for (int x = 10; x < 18; x++)
    {
        TEST_ASSERT_EQUAL_HEX32(0xFF0000FF, mock_framebuffer[20 * 320 + x]);
    }
}

void test_status_functions(void)
{
    // Configurar estado conhecido
    sprite_system.sprite_count = 5;
    sprite_system.sprite_overflow = 1;
    sprite_system.sprite_collision = 1;

    // Verificar funções de status
    TEST_ASSERT_EQUAL_INT(5, md_vdp_sprite_get_count());
    TEST_ASSERT_TRUE(md_vdp_sprite_get_overflow());
    TEST_ASSERT_TRUE(md_vdp_sprite_get_collision());

    // Limpar flags
    md_vdp_sprite_clear_overflow();
    md_vdp_sprite_clear_collision();

    // Verificar se flags foram limpas
    TEST_ASSERT_FALSE(md_vdp_sprite_get_overflow());
    TEST_ASSERT_FALSE(md_vdp_sprite_get_collision());
}

void test_debug_view(void)
{
    // Criar alguns sprites para debug
    create_test_sprite(0, 10, 20, 1, 1, 1, 0, 0, 0x100);
    create_test_sprite(1, 30, 40, 2, 2, 2, 1, 1, 0x200);

    // Processar sprites
    md_vdp_sprite_process_table();

    // Gerar visualização de debug
    char debug_buffer[1024];
    md_vdp_sprite_debug_view(debug_buffer, sizeof(debug_buffer));

    // Verificar se a string contém informações esperadas
    TEST_ASSERT_NOT_NULL(strstr(debug_buffer, "Sprite 0"));
    TEST_ASSERT_NOT_NULL(strstr(debug_buffer, "Sprite 1"));
    TEST_ASSERT_NOT_NULL(strstr(debug_buffer, "x: 10"));
    TEST_ASSERT_NOT_NULL(strstr(debug_buffer, "y: 20"));
    TEST_ASSERT_NOT_NULL(strstr(debug_buffer, "x: 30"));
    TEST_ASSERT_NOT_NULL(strstr(debug_buffer, "y: 40"));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_sprite_init);
    RUN_TEST(test_sprite_reset);
    RUN_TEST(test_parse_sprite_table);
    RUN_TEST(test_build_sprite_line_cache);
    RUN_TEST(test_sprite_collision);
    RUN_TEST(test_sprite_rendering);
    RUN_TEST(test_status_functions);
    RUN_TEST(test_debug_view);

    return UNITY_END();
}
