#ifndef GUI_CONFIG_H
#define GUI_CONFIG_H

#include "gui/gui_element.h"

// Widget de configuração
typedef struct gui_config_t gui_config_t;

// Cria um novo widget de configuração
gui_config_t *gui_config_create(void);

// Destrói um widget de configuração
void gui_config_destroy(gui_config_t *config);

#endif // GUI_CONFIG_H
