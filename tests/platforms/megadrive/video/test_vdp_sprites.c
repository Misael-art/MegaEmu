/**
 * @file test_vdp_sprites.c
 * @brief Testes unitários para o sistema de sprites do VDP do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "../../../../src/platforms/megadrive/video/vdp.h"
#include "../../../../src/platforms/megadrive/video/vdp_sprites.c" // Incluindo diretamente para testar funções estáticas
#include "../../../../src/utils/test_utils.h"

// Mocks e stubs necessários
static uint8_t mock_vram[VRAM_SIZE];
static uint16_t mock_cram[CRAM_SIZE];
static uint32_t mock_framebuffer[320 * 240];

/**
 * @brief Configura o ambiente para os testes
 */
static void setup(void)
{
    // Limpar áreas de memória
    memset(mock_vram, 0, sizeof(mock_vram));
    memset(mock_cram, 0, sizeof(mock_cram));
    memset(mock_framebuffer, 0, sizeof(mock_framebuffer));
    
    // Inicializar o sistema de sprites
    md_vdp_sprite_init();
}

/**
 * @brief Limpa o ambiente após os testes
 */
static void teardown(void)
{
    // Nada a fazer aqui por enquanto
}

/**
 * @brief Cria um sprite de teste na VRAM
 * @param index Índice do sprite na tabela
 * @param x Posição X
 * @param y Posição Y
 * @param size_h Tamanho horizontal (0-3)
 * @param size_v Tamanho vertical (0-3)
 * @param link Link para o próximo sprite
 * @param palette Paleta (0-3)
 * @param priority Prioridade (0-1)
 * @param pattern Índice do padrão
 */
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

/**
 * @brief Testa a inicialização do sistema de sprites
 */
static void test_sprite_init(void)
{
    printf("Executando teste: inicialização do sistema de sprites\n");
    
    // Inicializar com valores conhecidos
    sprite_system.sprite_count = 10;
    sprite_system.sprite_overflow = 1;
    sprite_system.sprite_collision = 1;
    
    // Chamar inicialização
    md_vdp_sprite_init();
    
    // Verificar se os valores foram resetados
    assert(sprite_system.sprite_count == 0);
    assert(sprite_system.sprite_overflow == 0);
    assert(sprite_system.sprite_collision == 0);
    
    printf("Teste de inicialização concluído com sucesso\n");
}

/**
 * @brief Testa o reset do sistema de sprites
 */
static void test_sprite_reset(void)
{
    printf("Executando teste: reset do sistema de sprites\n");
    
    // Configurar alguns sprites
    for (int i = 0; i < 5; i++) {
        sprite_system.sprites[i].x = 10 + i * 10;
        sprite_system.sprites[i].y = 20 + i * 10;
        sprite_system.sprites[i].width = 16;
        sprite_system.sprites[i].height = 16;
        sprite_system.sprites[i].tile_index = 100 + i;
        sprite_system.sprites[i].palette = i % 4;
        sprite_system.sprites[i].priority = i % 2;
        sprite_system.sprites[i].h_flip = i % 2;
        sprite_system.sprites[i].v_flip = (i + 1) % 2;
        sprite_system.sprites[i].link = i + 1;
    }
    
    sprite_system.sprite_count = 5;
    sprite_system.sprite_overflow = 1;
    sprite_system.sprite_collision = 1;
    sprite_system.cache_valid = 1;
    
    // Chamar reset
    md_vdp_sprite_reset();
    
    // Verificar se os valores foram resetados
    assert(sprite_system.sprite_count == 0);
    assert(sprite_system.sprite_overflow == 0);
    assert(sprite_system.sprite_collision == 0);
    assert(sprite_system.cache_valid == 0);
    
    // Verificar se os sprites foram resetados
    for (int i = 0; i < 5; i++) {
        assert(sprite_system.sprites[i].x == 0);
        assert(sprite_system.sprites[i].y == 0);
        assert(sprite_system.sprites[i].width == SPRITE_WIDTH_NORMAL);
        assert(sprite_system.sprites[i].height == SPRITE_HEIGHT_NORMAL);
        assert(sprite_system.sprites[i].tile_index == 0);
        assert(sprite_system.sprites[i].palette == 0);
        assert(sprite_system.sprites[i].priority == 0);
        assert(sprite_system.sprites[i].h_flip == 0);
        assert(sprite_system.sprites[i].v_flip == 0);
        assert(sprite_system.sprites[i].link == 0);
    }
    
    printf("Teste de reset concluído com sucesso\n");
}

