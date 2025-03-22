/**
 * @file test_event_priority.c
 * @brief Testes unitários para o sistema de fila de prioridade de eventos
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../deps/unity/src/unity.h"
#include "../../core/events/priority_queue.h"
#include "../../core/events/events_enhanced.h"
#include "../test_common.h"

// Variáveis para teste
static emu_event_priority_queue_t *g_queue = NULL;
static emu_events_enhanced_t g_events;

// Contadores para callbacks
static int g_callback_counter = 0;
static int g_event_types_received[EMU_EVENT_MAX] = {0};

void setUp(void)
{
    // Inicializar fila de prioridade para testes
    g_queue = emu_event_queue_create(16);
    TEST_ASSERT_NOT_NULL(g_queue);

    // Inicializar sistema de eventos aprimorado
    TEST_ASSERT_TRUE(emu_events_enhanced_init(&g_events, 32));

    // Resetar contadores de callback
    g_callback_counter = 0;
    memset(g_event_types_received, 0, sizeof(g_event_types_received));
}

void tearDown(void)
{
    if (g_queue)
    {
        emu_event_queue_destroy(g_queue);
        g_queue = NULL;
    }

    emu_events_enhanced_shutdown(&g_events);
}

// Callback para testes
static void test_callback(emu_event_t *event, void *userdata)
{
    TEST_ASSERT_NOT_NULL(event);

    g_callback_counter++;

    if (event->type < EMU_EVENT_MAX)
    {
        g_event_types_received[event->type]++;
    }
}

// Testes de Criação/Destruição da Fila de Prioridade
void test_queue_create_destroy(void)
{
    emu_event_priority_queue_t *queue = emu_event_queue_create(32);
    TEST_ASSERT_NOT_NULL(queue);

    emu_event_queue_destroy(queue);

    // Teste com valor inválido
    queue = emu_event_queue_create(0);
    TEST_ASSERT_NULL(queue);
}

// Testes de Operações Básicas da Fila
void test_queue_basic_operations(void)
{
    emu_event_t event = {
        .type = EMU_EVENT_FRAME_START,
        .timestamp = 100,
        .data = NULL,
        .data_size = 0};

    // Adicionar evento à fila
    TEST_ASSERT_TRUE(emu_event_queue_push(g_queue, &event, EMU_EVENT_PRIORITY_NORMAL, 0));

    // Verificar tamanho da fila
    TEST_ASSERT_EQUAL_UINT32(1, emu_event_queue_size(g_queue));

    // Ler evento da fila
    emu_event_t result;
    TEST_ASSERT_TRUE(emu_event_queue_peek(g_queue, &result));
    TEST_ASSERT_EQUAL_UINT32(EMU_EVENT_FRAME_START, result.type);

    // Remover evento da fila
    TEST_ASSERT_TRUE(emu_event_queue_pop(g_queue, &result));
    TEST_ASSERT_EQUAL_UINT32(EMU_EVENT_FRAME_START, result.type);

    // Verificar que a fila está vazia
    TEST_ASSERT_EQUAL_UINT32(0, emu_event_queue_size(g_queue));
}

// Testes de Prioridade
void test_queue_priority_order(void)
{
    // Adicionar eventos com diferentes prioridades
    emu_event_t events[4] = {
        {.type = EMU_EVENT_FRAME_START, .timestamp = 100, .data = NULL, .data_size = 0},
        {.type = EMU_EVENT_FRAME_END, .timestamp = 100, .data = NULL, .data_size = 0},
        {.type = EMU_EVENT_VBLANK, .timestamp = 100, .data = NULL, .data_size = 0},
        {.type = EMU_EVENT_HBLANK, .timestamp = 100, .data = NULL, .data_size = 0}};

    TEST_ASSERT_TRUE(emu_event_queue_push(g_queue, &events[0], EMU_EVENT_PRIORITY_NORMAL, 0));
    TEST_ASSERT_TRUE(emu_event_queue_push(g_queue, &events[1], EMU_EVENT_PRIORITY_LOW, 0));
    TEST_ASSERT_TRUE(emu_event_queue_push(g_queue, &events[2], EMU_EVENT_PRIORITY_CRITICAL, 0));
    TEST_ASSERT_TRUE(emu_event_queue_push(g_queue, &events[3], EMU_EVENT_PRIORITY_HIGH, 0));

    // Verificar a ordem de processamento (da maior para a menor prioridade)
    emu_event_t result;

    // Primeiro: CRITICAL
    TEST_ASSERT_TRUE(emu_event_queue_pop(g_queue, &result));
    TEST_ASSERT_EQUAL_UINT32(EMU_EVENT_VBLANK, result.type);

    // Segundo: HIGH
    TEST_ASSERT_TRUE(emu_event_queue_pop(g_queue, &result));
    TEST_ASSERT_EQUAL_UINT32(EMU_EVENT_HBLANK, result.type);

    // Terceiro: NORMAL
    TEST_ASSERT_TRUE(emu_event_queue_pop(g_queue, &result));
    TEST_ASSERT_EQUAL_UINT32(EMU_EVENT_FRAME_START, result.type);

    // Quarto: LOW
    TEST_ASSERT_TRUE(emu_event_queue_pop(g_queue, &result));
    TEST_ASSERT_EQUAL_UINT32(EMU_EVENT_FRAME_END, result.type);
}

// Testes de Agendamento Diferido
void test_queue_delayed_events(void)
{
    // Adicionar eventos com diferentes atrasos
    emu_event_t events[3] = {
        {.type = EMU_EVENT_FRAME_START, .timestamp = 100, .data = NULL, .data_size = 0},
        {.type = EMU_EVENT_FRAME_END, .timestamp = 100, .data = NULL, .data_size = 0},
        {.type = EMU_EVENT_VBLANK, .timestamp = 100, .data = NULL, .data_size = 0}};

    // Adicionar eventos com atrasos de 100ms, 50ms e 0ms
    emu_event_queue_push(g_queue, &events[0], EMU_EVENT_PRIORITY_NORMAL, 100);
    emu_event_queue_push(g_queue, &events[1], EMU_EVENT_PRIORITY_NORMAL, 50);
    emu_event_queue_push(g_queue, &events[2], EMU_EVENT_PRIORITY_NORMAL, 0);

    // Atualizar o timestamp da fila para 100
    emu_event_queue_update(g_queue, 100);

    // Verificar que apenas o evento sem atraso está disponível
    TEST_ASSERT_EQUAL_UINT32(1, emu_event_queue_size(g_queue));

    emu_event_t result;
    TEST_ASSERT_TRUE(emu_event_queue_pop(g_queue, &result));
    TEST_ASSERT_EQUAL_UINT32(EMU_EVENT_VBLANK, result.type);

    // Atualizar o timestamp da fila para 150
    emu_event_queue_update(g_queue, 150);

    // Verificar que o evento com 50ms de atraso está disponível
    TEST_ASSERT_EQUAL_UINT32(1, emu_event_queue_size(g_queue));

    TEST_ASSERT_TRUE(emu_event_queue_pop(g_queue, &result));
    TEST_ASSERT_EQUAL_UINT32(EMU_EVENT_FRAME_END, result.type);

    // Atualizar o timestamp da fila para 200
    emu_event_queue_update(g_queue, 200);

    // Verificar que o evento com 100ms de atraso está disponível
    TEST_ASSERT_EQUAL_UINT32(1, emu_event_queue_size(g_queue));

    TEST_ASSERT_TRUE(emu_event_queue_pop(g_queue, &result));
    TEST_ASSERT_EQUAL_UINT32(EMU_EVENT_FRAME_START, result.type);
}

// Testes de Cancelamento
void test_queue_cancel_events(void)
{
    // Adicionar eventos
    emu_event_t events[3] = {
        {.type = EMU_EVENT_FRAME_START, .timestamp = 100, .data = NULL, .data_size = 0},
        {.type = EMU_EVENT_FRAME_END, .timestamp = 100, .data = NULL, .data_size = 0},
        {.type = EMU_EVENT_FRAME_START, .timestamp = 100, .data = NULL, .data_size = 0}};

    // Adicionar eventos à fila
    for (int i = 0; i < 3; i++)
    {
        TEST_ASSERT_TRUE(emu_event_queue_push(g_queue, &events[i], EMU_EVENT_PRIORITY_NORMAL, 0));
    }

    // Verificar que a fila tem 3 eventos
    TEST_ASSERT_EQUAL_UINT32(3, emu_event_queue_size(g_queue));

    // Cancelar todos os eventos FRAME_START
    TEST_ASSERT_TRUE(emu_event_queue_cancel(g_queue, EMU_EVENT_FRAME_START, NULL));

    // A fila agora deve ter apenas 1 evento (devido à forma como o cancelamento é implementado)
    // O tamanho retornado leva em consideração apenas eventos não cancelados
    TEST_ASSERT_EQUAL_UINT32(1, emu_event_queue_size(g_queue));

    // Remover os eventos remanescentes
    emu_event_t result;
    TEST_ASSERT_TRUE(emu_event_queue_pop(g_queue, &result));
    TEST_ASSERT_EQUAL_UINT32(EMU_EVENT_FRAME_END, result.type);

    // Não deve haver mais eventos
    TEST_ASSERT_FALSE(emu_event_queue_pop(g_queue, &result));
}

// Testes de Sistema de Eventos Aprimorado - Inicialização
void test_enhanced_init_shutdown(void)
{
    emu_events_enhanced_t events;

    // Teste de inicialização
    TEST_ASSERT_TRUE(emu_events_enhanced_init(&events, 32));

    // Verificar valores padrão
    TEST_ASSERT_FALSE(emu_events_enhanced_is_paused(&events));

    // Testar shutdown
    emu_events_enhanced_shutdown(&events);

    // Testar parâmetros inválidos
    TEST_ASSERT_FALSE(emu_events_enhanced_init(NULL, 32));
    TEST_ASSERT_FALSE(emu_events_enhanced_init(&events, 0));
}

// Testes de Agendamento de Eventos
void test_enhanced_scheduling(void)
{
    // Registrar callbacks
    uint32_t callback_id = emu_events_enhanced_register_callback(&g_events,
                                                                 EMU_EVENT_FRAME_START,
                                                                 test_callback,
                                                                 NULL);
    TEST_ASSERT_NOT_EQUAL_UINT32(0, callback_id);

    // Agendar eventos normais
    TEST_ASSERT_TRUE(emu_events_enhanced_schedule(&g_events,
                                                  EMU_EVENT_FRAME_START,
                                                  NULL, 0,
                                                  EMU_EVENT_PRIORITY_NORMAL,
                                                  0));

    // Agendar eventos diferidos
    TEST_ASSERT_TRUE(emu_events_enhanced_schedule(&g_events,
                                                  EMU_EVENT_FRAME_START,
                                                  NULL, 0,
                                                  EMU_EVENT_PRIORITY_NORMAL,
                                                  50));

    // Processar eventos (apenas o imediato deve ser processado)
    uint32_t processed = emu_events_enhanced_process(&g_events, 100, 0);
    TEST_ASSERT_EQUAL_UINT32(1, processed);
    TEST_ASSERT_EQUAL_INT(1, g_callback_counter);

    // Avançar o tempo e processar novamente para pegar o evento diferido
    processed = emu_events_enhanced_process(&g_events, 150, 0);
    TEST_ASSERT_EQUAL_UINT32(1, processed);
    TEST_ASSERT_EQUAL_INT(2, g_callback_counter);

    // Remover callback
    TEST_ASSERT_TRUE(emu_events_enhanced_unregister_callback(&g_events,
                                                             EMU_EVENT_FRAME_START,
                                                             test_callback));
}

// Testes de Eventos Periódicos
void test_enhanced_periodic_events(void)
{
    // Registrar callbacks
    emu_events_enhanced_register_callback(&g_events,
                                          EMU_EVENT_FRAME_START,
                                          test_callback,
                                          NULL);

    // Agendar evento periódico a cada 50ms, por 3 vezes
    uint32_t periodic_id = emu_events_enhanced_schedule_periodic(&g_events,
                                                                 EMU_EVENT_FRAME_START,
                                                                 NULL, 0,
                                                                 EMU_EVENT_PRIORITY_NORMAL,
                                                                 50, 3);
    TEST_ASSERT_NOT_EQUAL_UINT32(0, periodic_id);

    // Processar eventos em intervalos de 50ms
    emu_events_enhanced_process(&g_events, 50, 0);
    TEST_ASSERT_EQUAL_INT(1, g_callback_counter);

    emu_events_enhanced_process(&g_events, 100, 0);
    TEST_ASSERT_EQUAL_INT(2, g_callback_counter);

    emu_events_enhanced_process(&g_events, 150, 0);
    TEST_ASSERT_EQUAL_INT(3, g_callback_counter);

    // Não deve haver mais eventos (limite de 3)
    emu_events_enhanced_process(&g_events, 200, 0);
    TEST_ASSERT_EQUAL_INT(3, g_callback_counter);

    // Agendar evento periódico infinito
    periodic_id = emu_events_enhanced_schedule_periodic(&g_events,
                                                        EMU_EVENT_FRAME_END,
                                                        NULL, 0,
                                                        EMU_EVENT_PRIORITY_NORMAL,
                                                        50, 0);
    TEST_ASSERT_NOT_EQUAL_UINT32(0, periodic_id);

    // Registrar callback para o evento
    emu_events_enhanced_register_callback(&g_events,
                                          EMU_EVENT_FRAME_END,
                                          test_callback,
                                          NULL);

    // Processar alguns eventos
    emu_events_enhanced_process(&g_events, 250, 0);
    emu_events_enhanced_process(&g_events, 300, 0);
    emu_events_enhanced_process(&g_events, 350, 0);

    // Verificar que os eventos estão sendo processados
    TEST_ASSERT_GREATER_THAN_INT(3, g_callback_counter);

    // Cancelar o evento periódico
    TEST_ASSERT_TRUE(emu_events_enhanced_cancel_periodic(&g_events, periodic_id));

    // Processar mais eventos
    int current_count = g_callback_counter;
    emu_events_enhanced_process(&g_events, 400, 0);
    emu_events_enhanced_process(&g_events, 450, 0);

    // Verificar que não houve novos eventos
    TEST_ASSERT_EQUAL_INT(current_count, g_callback_counter);
}

// Testes de Pausa/Retomada
void test_enhanced_pause_resume(void)
{
    // Registrar callbacks
    emu_events_enhanced_register_callback(&g_events,
                                          EMU_EVENT_FRAME_START,
                                          test_callback,
                                          NULL);

    // Agendar evento
    TEST_ASSERT_TRUE(emu_events_enhanced_schedule(&g_events,
                                                  EMU_EVENT_FRAME_START,
                                                  NULL, 0,
                                                  EMU_EVENT_PRIORITY_NORMAL,
                                                  0));

    // Pausar o sistema
    emu_events_enhanced_pause(&g_events);
    TEST_ASSERT_TRUE(emu_events_enhanced_is_paused(&g_events));

    // Processar eventos - não deve processar nada pois está pausado
    uint32_t processed = emu_events_enhanced_process(&g_events, 100, 0);
    TEST_ASSERT_EQUAL_UINT32(0, processed);
    TEST_ASSERT_EQUAL_INT(0, g_callback_counter);

    // Retomar o sistema
    emu_events_enhanced_resume(&g_events);
    TEST_ASSERT_FALSE(emu_events_enhanced_is_paused(&g_events));

    // Processar eventos novamente
    processed = emu_events_enhanced_process(&g_events, 100, 0);
    TEST_ASSERT_EQUAL_UINT32(1, processed);
    TEST_ASSERT_EQUAL_INT(1, g_callback_counter);
}

// Testes de Carga do Sistema
void test_enhanced_system_load(void)
{
    // Configurar carga do sistema
    emu_events_enhanced_set_system_load(&g_events, 0.9f); // Alta carga

    // Registrar callbacks
    emu_events_enhanced_register_callback(&g_events,
                                          EMU_EVENT_FRAME_START,
                                          test_callback,
                                          NULL);

    emu_events_enhanced_register_callback(&g_events,
                                          EMU_EVENT_FRAME_END,
                                          test_callback,
                                          NULL);

    // Agendar eventos com diferentes prioridades
    TEST_ASSERT_TRUE(emu_events_enhanced_schedule(&g_events,
                                                  EMU_EVENT_FRAME_START,
                                                  NULL, 0,
                                                  EMU_EVENT_PRIORITY_CRITICAL,
                                                  0));

    TEST_ASSERT_TRUE(emu_events_enhanced_schedule(&g_events,
                                                  EMU_EVENT_FRAME_END,
                                                  NULL, 0,
                                                  EMU_EVENT_PRIORITY_LOW,
                                                  0));

    // Processar eventos
    uint32_t processed = emu_events_enhanced_process(&g_events, 100, 0);

    // Mesmo com alta carga, eventos críticos são processados
    TEST_ASSERT_GREATER_THAN_UINT32(0, processed);
    TEST_ASSERT_GREATER_THAN_INT(0, g_event_types_received[EMU_EVENT_FRAME_START]);
}

// Testes de Purga
void test_enhanced_purge(void)
{
    // Agendar eventos
    TEST_ASSERT_TRUE(emu_events_enhanced_schedule(&g_events,
                                                  EMU_EVENT_FRAME_START,
                                                  NULL, 0,
                                                  EMU_EVENT_PRIORITY_NORMAL,
                                                  0));

    TEST_ASSERT_TRUE(emu_events_enhanced_schedule(&g_events,
                                                  EMU_EVENT_FRAME_END,
                                                  NULL, 0,
                                                  EMU_EVENT_PRIORITY_NORMAL,
                                                  0));

    // Cancelar um evento
    TEST_ASSERT_TRUE(emu_events_enhanced_cancel(&g_events, EMU_EVENT_FRAME_START, NULL));

    // Purgar eventos cancelados
    uint32_t purged = emu_events_enhanced_purge(&g_events);
    TEST_ASSERT_EQUAL_UINT32(1, purged);

    // Verificar estatísticas
    uint32_t queue_size;
    emu_events_enhanced_get_stats(&g_events, &queue_size, NULL, NULL);
    TEST_ASSERT_EQUAL_UINT32(1, queue_size);
}

// Testes de Estatísticas
void test_enhanced_stats(void)
{
    // Registrar callbacks
    emu_events_enhanced_register_callback(&g_events,
                                          EMU_EVENT_FRAME_START,
                                          test_callback,
                                          NULL);

    // Agendar e processar vários eventos
    for (int i = 0; i < 10; i++)
    {
        TEST_ASSERT_TRUE(emu_events_enhanced_schedule(&g_events,
                                                      EMU_EVENT_FRAME_START,
                                                      NULL, 0,
                                                      EMU_EVENT_PRIORITY_NORMAL,
                                                      0));

        emu_events_enhanced_process(&g_events, 100 + i * 10, 0);
    }

    // Obter estatísticas
    uint32_t queue_size, events_per_second;
    float avg_time;
    emu_events_enhanced_get_stats(&g_events, &queue_size, &events_per_second, &avg_time);

    // Verificar estatísticas básicas
    TEST_ASSERT_EQUAL_UINT32(0, queue_size); // Todos processados
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(0, events_per_second);
    TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(0.0f, avg_time);
}

// Runner de testes
int main(void)
{
    UNITY_BEGIN();

    // Testes da fila de prioridade
    RUN_TEST(test_queue_create_destroy);
    RUN_TEST(test_queue_basic_operations);
    RUN_TEST(test_queue_priority_order);
    RUN_TEST(test_queue_delayed_events);
    RUN_TEST(test_queue_cancel_events);

    // Testes do sistema de eventos aprimorado
    RUN_TEST(test_enhanced_init_shutdown);
    RUN_TEST(test_enhanced_scheduling);
    RUN_TEST(test_enhanced_periodic_events);
    RUN_TEST(test_enhanced_pause_resume);
    RUN_TEST(test_enhanced_system_load);
    RUN_TEST(test_enhanced_purge);
    RUN_TEST(test_enhanced_stats);

    return UNITY_END();
}
