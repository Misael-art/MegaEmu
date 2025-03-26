/**
 * @file rewind_buffer.c
 * @brief Implementação do sistema de rewind otimizado
 */

#include "save_state.h"
#include "../utils/enhanced_log.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Definição da categoria de log
#define LOG_CAT_REWIND EMU_LOG_CAT_CORE

// Estrutura para um snapshot no buffer de rewind
typedef struct
{
    void *data;            // Dados do snapshot
    uint32_t size;         // Tamanho dos dados
    uint64_t timestamp;    // Timestamp de quando foi capturado
    uint32_t frame_number; // Número do frame
    bool is_valid;         // Se o snapshot é válido
} rewind_snapshot_t;

// Estrutura para o buffer circular de rewind
typedef struct
{
    rewind_snapshot_t *snapshots; // Array de snapshots
    uint32_t capacity;            // Capacidade máxima
    uint32_t head;                // Posição atual para inserção
    uint32_t tail;                // Posição atual para leitura
    uint32_t count;               // Número de snapshots válidos
    uint32_t current_frame;       // Frame atual
    uint32_t frames_per_snapshot; // A cada quantos frames capturar
    bool initialized;             // Se o buffer está inicializado

    // Configurações de efeito visual
    save_state_rewind_effect_t effect_type; // Tipo de efeito
    float speed_multiplier;                 // Multiplicador de velocidade
    bool show_countdown;                    // Mostrar contagem regressiva
    bool show_progress_bar;                 // Mostrar barra de progresso
    bool skip_effect_button;                // Botão para pular o efeito

    // Estado do efeito de rewind
    bool effect_active;           // Se o efeito está ativo
    uint32_t effect_start_time;   // Timestamp de início do efeito
    uint32_t effect_duration_ms;  // Duração do efeito em ms
    uint32_t effect_target_frame; // Frame alvo do rewind
} rewind_buffer_t;

// Buffer global de rewind
static rewind_buffer_t g_rewind_buffer = {0};

/**
 * @brief Inicializa o buffer de rewind
 *
 * @param capacity Número máximo de snapshots
 * @param frames_per_snapshot A cada quantos frames capturar um snapshot
 * @return int32_t 0 em caso de sucesso, código de erro caso contrário
 */
