/**
 * @file vdp_dma.c
 * @brief Implementação das funcionalidades de DMA para o VDP do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vdp_dma.h"
#include "utils/log_utils.h"
#include "utils/validation_utils.h"

// Definições privadas
#define DMA_LOG_ENABLED 1
#define DMA_MAX_CYCLES_PER_WORD 16

// Estado global de DMA
static struct {
    bool active;
    uint32_t cycles_used;
    uint32_t words_transferred;
    uint16_t pending_length;
} dma_state;

/**
 * @brief Inicializa o controlador de DMA
 * @param dma Estrutura de DMA a ser inicializada
 */
void emu_vdp_dma_init(emu_vdp_dma_t* dma)
{
    CHECK_NULL_RETURN_VOID(dma, "DMA estrutura nula passada para inicialização");

    // Inicializa a estrutura DMA
    memset(dma, 0, sizeof(emu_vdp_dma_t));
    
    // Inicializa o estado global
    dma_state.active = false;
    dma_state.cycles_used = 0;
    dma_state.words_transferred = 0;
    dma_state.pending_length = 0;
    
    LOG_INFO("Subsistema VDP-DMA inicializado");
}

/**
 * @brief Inicia uma transferência de DMA
 * @param dma Estrutura de DMA
 * @param mode Modo de DMA (VRAM Fill, VRAM Copy, ou 68K to VRAM)
 */
void emu_vdp_dma_start_transfer(emu_vdp_dma_t* dma, emu_dma_mode_t mode)
{
    CHECK_NULL_RETURN_VOID(dma, "DMA estrutura nula passada para iniciar transferência");
    
    // Verificar parâmetros
    if (dma->length == 0 || dma->length > 0xFFFF) {
        LOG_ERROR("Comprimento de DMA inválido: %d", dma->length);
        return;
    }
    
    // Configurar a transferência
    dma->mode = mode;
    dma_state.active = true;
    dma_state.cycles_used = 0;
    dma_state.words_transferred = 0;
    dma_state.pending_length = dma->length;
    
    LOG_INFO("Iniciando transferência DMA modo %d: %d words de 0x%08X para 0x%04X",
             mode, dma->length, dma->source_addr, dma->destination);
}

/**
 * @brief Processa uma transferência de DMA em andamento
 * @param dma Estrutura de DMA
 * @param cycles Número de ciclos disponíveis para processamento
 * @return true se a transferência foi concluída, false se ainda está em andamento
 */
bool emu_vdp_dma_process(emu_vdp_dma_t* dma, int32_t cycles)
{
    CHECK_NULL_RETURN_VALUE(dma, false, "DMA estrutura nula passada para processamento");
    
    // Se não houver DMA ativo, retornar imediatamente
    if (!dma_state.active) {
        return true;
    }
    
    // Calcular quantas words podemos transferir com os ciclos disponíveis
    uint32_t max_words = cycles / DMA_MAX_CYCLES_PER_WORD;
    
    // Limitar ao número de words pendentes
    if (max_words > dma_state.pending_length) {
        max_words = dma_state.pending_length;
    }
    
    // Limitar a um máximo razoável por chamada
    const uint32_t MAX_WORDS_PER_CALL = 256;
    if (max_words > MAX_WORDS_PER_CALL) {
        max_words = MAX_WORDS_PER_CALL;
    }
    
    // Nada para fazer se não pudermos transferir nenhuma word
    if (max_words == 0) {
        return false;
    }
    
    // Realizar a transferência com base no modo
    uint32_t words_transferred = 0;
    
    switch (dma->mode) {
        case EMU_DMA_VRAM_FILL:
            // Operação de preenchimento: valor constante em múltiplos endereços
            // Neste caso, source_addr contém o valor de preenchimento
            words_transferred = _process_vram_fill_dma(dma, max_words);
            break;
            
        case EMU_DMA_VRAM_COPY:
            // Cópia de VRAM para VRAM
            words_transferred = _process_vram_copy_dma(dma, max_words);
            break;
            
        case EMU_DMA_VRAM_68K:
            // Transferência da memória da CPU para VRAM
            words_transferred = _process_68k_to_vram_dma(dma, max_words);
            break;
            
        default:
            LOG_ERROR("Modo de DMA desconhecido: %d", dma->mode);
            dma_state.active = false;
            return true;
    }
    
    // Atualizar estado
    dma_state.words_transferred += words_transferred;
    dma_state.pending_length -= words_transferred;
    dma_state.cycles_used += words_transferred * DMA_MAX_CYCLES_PER_WORD;
    
    // Verificar se a transferência foi concluída
    if (dma_state.pending_length == 0) {
        LOG_INFO("Transferência DMA concluída: %d words transferidas", 
                dma_state.words_transferred);
        
        dma_state.active = false;
        
        // Chamar callback de conclusão se estiver definido
        if (dma->dma_complete_callback) {
            dma->dma_complete_callback();
        }
        
        return true;
    }
    
    return false;
}

/**
 * @brief Implementação privada para processar DMA de preenchimento de VRAM
 * @param dma Estrutura de DMA
 * @param max_words Número máximo de words a transferir
 * @return Número de words efetivamente transferidas
 */
