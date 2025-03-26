/**
 * @file priority_queue.c
 * @brief Implementação de fila de prioridade adaptativa para gerenciamento de eventos
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "priority_queue.h"
#include "../../utils/error_handling.h"

// Obter o tempo atual em milissegundos
static uint64_t _get_current_time_ms(void)
{
    // Implementação real deve usar o tempo do sistema
    // Esta é apenas uma função stub para referência
    static uint64_t time = 0;
    return time++;
}

emu_event_priority_queue_t *emu_event_queue_create(uint32_t capacity)
{
    if (capacity == 0)
    {
        return NULL;
    }

    emu_event_priority_queue_t *queue = (emu_event_priority_queue_t *)malloc(sizeof(emu_event_priority_queue_t));
    if (!queue)
    {
        return NULL;
    }

    queue->items = (emu_event_queue_item_t *)malloc(capacity * sizeof(emu_event_queue_item_t));
    if (!queue->items)
    {
        free(queue);
        return NULL;
    }

    queue->capacity = capacity;
    queue->size = 0;
    queue->current_time = _get_current_time_ms();

    return queue;
}

void emu_event_queue_destroy(emu_event_priority_queue_t *queue)
{
    if (!queue)
    {
        return;
    }

    if (queue->items)
    {
        // Liberar memória de dados dos eventos, se necessário
        for (uint32_t i = 0; i < queue->size; i++)
        {
            if (queue->items[i].event.data)
            {
                free(queue->items[i].event.data);
                queue->items[i].event.data = NULL;
            }
        }

        free(queue->items);
        queue->items = NULL;
    }

    free(queue);
}

/**
 * Função auxiliar para posicionar um evento na fila usando heap binário
 * @param queue Ponteiro para a fila
 * @param index Índice do item a ser posicionado
 */
static void _heapify_up(emu_event_priority_queue_t *queue, uint32_t index)
{
    if (index == 0)
    {
        return; // Já está na raiz
    }

    while (index > 0)
    {
        uint32_t parent_index = (index - 1) / 2;

        // Verificar se o item já está na posição correta
        if (queue->items[parent_index].priority >= queue->items[index].priority &&
            queue->items[parent_index].scheduled_time <= queue->items[index].scheduled_time)
        {
            break;
        }

        // Trocar com o pai
        emu_event_queue_item_t temp = queue->items[parent_index];
        queue->items[parent_index] = queue->items[index];
        queue->items[index] = temp;

        // Continuar subindo
        index = parent_index;
    }
}

/**
 * Função auxiliar para manter a propriedade de heap após remoção
 * @param queue Ponteiro para a fila
 * @param index Índice a partir do qual fazer heapify
 */
static void _heapify_down(emu_event_priority_queue_t *queue, uint32_t index)
{
    uint32_t size = queue->size;
    if (size <= 1 || index >= size)
    {
        return;
    }

    while (true)
    {
        uint32_t left_child = 2 * index + 1;
        uint32_t right_child = 2 * index + 2;
        uint32_t highest_priority = index;

        // Encontrar o item de maior prioridade entre o nó atual e seus filhos
        if (left_child < size &&
            (queue->items[left_child].priority > queue->items[highest_priority].priority ||
             (queue->items[left_child].priority == queue->items[highest_priority].priority &&
              queue->items[left_child].scheduled_time < queue->items[highest_priority].scheduled_time)))
        {
            highest_priority = left_child;
        }

        if (right_child < size &&
            (queue->items[right_child].priority > queue->items[highest_priority].priority ||
             (queue->items[right_child].priority == queue->items[highest_priority].priority &&
              queue->items[right_child].scheduled_time < queue->items[highest_priority].scheduled_time)))
        {
            highest_priority = right_child;
        }

        // Se o item atual tem a maior prioridade, está na posição correta
        if (highest_priority == index)
        {
            break;
        }

        // Trocar com o filho de maior prioridade
        emu_event_queue_item_t temp = queue->items[index];
        queue->items[index] = queue->items[highest_priority];
        queue->items[highest_priority] = temp;

        // Continuar descendo
        index = highest_priority;
    }
}

bool emu_event_queue_push(emu_event_priority_queue_t *queue, const emu_event_t *event,
                          emu_event_priority_t priority, uint64_t delay_ms)
{
    if (!queue || !event)
    {
        return false;
    }

    // Verificar se a fila está cheia
    if (queue->size >= queue->capacity)
    {
        return false;
    }

    // Copiar o evento
    emu_event_queue_item_t item;
    memcpy(&item.event, event, sizeof(emu_event_t));

    // Copiar os dados do evento, se existirem
    if (event->data && event->data_size > 0)
    {
        item.event.data = malloc(event->data_size);
        if (!item.event.data)
        {
            return false;
        }
        memcpy(item.event.data, event->data, event->data_size);
    }

    // Configurar prioridade e tempo de agendamento
    item.priority = priority;
    item.scheduled_time = queue->current_time + delay_ms;
    item.is_canceled = false;

    // Adicionar o item ao final da fila
    queue->items[queue->size] = item;

    // Ajustar a posição do item na fila
    _heapify_up(queue, queue->size);

    // Incrementar o tamanho da fila
    queue->size++;

    return true;
}