/**
 * @brief Testa o parsing da tabela de sprites
 */
static void test_parse_sprite_table(void)
{
    printf("Executando teste: parsing da tabela de sprites\n");
    
    // Limpar VRAM
    memset(mock_vram, 0, sizeof(mock_vram));
    
    // Criar alguns sprites de teste
    create_test_sprite(0, 10, 20, 1, 1, 0, 2, 1, 100); // 16x16, paleta 2, prioridade 1
    create_test_sprite(1, 50, 60, 0, 0, 0, 1, 0, 200); // 8x8, paleta 1, prioridade 0
    create_test_sprite(2, 100, 120, 2, 1, 0, 3, 1, 300); // 32x16, paleta 3, prioridade 1
    
    // Chamar função de parsing (usando o endereço da tabela de sprites)
    _parse_sprite_table(mock_vram, SPRITE_TABLE_ADDR);
    
    // Verificar se os sprites foram processados corretamente
    assert(sprite_system.sprite_count == 1); // Apenas o primeiro sprite deve ser processado (link = 0)
    
    // Verificar dados do primeiro sprite
    assert(sprite_system.sprites[0].x == 10);
    assert(sprite_system.sprites[0].y == 20);
    assert(sprite_system.sprites[0].width == 16);
    assert(sprite_system.sprites[0].height == 16);
    assert(sprite_system.sprites[0].tile_index == 100);
    assert(sprite_system.sprites[0].palette == 2);
    assert(sprite_system.sprites[0].priority == 1);
    
    // Testar com links
    create_test_sprite(0, 10, 20, 1, 1, 2, 2, 1, 100); // Link para sprite 2
    create_test_sprite(1, 50, 60, 0, 0, 3, 1, 0, 200); // Link para sprite 3
    create_test_sprite(2, 100, 120, 2, 1, 0, 3, 1, 300); // Fim da cadeia
    
    // Chamar função de parsing novamente
    _parse_sprite_table(mock_vram, SPRITE_TABLE_ADDR);
    
    // Verificar se os sprites foram processados corretamente
    assert(sprite_system.sprite_count == 2); // Sprite 0 e 2 devem ser processados (seguindo o link)
    
    // Verificar dados do segundo sprite (índice 2 na VRAM, índice 1 no array)
    assert(sprite_system.sprites[1].x == 100);
    assert(sprite_system.sprites[1].y == 120);
    assert(sprite_system.sprites[1].width == 32);
    assert(sprite_system.sprites[1].height == 16);
    assert(sprite_system.sprites[1].tile_index == 300);
    assert(sprite_system.sprites[1].palette == 3);
    assert(sprite_system.sprites[1].priority == 1);
    
    printf("Teste de parsing da tabela de sprites concluído com sucesso\n");
}

/**
 * @brief Testa a construção do cache de sprites por linha
 */