static uint32_t _process_vram_fill_dma(emu_vdp_dma_t* dma, uint32_t max_words)
{
    // NOTA: Esta é uma função stub - a implementação real requer
    // acesso ao subsistema de memória do VDP e não está incluída neste arquivo
    
    // Simulamos o preenchimento para fins de interface
    uint32_t destination = dma->destination;
    uint16_t fill_value = (uint16_t)dma->source_addr;
    
    LOG_DEBUG("DMA VRAM Fill: Preenchendo %d words em 0x%04X com valor 0x%04X",
              max_words, destination, fill_value);
    
    // Evento de log para cada 1024 words
    if ((dma_state.words_transferred + max_words) / 1024 > 
        dma_state.words_transferred / 1024) {
        LOG_INFO("DMA VRAM Fill: %d/%d words (%d%%)",
                dma_state.words_transferred + max_words,
                dma->length,
                (dma_state.words_transferred + max_words) * 100 / dma->length);
    }
    
    return max_words;
}

/**
 * @brief Implementação privada para processar DMA de cópia de VRAM
 * @param dma Estrutura de DMA
 * @param max_words Número máximo de words a transferir
 * @return Número de words efetivamente transferidas
 */
static uint32_t _process_vram_copy_dma(emu_vdp_dma_t* dma, uint32_t max_words)
{
    // NOTA: Esta é uma função stub - a implementação real requer
    // acesso ao subsistema de memória do VDP e não está incluída neste arquivo
    
    // Simulamos a cópia para fins de interface
    uint32_t source = dma->source_addr;
    uint32_t destination = dma->destination;
    
    LOG_DEBUG("DMA VRAM Copy: Copiando %d words de 0x%08X para 0x%04X",
              max_words, source, destination);
    
    // Evento de log para cada 1024 words
    if ((dma_state.words_transferred + max_words) / 1024 > 
        dma_state.words_transferred / 1024) {
        LOG_INFO("DMA VRAM Copy: %d/%d words (%d%%)",
                dma_state.words_transferred + max_words,
                dma->length,
                (dma_state.words_transferred + max_words) * 100 / dma->length);
    }
    
    return max_words;
}

/**
 * @brief Implementação privada para processar DMA da CPU (68K) para VRAM
 * @param dma Estrutura de DMA
 * @param max_words Número máximo de words a transferir
 * @return Número de words efetivamente transferidas
 */
static uint32_t _process_68k_to_vram_dma(emu_vdp_dma_t* dma, uint32_t max_words)
{
    // NOTA: Esta é uma função stub - a implementação real requer
    // acesso ao subsistema de memória do VDP e da CPU, não está incluída neste arquivo
    
    // Simulamos a transferência para fins de interface
    uint32_t source = dma->source_addr + (dma_state.words_transferred * 2);
    uint32_t destination = dma->destination + dma_state.words_transferred;
    
    LOG_DEBUG("DMA 68K to VRAM: Transferindo %d words de 0x%08X para 0x%04X",
              max_words, source, destination);
    
    // Evento de log para cada 1024 words
    if ((dma_state.words_transferred + max_words) / 1024 > 
        dma_state.words_transferred / 1024) {
        LOG_INFO("DMA 68K to VRAM: %d/%d words (%d%%)",
                dma_state.words_transferred + max_words,
                dma->length,
                (dma_state.words_transferred + max_words) * 100 / dma->length);
    }
    
    return max_words;
}

/**
 * @brief Verifica se um DMA está atualmente em andamento
 * @return true se houver um DMA ativo, false caso contrário
 */
bool emu_vdp_dma_is_active(void)
{
    return dma_state.active;
}

/**
 * @brief Obtém o progresso atual de uma operação de DMA
 * @param dma Estrutura de DMA
 * @param total_words Ponteiro para receber o total de words a transferir
 * @param completed_words Ponteiro para receber o número de words já transferidas
 * @return Porcentagem de conclusão (0-100)
 */
uint8_t emu_vdp_dma_get_progress(emu_vdp_dma_t* dma, uint32_t* total_words, uint32_t* completed_words)
{
    CHECK_NULL_RETURN_VALUE(dma, 0, "DMA estrutura nula passada para obter progresso");
    
    if (total_words) {
        *total_words = dma->length;
    }
    
    if (completed_words) {
        *completed_words = dma_state.words_transferred;
    }
    
    if (dma->length == 0) {
        return 100;
    }
    
    return (uint8_t)((dma_state.words_transferred * 100) / dma->length);
}

/**
 * @brief Aborta uma transferência de DMA em andamento
 * @param dma Estrutura de DMA
 */
void emu_vdp_dma_abort(emu_vdp_dma_t* dma)
{
    CHECK_NULL_RETURN_VOID(dma, "DMA estrutura nula passada para abortar");
    
    if (dma_state.active) {
        LOG_WARNING("Abortando transferência DMA: %d/%d words transferidas",
                   dma_state.words_transferred, dma->length);
                   
        dma_state.active = false;
    }
}