bool emu_event_queue_pop(emu_event_priority_queue_t *queue, emu_event_t *event)
{
    if (!queue || !event || queue->size == 0)
    {
        return false;
    }

    // Encontrar o próximo evento válido
    uint32_t index = 0;
    bool found = false;

    // Verificar se o evento no topo não foi cancelado e está no tempo de execução
    if (!queue->items[0].is_canceled && queue->items[0].scheduled_time <= queue->current_time)
    {
        found = true;
    }
    else
    {
        // Percorrer a fila em busca de um evento válido
        uint32_t i = 0;
        while (i < queue->size)
        {
            if (!queue->items[i].is_canceled && queue->items[i].scheduled_time <= queue->current_time)
            {
                found = true;
                index = i;
                break;
            }
            i++;
        }
    }

    if (!found)
    {
        return false;
    }

    // Copiar o evento para o resultado
    memcpy(event, &queue->items[index].event, sizeof(emu_event_t));

    // Remover o evento da fila
    if (index < queue->size - 1)
    {
        // Mover o último item para a posição do item removido
        queue->items[index] = queue->items[queue->size - 1];

        // Manter a propriedade de heap
        _heapify_down(queue, index);
    }

    // Diminuir o tamanho da fila
    queue->size--;

    return true;
}

bool emu_event_queue_peek(emu_event_priority_queue_t *queue, emu_event_t *event)
{
    if (!queue || !event || queue->size == 0)
    {
        return false;
    }

    // Encontrar o próximo evento válido
    uint32_t i = 0;
    bool found = false;

    while (i < queue->size)
    {
        if (!queue->items[i].is_canceled && queue->items[i].scheduled_time <= queue->current_time)
        {
            found = true;
            break;
        }
        i++;
    }

    if (!found)
    {
        return false;
    }

    // Copiar o evento para o resultado
    memcpy(event, &queue->items[i].event, sizeof(emu_event_t));

    return true;
}

uint32_t emu_event_queue_size(emu_event_priority_queue_t *queue)
{
    if (!queue)
    {
        return 0;
    }

    // Contar apenas eventos não cancelados
    uint32_t valid_count = 0;
    for (uint32_t i = 0; i < queue->size; i++)
    {
        if (!queue->items[i].is_canceled)
        {
            valid_count++;
        }
    }

    return valid_count;
}

void emu_event_queue_update(emu_event_priority_queue_t *queue, uint64_t current_time)
{
    if (!queue)
    {
        return;
    }

    queue->current_time = current_time;
}

bool emu_event_queue_cancel(emu_event_priority_queue_t *queue, emu_event_type_t type, void *data)
{
    if (!queue)
    {
        return false;
    }

    bool canceled = false;

    for (uint32_t i = 0; i < queue->size; i++)
    {
        if (queue->items[i].event.type == type && !queue->items[i].is_canceled)
        {
            // Se data for fornecido, verificar correspondência
            if (data && queue->items[i].event.data)
            {
                if (queue->items[i].event.data == data)
                {
                    queue->items[i].is_canceled = true;
                    canceled = true;
                }
            }
            else
            {
                // Se data não for especificado, cancelar todos do tipo
                queue->items[i].is_canceled = true;
                canceled = true;
            }
        }
    }

    return canceled;
}

void emu_event_queue_adapt_priorities(emu_event_priority_queue_t *queue, float system_load)
{
    if (!queue)
    {
        return;
    }

    // Limitar system_load ao intervalo [0.0, 1.0]
    if (system_load < 0.0f)
        system_load = 0.0f;
    if (system_load > 1.0f)
        system_load = 1.0f;

    // Sob alta carga do sistema, priorizamos eventos críticos e diminuímos a prioridade de eventos de baixa prioridade
    if (system_load > 0.8f)
    {
        for (uint32_t i = 0; i < queue->size; i++)
        {
            if (queue->items[i].priority == EMU_EVENT_PRIORITY_LOW)
            {
                // Atrasar eventos de baixa prioridade
                queue->items[i].scheduled_time += 50;
            }
            else if (queue->items[i].priority == EMU_EVENT_PRIORITY_CRITICAL)
            {
                // Garantir que eventos críticos sejam processados o quanto antes
                queue->items[i].scheduled_time = queue->current_time;
            }
        }
    }

    // Reordenar a fila após a adaptação de prioridades
    // Isso poderia ser otimizado usando um algoritmo de build-heap em O(n)
    for (uint32_t i = queue->size / 2; i > 0; i--)
    {
        _heapify_down(queue, i - 1);
    }
}

