#ifndef PLATFORM_BASE_H
#define PLATFORM_BASE_H

#include <stdint.h>
#include <stdbool.h>

// Estrutura opaca para a plataforma base
typedef struct PlatformBase PlatformBase;

// Funções de criação e destruição
PlatformBase *platform_base_create(const char *name, uint32_t clock_speed);
void platform_base_destroy(PlatformBase *base);

// Funções de acesso
const char *platform_base_get_name(const PlatformBase *base);
uint32_t platform_base_get_clock_speed(const PlatformBase *base);
bool platform_base_is_running(const PlatformBase *base);
void platform_base_set_running(PlatformBase *base, bool running);

// Funções para dados específicos da plataforma
void *platform_base_get_specific(PlatformBase *base);
void platform_base_set_specific(PlatformBase *base, void *specific);

#endif // PLATFORM_BASE_H