static void test_build_sprite_line_cache(void)
{
    printf("Executando teste: construção do cache de sprites por linha\n");
    
    // Limpar estado
    memset(&sprite_system, 0, sizeof(md_sprite_system_t));
    
    // Criar alguns sprites manualmente
    sprite_system.sprites[0].x = 10;
    sprite_system.sprites[0].y = 20;
    sprite_system.sprites[0].width = 16;
    sprite_system.sprites[0].height = 16;
    
    sprite_system.sprites[1].x = 50;
    sprite_system.sprites[1].y = 20; // Mesma linha que o sprite 0
    sprite_system.sprites[1].width = 8;
    sprite_system.sprites[1].height = 8;
    
    sprite_system.sprites[2].x = 100;
    sprite_system.sprites[2].y = 100;
    sprite_system.sprites[2].width = 32;
    sprite_system.sprites[2].height = 32;
    
    sprite_system.sprite_count = 3;
    
    // Construir cache
    _build_sprite_line_cache();
    
    // Verificar se o cache foi construído corretamente
    assert(sprite_system.cache_valid == 1);
    
    // Verificar sprites na linha 20
    assert(sprite_system.sprites_per_line[20] == 2); // Dois sprites nesta linha
    assert(sprite_system.sprite_line_indices[20][0] == 0); // Sprite 0
    assert(sprite_system.sprite_line_indices[20][1] == 1); // Sprite 1
    
    // Verificar sprites na linha 100
    assert(sprite_system.sprites_per_line[100] == 1); // Um sprite nesta linha
    assert(sprite_system.sprite_line_indices[100][0] == 2); // Sprite 2
    
    // Verificar linha sem sprites
    assert(sprite_system.sprites_per_line[50] == 0); // Nenhum sprite nesta linha
    
    printf("Teste de construção do cache de sprites por linha concluído com sucesso\n");
}

/**
 * @brief Testa a detecção de colisão entre sprites
 */
static void test_sprite_collision(void)
{
    printf("Executando teste: detecção de colisão entre sprites\n");
    
    // Testar colisão entre sprites
    // Caso 1: Sprites que se sobrepõem
    assert(_check_sprite_collision(10, 10, 20, 20, 20, 20, 20, 20) == 1);
    
    // Caso 2: Sprites que se tocam nas bordas
    assert(_check_sprite_collision(10, 10, 10, 10, 20, 10, 10, 10) == 1);
    
    // Caso 3: Sprites que não se tocam
    assert(_check_sprite_collision(10, 10, 10, 10, 30, 30, 10, 10) == 0);
    
    // Caso 4: Um sprite dentro do outro
    assert(_check_sprite_collision(10, 10, 30, 30, 15, 15, 10, 10) == 1);
    
    // Configurar sprites para teste de colisão global
    memset(&sprite_system, 0, sizeof(md_sprite_system_t));
    
    // Dois sprites que colidem
    sprite_system.sprites[0].x = 10;
    sprite_system.sprites[0].y = 10;
    sprite_system.sprites[0].width = 20;
    sprite_system.sprites[0].height = 20;
    
    sprite_system.sprites[1].x = 20;
    sprite_system.sprites[1].y = 20;
    sprite_system.sprites[1].width = 20;
    sprite_system.sprites[1].height = 20;
    
    sprite_system.sprite_count = 2;
    
    // Verificar colisão
    assert(md_vdp_check_sprite_collisions() == 1);
    assert(sprite_system.sprite_collision == 1);
    
    // Modificar para que não haja colisão
    sprite_system.sprites[1].x = 40;
    sprite_system.sprites[1].y = 40;
    
    // Resetar flag de colisão
    sprite_system.sprite_collision = 0;
    
    // Verificar ausência de colisão
    assert(md_vdp_check_sprite_collisions() == 0);
    assert(sprite_system.sprite_collision == 0);
    
    printf("Teste de detecção de colisão entre sprites concluído com sucesso\n");
}

/**
 * @brief Testa a renderização de sprites
 */
static void test_sprite_rendering(void)
{
    printf("Executando teste: renderização de sprites\n");
    
    // Limpar estado
    memset(&sprite_system, 0, sizeof(md_sprite_system_t));
    memset(mock_framebuffer, 0, sizeof(mock_framebuffer));
    
    // Configurar alguns sprites
    create_test_sprite(0, 10, 20, 1, 1, 0, 2, 1, 100); // 16x16, paleta 2, prioridade 1
    
    // Configurar CRAM com algumas cores
    for (int i = 0; i < 64; i++) {
        mock_cram[i] = 0x0FFF; // Branco
    }
    
    // Renderizar sprites
    md_vdp_render_sprites(mock_framebuffer, 320, 240, mock_vram, mock_cram, SPRITE_TABLE_ADDR);
    
    // Verificar se o sprite foi processado
    assert(sprite_system.sprite_count == 1);
    
    // Verificar se o cache foi construído
    assert(sprite_system.cache_valid == 1);
    
    // Verificar se há pixels renderizados na área do sprite
    // Nota: Como a função real de renderização depende da VRAM e padrões,
    // não podemos verificar exatamente quais pixels foram renderizados,
    // mas podemos verificar se a função foi chamada corretamente.
    
    printf("Teste de renderização de sprites concluído\n");
}

