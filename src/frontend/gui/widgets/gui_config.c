#include "gui_config.h"
#include "gui/gui_manager.h"
#include "gui/gui_element.h"
#include "frontend_config.h"
#include "utils/log_utils.h"
#include "utils/log_categories.h"

#include &ltstdlib.h&gt

// Widget de configuração
struct gui_config_t {
    gui_element_t element;
};

// Cria um novo widget de configuração
gui_config_t *gui_config_create(void) {
    gui_config_t *config = (gui_config_t *)malloc(sizeof(gui_config_t));
    if (!config) {
        LOG_ERROR("Falha ao alocar memória para o widget de configuração");
        return NULL;
    }

    // Inicializar o elemento da GUI
    gui_element_init(&config->element);

    // Definir o tipo do elemento
    config->element.type = GUI_ELEMENT_TYPE_CONFIG;

    return config;
}

// Destrói um widget de configuração
void gui_config_destroy(gui_config_t *config) {
    if (!config)
        return;

    // Limpar o elemento da GUI
    gui_element_cleanup(&config->element);

    free(config);
}