uint32_t emu_event_queue_purge(emu_event_priority_queue_t *queue)
{
    if (!queue)
    {
        return 0;
    }

    uint32_t removed_count = 0;

    // Compactar a fila, removendo eventos cancelados
    uint32_t write_index = 0;
    for (uint32_t read_index = 0; read_index < queue->size; read_index++)
    {
        if (!queue->items[read_index].is_canceled)
        {
            if (write_index != read_index)
            {
                queue->items[write_index] = queue->items[read_index];
            }
            write_index++;
        }
        else
        {
            // Liberar memória do evento cancelado
            if (queue->items[read_index].event.data)
            {
                free(queue->items[read_index].event.data);
                queue->items[read_index].event.data = NULL;
            }
            removed_count++;
        }
    }

    // Atualizar o tamanho da fila
    queue->size = write_index;

    // Reordenar a fila após a remoção de itens
    for (uint32_t i = queue->size / 2; i > 0; i--)
    {
        _heapify_down(queue, i - 1);
    }

    return removed_count;
}

void emu_event_queue_clear(emu_event_priority_queue_t *queue)
{
    if (!queue)
    {
        return;
    }

    // Liberar memória dos dados dos eventos
    for (uint32_t i = 0; i < queue->size; i++)
    {
        if (queue->items[i].event.data)
        {
            free(queue->items[i].event.data);
            queue->items[i].event.data = NULL;
        }
    }

    // Resetar tamanho da fila
    queue->size = 0;
}

// Funções auxiliares para análise e depuração da fila de prioridade

/**
 * Imprime o conteúdo da fila de prioridade para fins de depuração
 * @param queue Ponteiro para a fila
 */
void emu_event_queue_debug_print(emu_event_priority_queue_t *queue)
{
    if (!queue)
    {
        printf("Queue is NULL\n");
        return;
    }

    printf("Queue: size=%u, capacity=%u, current_time=%llu\n",
           queue->size, queue->capacity, (unsigned long long)queue->current_time);

    for (uint32_t i = 0; i < queue->size; i++)
    {
        printf("  [%u] type=%d, priority=%d, scheduled=%llu, canceled=%d\n",
               i, queue->items[i].event.type, queue->items[i].priority,
               (unsigned long long)queue->items[i].scheduled_time, queue->items[i].is_canceled);
    }
}

/**
 * Verifica a integridade da fila como heap binário
 * @param queue Ponteiro para a fila
 * @return true se a fila é um heap válido, false caso contrário
 */
bool emu_event_queue_debug_validate(emu_event_priority_queue_t *queue)
{
    if (!queue)
    {
        return false;
    }

    for (uint32_t i = 1; i < queue->size; i++)
    {
        uint32_t parent = (i - 1) / 2;

        // Verificar prioridade
        if (queue->items[i].priority > queue->items[parent].priority)
        {
            return false;
        }

        // Se mesma prioridade, verificar timestamp
        if (queue->items[i].priority == queue->items[parent].priority &&
            queue->items[i].scheduled_time < queue->items[parent].scheduled_time)
        {
            return false;
        }
    }

    return true;
}

/**
 * Calcula estatísticas sobre a distribuição de prioridades na fila
 * @param queue Ponteiro para a fila
 * @param low Ponteiro para armazenar contagem de eventos de baixa prioridade
 * @param normal Ponteiro para armazenar contagem de eventos de prioridade normal
 * @param high Ponteiro para armazenar contagem de eventos de alta prioridade
 * @param critical Ponteiro para armazenar contagem de eventos de prioridade crítica
 */
void emu_event_queue_debug_stats(emu_event_priority_queue_t *queue,
                                 uint32_t *low, uint32_t *normal,
                                 uint32_t *high, uint32_t *critical)
{
    if (!queue)
    {
        if (low)
            *low = 0;
        if (normal)
            *normal = 0;
        if (high)
            *high = 0;
        if (critical)
            *critical = 0;
        return;
    }

    uint32_t count_low = 0, count_normal = 0, count_high = 0, count_critical = 0;

    for (uint32_t i = 0; i < queue->size; i++)
    {
        if (queue->items[i].is_canceled)
        {
            continue;
        }

        switch (queue->items[i].priority)
        {
        case EMU_EVENT_PRIORITY_LOW:
            count_low++;
            break;
        case EMU_EVENT_PRIORITY_NORMAL:
            count_normal++;
            break;
        case EMU_EVENT_PRIORITY_HIGH:
            count_high++;
            break;
        case EMU_EVENT_PRIORITY_CRITICAL:
            count_critical++;
            break;
        }
    }

    if (low)
        *low = count_low;
    if (normal)
        *normal = count_normal;
    if (high)
        *high = count_high;
    if (critical)
        *critical = count_critical;
}
