#include "frontend.h"
#include "frontend_internal.h"
#include "../../utils/enhanced_log.h"
#include "../../utils/error_handling.h"
#include "../platform/sdl/sdl_frontend_adapter.h"
#include <stdlib.h>
#include <string.h>

// Limites do framebuffer
#define EMU_MAX_FRAME_WIDTH 1920
#define EMU_MAX_FRAME_HEIGHT 1080

// Macros de verificação
#define EMU_CHECK_BOUNDS(value, min, max, msg)                                                  \
    do                                                                                          \
    {                                                                                           \
        if ((value) < (min) || (value) > (max))                                                 \
        {                                                                                       \
            LOG_ERROR(EMU_LOG_CAT_FRONTEND, "%s: %d (min: %d, max: %d)", msg, value, min, max); \
            return -1;                                                                          \
        }                                                                                       \
    } while (0)

// Estrutura do frontend
struct emu_frontend {
    bool initialized;
    uint32_t *framebuffer;
    int32_t framebuffer_width;
    int32_t framebuffer_height;
};

// Implementação das funções do frontend

emu_frontend_t emu_frontend_init(const char* title, int32_t width, int32_t height) {
    // Alocar estrutura do frontend
    emu_frontend_t frontend = (emu_frontend_t)malloc(sizeof(struct emu_frontend));
    
    if (!frontend) {
        LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Failed to allocate frontend structure");
        return NULL;
    }
    
    // Inicializar estrutura
    memset(frontend, 0, sizeof(struct emu_frontend));
    
    // Inicializar adaptador SDL
    if (!sdl_frontend_init(title, width, height)) {
        LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Failed to initialize SDL frontend adapter");
        free(frontend);
        return NULL;
    }
    
    // Configurar frontend
    frontend->initialized = true;
    frontend->framebuffer = NULL;
    frontend->framebuffer_width = 0;
    frontend->framebuffer_height = 0;
    
    LOG_INFO(EMU_LOG_CAT_FRONTEND, "Frontend initialized");
    return frontend;
}

void emu_frontend_shutdown(emu_frontend_t frontend) {
    if (!frontend) {
        return;
    }
    
    // Finalizar adaptador SDL
    sdl_frontend_shutdown();
    
    // Liberar framebuffer
    if (frontend->framebuffer) {
        free(frontend->framebuffer);
        frontend->framebuffer = NULL;
    }
    
    // Liberar estrutura
    frontend->initialized = false;
    free(frontend);
    
    LOG_INFO(EMU_LOG_CAT_FRONTEND, "Frontend shutdown");
}

int32_t emu_frontend_run(emu_frontend_t frontend) {
    if (!frontend || !frontend->initialized) {
        LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Invalid frontend");
        return -1;
    }
    
    bool running = true;
    
    while (running) {
        // Processar eventos
        running = sdl_frontend_process_events();
        
        // Atualizar estado
        sdl_frontend_update();
        
        // Renderizar
        sdl_frontend_render();
    }
    
    return 0;
}

int32_t emu_frontend_render_frame(emu_frontend_t frontend, const uint32_t *framebuffer, int32_t width, int32_t height) {
    if (!frontend || !frontend->initialized) {
        LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Invalid frontend");
        return -1;
    }
    
    if (!framebuffer) {
        LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Invalid framebuffer");
        return -1;
    }
    
    // Verificar limites
    EMU_CHECK_BOUNDS(width, 1, EMU_MAX_FRAME_WIDTH, "Invalid framebuffer width");
    EMU_CHECK_BOUNDS(height, 1, EMU_MAX_FRAME_HEIGHT, "Invalid framebuffer height");
    
    // Realocar framebuffer se necessário
    if (frontend->framebuffer_width != width || frontend->framebuffer_height != height) {
        if (frontend->framebuffer) {
            free(frontend->framebuffer);
        }
        
        frontend->framebuffer = (uint32_t *)malloc(width * height * sizeof(uint32_t));
        
        if (!frontend->framebuffer) {
            LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Failed to allocate framebuffer");
            return -1;
        }
        
        frontend->framebuffer_width = width;
        frontend->framebuffer_height = height;
    }
    
    // Copiar framebuffer
    memcpy(frontend->framebuffer, framebuffer, width * height * sizeof(uint32_t));
    
    return 0;
}

int32_t emu_frontend_update_window(emu_frontend_t frontend) {
    if (!frontend || !frontend->initialized) {
        LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Invalid frontend");
        return -1;
    }
    
    // Atualizar e renderizar
    sdl_frontend_update();
    sdl_frontend_render();
    
    return 0;
}

bool emu_frontend_process_events(emu_frontend_t frontend) {
    if (!frontend || !frontend->initialized) {
        LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Invalid frontend");
        return false;
    }
    
    return sdl_frontend_process_events();
}

gui_manager_t emu_frontend_get_gui_manager(emu_frontend_t frontend) {
    if (!frontend || !frontend->initialized) {
        LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Invalid frontend");
        return NULL;
    }
    
    return sdl_frontend_get_gui_manager();
}

gui_element_id_t emu_frontend_create_element(emu_frontend_t frontend, gui_element_type_t type, const gui_rect_t* rect, const char* text) {
    if (!frontend || !frontend->initialized) {
        LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Invalid frontend");
        return GUI_INVALID_ID;
    }
    
    gui_manager_t manager = sdl_frontend_get_gui_manager();
    
    if (!manager) {
        LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Invalid GUI manager");
        return GUI_INVALID_ID;
    }
    
    return gui_manager_add_element(manager, type, rect, text);
}

void emu_frontend_remove_element(emu_frontend_t frontend, gui_element_id_t element_id) {
    if (!frontend || !frontend->initialized) {
        LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Invalid frontend");
        return;
    }
    
    gui_manager_t manager = sdl_frontend_get_gui_manager();
    
    if (!manager) {
        LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Invalid GUI manager");
        return;
    }
    
    gui_manager_remove_element(manager, element_id);
}

void emu_frontend_set_background_color(emu_frontend_t frontend, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!frontend || !frontend->initialized) {
        LOG_ERROR(EMU_LOG_CAT_FRONTEND, "Invalid frontend");
        return;
    }
    
    sdl_frontend_set_background_color(r, g, b, a);
}