int32_t rewind_buffer_init(uint32_t capacity, uint32_t frames_per_snapshot)
{
    // Verificar parâmetros
    if (capacity == 0 || frames_per_snapshot == 0)
    {
        EMU_LOG_ERROR(LOG_CAT_REWIND, "Parâmetros inválidos para inicialização do buffer de rewind");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Limitar a capacidade máxima
    if (capacity > SAVE_STATE_MAX_REWIND_STATES)
    {
        capacity = SAVE_STATE_MAX_REWIND_STATES;
        EMU_LOG_WARN(LOG_CAT_REWIND, "Capacidade de rewind limitada a %u snapshots", SAVE_STATE_MAX_REWIND_STATES);
    }

    // Liberar buffer anterior se existir
    if (g_rewind_buffer.snapshots)
    {
        for (uint32_t i = 0; i < g_rewind_buffer.capacity; i++)
        {
            if (g_rewind_buffer.snapshots[i].data)
            {
                free(g_rewind_buffer.snapshots[i].data);
            }
        }
        free(g_rewind_buffer.snapshots);
    }

    // Alocar novo buffer
    g_rewind_buffer.snapshots = (rewind_snapshot_t *)calloc(capacity, sizeof(rewind_snapshot_t));
    if (!g_rewind_buffer.snapshots)
    {
        EMU_LOG_ERROR(LOG_CAT_REWIND, "Falha ao alocar memória para buffer de rewind");
        return SAVE_STATE_ERROR_MEMORY;
    }

    // Inicializar estrutura
    g_rewind_buffer.capacity = capacity;
    g_rewind_buffer.head = 0;
    g_rewind_buffer.tail = 0;
    g_rewind_buffer.count = 0;
    g_rewind_buffer.current_frame = 0;
    g_rewind_buffer.frames_per_snapshot = frames_per_snapshot;
    g_rewind_buffer.initialized = true;

    // Configurações padrão para efeito visual
    g_rewind_buffer.effect_type = SAVE_STATE_REWIND_EFFECT_GREYSCALE;
    g_rewind_buffer.speed_multiplier = 0.5f;
    g_rewind_buffer.show_countdown = true;
    g_rewind_buffer.show_progress_bar = true;
    g_rewind_buffer.skip_effect_button = true;
    g_rewind_buffer.effect_active = false;

    EMU_LOG_INFO(LOG_CAT_REWIND, "Buffer de rewind inicializado com capacidade para %u snapshots, capturando a cada %u frames",
                 capacity, frames_per_snapshot);

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Finaliza o buffer de rewind
 */
void rewind_buffer_shutdown(void)
{
    if (!g_rewind_buffer.initialized)
    {
        return;
    }

    // Liberar memória dos snapshots
    if (g_rewind_buffer.snapshots)
    {
        for (uint32_t i = 0; i < g_rewind_buffer.capacity; i++)
        {
            if (g_rewind_buffer.snapshots[i].data)
            {
                free(g_rewind_buffer.snapshots[i].data);
                g_rewind_buffer.snapshots[i].data = NULL;
            }
        }
        free(g_rewind_buffer.snapshots);
        g_rewind_buffer.snapshots = NULL;
    }

    g_rewind_buffer.initialized = false;
    EMU_LOG_INFO(LOG_CAT_REWIND, "Buffer de rewind finalizado");
}

/**
 * @brief Configura o efeito visual para o rewind
 */
int32_t rewind_buffer_configure_effect(const save_state_rewind_effect_config_t *config)
{
    if (!g_rewind_buffer.initialized)
    {
        EMU_LOG_ERROR(LOG_CAT_REWIND, "Buffer de rewind não inicializado");
        return SAVE_STATE_ERROR_INVALID;
    }

    if (!config)
    {
        // Usar configuração padrão
        g_rewind_buffer.effect_type = SAVE_STATE_REWIND_EFFECT_GREYSCALE;
        g_rewind_buffer.speed_multiplier = 0.5f;
        g_rewind_buffer.show_countdown = true;
        g_rewind_buffer.show_progress_bar = true;
        g_rewind_buffer.skip_effect_button = true;
    }
    else
    {
        // Aplicar configuração fornecida
        g_rewind_buffer.effect_type = config->effect_type;

        // Validar multiplicador de velocidade (0.1 a 1.0)
        if (config->speed_multiplier < 0.1f || config->speed_multiplier > 1.0f)
        {
            g_rewind_buffer.speed_multiplier = 0.5f;
        }
        else
        {
            g_rewind_buffer.speed_multiplier = config->speed_multiplier;
        }

        g_rewind_buffer.show_countdown = config->show_countdown;
        g_rewind_buffer.show_progress_bar = config->show_progress_bar;
        g_rewind_buffer.skip_effect_button = config->skip_effect_button;
    }

    EMU_LOG_INFO(LOG_CAT_REWIND, "Efeito de rewind configurado: tipo=%d, velocidade=%.1f",
                 g_rewind_buffer.effect_type, g_rewind_buffer.speed_multiplier);

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Adiciona um snapshot ao buffer de rewind
 */
int32_t rewind_buffer_add_snapshot(const void *data, uint32_t size)
{
    if (!g_rewind_buffer.initialized || !data || size == 0)
    {
        EMU_LOG_ERROR(LOG_CAT_REWIND, "Parâmetros inválidos para adicionar snapshot");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Verificar se é momento de capturar um snapshot (a cada frames_per_snapshot frames)
    g_rewind_buffer.current_frame++;
    if ((g_rewind_buffer.current_frame % g_rewind_buffer.frames_per_snapshot) != 0)
    {
        // Não é momento de capturar
        return SAVE_STATE_ERROR_NONE;
    }

    // Liberar memória do snapshot antigo na posição atual
    if (g_rewind_buffer.snapshots[g_rewind_buffer.head].data)
    {
        free(g_rewind_buffer.snapshots[g_rewind_buffer.head].data);
    }

    // Alocar memória para o novo snapshot
    g_rewind_buffer.snapshots[g_rewind_buffer.head].data = malloc(size);
    if (!g_rewind_buffer.snapshots[g_rewind_buffer.head].data)
    {
        EMU_LOG_ERROR(LOG_CAT_REWIND, "Falha ao alocar memória para snapshot");
        g_rewind_buffer.snapshots[g_rewind_buffer.head].is_valid = false;
        return SAVE_STATE_ERROR_MEMORY;
    }

    // Copiar dados
    memcpy(g_rewind_buffer.snapshots[g_rewind_buffer.head].data, data, size);
    g_rewind_buffer.snapshots[g_rewind_buffer.head].size = size;
    g_rewind_buffer.snapshots[g_rewind_buffer.head].timestamp = (uint64_t)time(NULL);
    g_rewind_buffer.snapshots[g_rewind_buffer.head].frame_number = g_rewind_buffer.current_frame;
    g_rewind_buffer.snapshots[g_rewind_buffer.head].is_valid = true;

    // Atualizar head
    g_rewind_buffer.head = (g_rewind_buffer.head + 1) % g_rewind_buffer.capacity;

    // Atualizar contagem de snapshots
    if (g_rewind_buffer.count < g_rewind_buffer.capacity)
    {
        g_rewind_buffer.count++;
    }
    else
    {
        // Buffer cheio, atualizar tail
        g_rewind_buffer.tail = (g_rewind_buffer.tail + 1) % g_rewind_buffer.capacity;
    }

    EMU_LOG_DEBUG(LOG_CAT_REWIND, "Snapshot adicionado ao buffer de rewind: frame=%u, size=%u, count=%u",
                  g_rewind_buffer.current_frame, size, g_rewind_buffer.count);

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Retrocede um passo no buffer de rewind
 */
int32_t rewind_buffer_step_back(void **data, uint32_t *size)
{
    if (!g_rewind_buffer.initialized || !data || !size)
    {
        EMU_LOG_ERROR(LOG_CAT_REWIND, "Parâmetros inválidos para retroceder");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Verificar se o buffer está vazio
    if (g_rewind_buffer.count == 0)
    {
        EMU_LOG_WARN(LOG_CAT_REWIND, "Buffer de rewind vazio");
        return SAVE_STATE_ERROR_NO_SNAPSHOT;
    }

    // Calcular posição atual para leitura
    uint32_t current = (g_rewind_buffer.head + g_rewind_buffer.capacity - 1) % g_rewind_buffer.capacity;

    // Verificar se o snapshot é válido
    if (!g_rewind_buffer.snapshots[current].is_valid)
    {
        EMU_LOG_ERROR(LOG_CAT_REWIND, "Snapshot inválido na posição atual");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Retornar dados
    *data = g_rewind_buffer.snapshots[current].data;
    *size = g_rewind_buffer.snapshots[current].size;

    // Atualizar head
    g_rewind_buffer.head = current;
    g_rewind_buffer.count--;

    EMU_LOG_INFO(LOG_CAT_REWIND, "Retrocedendo para frame %u, restam %u snapshots",
                 g_rewind_buffer.snapshots[current].frame_number, g_rewind_buffer.count);

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Avança um passo no buffer de rewind (após retroceder)
 */
int32_t rewind_buffer_step_forward(void **data, uint32_t *size)
{
    if (!g_rewind_buffer.initialized || !data || !size)
    {
        EMU_LOG_ERROR(LOG_CAT_REWIND, "Parâmetros inválidos para avançar");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Calcular próxima posição
    uint32_t next = (g_rewind_buffer.head + 1) % g_rewind_buffer.capacity;

    // Verificar se o snapshot é válido
    if (!g_rewind_buffer.snapshots[next].is_valid)
    {
        EMU_LOG_WARN(LOG_CAT_REWIND, "Nenhum snapshot disponível para avançar");
        return SAVE_STATE_ERROR_NO_SNAPSHOT;
    }

    // Retornar dados
    *data = g_rewind_buffer.snapshots[next].data;
    *size = g_rewind_buffer.snapshots[next].size;

    // Atualizar head
    g_rewind_buffer.head = next;
    g_rewind_buffer.count++;

    EMU_LOG_INFO(LOG_CAT_REWIND, "Avançando para frame %u, total %u snapshots",
                 g_rewind_buffer.snapshots[next].frame_number, g_rewind_buffer.count);

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Inicia o efeito visual de rewind
 */
int32_t rewind_buffer_start_effect(uint32_t seconds_back)
{
    if (!g_rewind_buffer.initialized)
    {
        EMU_LOG_ERROR(LOG_CAT_REWIND, "Buffer de rewind não inicializado");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Verificar se já tem um efeito ativo
    if (g_rewind_buffer.effect_active)
    {
        EMU_LOG_WARN(LOG_CAT_REWIND, "Efeito de rewind já está ativo");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Verificar se o buffer tem snapshots suficientes
    if (g_rewind_buffer.count == 0)
    {
        EMU_LOG_WARN(LOG_CAT_REWIND, "Buffer de rewind vazio");
        return SAVE_STATE_ERROR_NO_SNAPSHOT;
    }

    // Limitar o número de segundos para voltar
    if (seconds_back > 10)
    {
        seconds_back = 10;
    }
    else if (seconds_back < 1)
    {
        seconds_back = 1;
    }

    // Calcular o número aproximado de snapshots para voltar
    // (considerando que cada snapshot representa frames_per_snapshot frames a 60fps)
    uint32_t snapshots_per_second = 60 / g_rewind_buffer.frames_per_snapshot;
    uint32_t snapshots_to_rewind = seconds_back * snapshots_per_second;

    // Limitar ao número de snapshots disponíveis
    if (snapshots_to_rewind > g_rewind_buffer.count)
    {
        snapshots_to_rewind = g_rewind_buffer.count;
    }

    // Calcular o frame alvo
    uint32_t current = g_rewind_buffer.head;
    for (uint32_t i = 0; i < snapshots_to_rewind; i++)
    {
        if (current == 0)
        {
            current = g_rewind_buffer.capacity - 1;
        }
        else
        {
            current--;
        }
    }

    // Armazenar informações do efeito
    g_rewind_buffer.effect_active = true;
    g_rewind_buffer.effect_start_time = (uint32_t)time(NULL) * 1000; // Em milissegundos

    // Calcular duração com base no multiplicador de velocidade
    // (mais lento = mais tempo no efeito)
    g_rewind_buffer.effect_duration_ms = (uint32_t)(seconds_back * 1000 / g_rewind_buffer.speed_multiplier);
    g_rewind_buffer.effect_target_frame = g_rewind_buffer.snapshots[current].frame_number;

    EMU_LOG_INFO(LOG_CAT_REWIND, "Efeito de rewind iniciado: voltando %u segundos para o frame %u",
                 seconds_back, g_rewind_buffer.effect_target_frame);

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Cancela o efeito visual de rewind
 */
int32_t rewind_buffer_cancel_effect(void)
{
    if (!g_rewind_buffer.initialized)
    {
        return SAVE_STATE_ERROR_INVALID;
    }

    if (!g_rewind_buffer.effect_active)
    {
        // Não há efeito ativo para cancelar
        return SAVE_STATE_ERROR_NONE;
    }

    g_rewind_buffer.effect_active = false;

    EMU_LOG_INFO(LOG_CAT_REWIND, "Efeito de rewind cancelado");

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Atualiza o efeito visual de rewind
 *
 * Esta função deve ser chamada a cada frame para atualizar o efeito visual.
 * Retorna true quando o efeito termina e é necessário aplicar o snapshot.
 */
bool rewind_buffer_update_effect(void **target_data, uint32_t *target_size)
{
    if (!g_rewind_buffer.initialized || !g_rewind_buffer.effect_active)
    {
        return false;
    }

    // Calcular tempo decorrido
    uint32_t current_time = (uint32_t)time(NULL) * 1000; // Em milissegundos
    uint32_t elapsed_time = current_time - g_rewind_buffer.effect_start_time;

    // Verificar se o efeito terminou
    if (elapsed_time >= g_rewind_buffer.effect_duration_ms)
    {
        // Efeito terminou, encontrar o snapshot correspondente ao frame alvo
        for (uint32_t i = 0; i < g_rewind_buffer.capacity; i++)
        {
            if (g_rewind_buffer.snapshots[i].is_valid &&
                g_rewind_buffer.snapshots[i].frame_number == g_rewind_buffer.effect_target_frame)
            {

                // Retornar dados do snapshot alvo
                if (target_data && target_size)
                {
                    *target_data = g_rewind_buffer.snapshots[i].data;
                    *target_size = g_rewind_buffer.snapshots[i].size;
                }

                // Atualizar head para a posição do snapshot alvo
                g_rewind_buffer.head = i;

                // Recalcular count
                if (i >= g_rewind_buffer.tail)
                {
                    g_rewind_buffer.count = i - g_rewind_buffer.tail + 1;
                }
                else
                {
                    g_rewind_buffer.count = g_rewind_buffer.capacity - g_rewind_buffer.tail + i + 1;
                }

                g_rewind_buffer.effect_active = false;

                EMU_LOG_INFO(LOG_CAT_REWIND, "Efeito de rewind concluído: aplicado snapshot do frame %u",
                             g_rewind_buffer.effect_target_frame);

                return true;
            }
        }

        // Snapshot alvo não encontrado
        EMU_LOG_ERROR(LOG_CAT_REWIND, "Snapshot alvo não encontrado: frame %u",
                      g_rewind_buffer.effect_target_frame);

        g_rewind_buffer.effect_active = false;
        return false;
    }

    // Efeito ainda em progresso
    return false;
}

/**
 * @brief Obtém o progresso atual do efeito de rewind
 */
float rewind_buffer_get_effect_progress(void)
{
    if (!g_rewind_buffer.initialized || !g_rewind_buffer.effect_active)
    {
        return 0.0f;
    }

    // Calcular tempo decorrido
    uint32_t current_time = (uint32_t)time(NULL) * 1000; // Em milissegundos
    uint32_t elapsed_time = current_time - g_rewind_buffer.effect_start_time;

    // Calcular progresso (0.0 a 1.0)
    return (float)elapsed_time / g_rewind_buffer.effect_duration_ms;
}

/**
 * @brief Verificar se o efeito de rewind está ativo
 */
bool rewind_buffer_is_effect_active(void)
{
    return g_rewind_buffer.initialized && g_rewind_buffer.effect_active;
}

/**
 * @brief Obter estatísticas do buffer de rewind
 */
void rewind_buffer_get_stats(uint32_t *capacity, uint32_t *count, uint32_t *frames_per_snapshot)
{
    if (capacity)
    {
        *capacity = g_rewind_buffer.capacity;
    }

    if (count)
    {
        *count = g_rewind_buffer.count;
    }

    if (frames_per_snapshot)
    {
        *frames_per_snapshot = g_rewind_buffer.frames_per_snapshot;
    }
}

/**
 * @brief Limpa todo o buffer de rewind
 */
void rewind_buffer_clear(void)
{
    if (!g_rewind_buffer.initialized)
    {
        return;
    }

    // Cancelar efeito se estiver ativo
    if (g_rewind_buffer.effect_active)
    {
        rewind_buffer_cancel_effect();
    }

    // Liberar memória dos snapshots
    for (uint32_t i = 0; i < g_rewind_buffer.capacity; i++)
    {
        if (g_rewind_buffer.snapshots[i].data)
        {
            free(g_rewind_buffer.snapshots[i].data);
            g_rewind_buffer.snapshots[i].data = NULL;
        }
        g_rewind_buffer.snapshots[i].is_valid = false;
    }

    // Resetar estado
    g_rewind_buffer.head = 0;
    g_rewind_buffer.tail = 0;
    g_rewind_buffer.count = 0;

    EMU_LOG_INFO(LOG_CAT_REWIND, "Buffer de rewind limpo");
}