/**
 * @brief Testa as funções de status
 */
static void test_status_functions(void)
{
    printf("Executando teste: funções de status\n");
    
    // Configurar estado conhecido
    sprite_system.sprite_count = 5;
    sprite_system.sprite_overflow = 1;
    sprite_system.sprite_collision = 1;
    
    // Verificar funções de status
    assert(md_vdp_get_sprite_count() == 5);
    assert(md_vdp_get_sprite_overflow() == 1);
    assert(md_vdp_get_sprite_collision() == 1);
    
    // Alterar estado
    sprite_system.sprite_count = 10;
    sprite_system.sprite_overflow = 0;
    sprite_system.sprite_collision = 0;
    
    // Verificar novamente
    assert(md_vdp_get_sprite_count() == 10);
    assert(md_vdp_get_sprite_overflow() == 0);
    assert(md_vdp_get_sprite_collision() == 0);
    
    printf("Teste de funções de status concluído com sucesso\n");
}

/**
 * @brief Testa a visualização de debug
 */
static void test_debug_view(void)
{
    printf("Executando teste: visualização de debug\n");
    
    // Limpar estado e framebuffer
    memset(&sprite_system, 0, sizeof(md_sprite_system_t));
    memset(mock_framebuffer, 0, sizeof(mock_framebuffer));
    
    // Configurar alguns sprites
    sprite_system.sprites[0].x = 10;
    sprite_system.sprites[0].y = 20;
    sprite_system.sprites[0].width = 16;
    sprite_system.sprites[0].height = 16;
    sprite_system.sprites[0].palette = 0;
    sprite_system.sprites[0].priority = 1;
    
    sprite_system.sprites[1].x = 50;
    sprite_system.sprites[1].y = 60;
    sprite_system.sprites[1].width = 8;
    sprite_system.sprites[1].height = 8;
    sprite_system.sprites[1].palette = 1;
    sprite_system.sprites[1].priority = 0;
    
    sprite_system.sprite_count = 2;
    sprite_system.sprite_overflow = 1;
    sprite_system.sprite_collision = 1;
    
    // Gerar visualização de debug
    md_vdp_generate_sprite_debug_view(mock_framebuffer, 320, 240);
    
    // Verificar se o framebuffer foi modificado
    int modified_pixels = 0;
    for (int i = 0; i < 320 * 240; i++) {
        if (mock_framebuffer[i] != 0) {
            modified_pixels++;
        }
    }
    
    // Deve haver pixels modificados para os sprites e informações de status
    assert(modified_pixels > 0);
    
    printf("Teste de visualização de debug concluído\n");
}

/**
 * @brief Função principal para execução dos testes
 */
int main(void)
{
    printf("Iniciando testes do sistema de sprites do VDP do Mega Drive\n");
    
    // Executar testes
    setup();
    test_sprite_init();
    teardown();
    
    setup();
    test_sprite_reset();
    teardown();
    
    setup();
    test_parse_sprite_table();
    teardown();
    
    setup();
    test_build_sprite_line_cache();
    teardown();
    
    setup();
    test_sprite_collision();
    teardown();
    
    setup();
    test_sprite_rendering();
    teardown();
    
    setup();
    test_status_functions();
    teardown();
    
    setup();
    test_debug_view();
    teardown();
    
    printf("Todos os testes do sistema de sprites concluídos com sucesso!\n");
    
    return 0;
}
