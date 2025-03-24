#include "platform_base.h"
#include <stdlib.h>
#include <string.h>

// Estrutura base para todas as plataformas
struct PlatformBase
{
    char *name;
    uint32_t clock_speed;
    bool running;
    void *platform_specific;
};

PlatformBase *platform_base_create(const char *name, uint32_t clock_speed)
{
    if (!name)
        return NULL;

    PlatformBase *base = (PlatformBase *)malloc(sizeof(PlatformBase));
    if (!base)
        return NULL;

    base->name = strdup(name);
    if (!base->name)
    {
        free(base);
        return NULL;
    }

    base->clock_speed = clock_speed;
    base->running = false;
    base->platform_specific = NULL;

    return base;
}

void platform_base_destroy(PlatformBase *base)
{
    if (!base)
        return;

    free(base->name);
    free(base);
}

const char *platform_base_get_name(const PlatformBase *base)
{
    return base ? base->name : NULL;
}

uint32_t platform_base_get_clock_speed(const PlatformBase *base)
{
    return base ? base->clock_speed : 0;
}

bool platform_base_is_running(const PlatformBase *base)
{
    return base ? base->running : false;
}

void platform_base_set_running(PlatformBase *base, bool running)
{
    if (base)
    {
        base->running = running;
    }
}

void *platform_base_get_specific(PlatformBase *base)
{
    return base ? base->platform_specific : NULL;
}

void platform_base_set_specific(PlatformBase *base, void *specific)
{
    if (base)
    {
        base->platform_specific = specific;
    }
}
