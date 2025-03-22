/** * @file event_filter.c * @brief Implementação do filtro para eventos do sistema */#include "event_filter.h"#include <stdlib.h>#include <string.h>/** * @brief Inicializa um filtro com valores padrão * * @param filter Ponteiro para o filtro a ser inicializado */void event_filter_init(EventFilter *filter){    if (filter)    {        memset(filter, 0, sizeof(EventFilter));        // Configurar valores padrão        filter->type_mask = (uint32_t)-1; // Todos os tipos por padrão        filter->time_start = 0;        filter->time_end = UINT64_MAX;        filter->source_id = 0;        filter->pattern = NULL;        filter->custom_filter = NULL;    }}/** * @brief Verifica se um evento corresponde aos critérios do filtro * * @param filter Filtro a ser aplicado * @param event Evento a ser verificado * @return true se o evento corresponde ao filtro, false caso contrário */bool event_filter_match(const EventFilter *filter, const EmuNodeEvent *event){    if (!filter || !event)    {        return false;    }    // Verificar tipo    if ((1 << event->type) & filter->type_mask)    {        // Verificar intervalo de tempo        if (event->timestamp >= filter->time_start && event->timestamp <= filter->time_end)        {            // Verificar fonte            if (filter->source_id == 0 || filter->source_id == event->source_id)            {                // Verificar padrão                if (filter->pattern == NULL || strstr(event->message, filter->pattern) != NULL)                {                    // Verificar filtro personalizado                    if (filter->custom_filter == NULL || filter->custom_filter(event))                    {                        return true;                    }                }            }        }    }    return false;}/** * @brief Limpa recursos alocados pelo filtro * * @param filter Filtro a ser limpo */void event_filter_cleanup(EventFilter *filter){    // Atualmente não há recursos alocados dinamicamente no filtro    // Esta função é mantida para compatibilidade futura    (void)filter;}