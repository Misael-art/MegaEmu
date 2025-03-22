#include "../../src/gui/widgets.h"
#include "../../src/gui/advanced_widgets.h"
#include "../../src/gui/theme.h"

static gui_sdl2_backend_t backend;
static gui_button_t* start_button;
static gui_textbox_t* rom_path;
static gui_line_graph_t* fps_graph;
static gui_circular_progress_t* load_progress;
static gui_terminal_t* debug_terminal;

// Callback para o botão de início
static void on_start_clicked(gui_widget_t* widget) {
    gui_button_t* button = (gui_button_t*)widget;
    const char* path = rom_path->text;

    // Simular carregamento
    for (float i = 0.0f; i <= 1.0f; i += 0.1f) {
        gui_circular_progress_set_progress(load_progress, i);
        SDL_Delay(100); // Simulação de carga
    }

    gui_terminal_print(debug_terminal, "ROM carregada: ");
    gui_terminal_print(debug_terminal, path);
    gui_terminal_print(debug_terminal, "\n");
}

// Função principal
int main(int argc, char* argv[]) {
    // Inicializar backend
    gui_size_t window_size = {1024, 768};
    if (gui_sdl2_init(&backend, "Mega_Emu - Interface de Exemplo", window_size, GUI_TRUE) != GUI_SUCCESS) {
        return 1;
    }

    // Inicializar sistema de temas
    gui_theme_init();
    gui_theme_set_current(GUI_THEME_DARK);

    // Criar widgets
    gui_rect_t bounds;

    // Botão de início
    bounds = (gui_rect_t){10, 10, 200, 40};
    start_button = gui_button_create("Iniciar Emulação", bounds);
    start_button->base.on_click = on_start_clicked;

    // Campo de caminho da ROM
    bounds.y += 50;
    rom_path = gui_textbox_create(bounds, 256);

    // Gráfico de FPS
    bounds = (gui_rect_t){10, 110, 400, 200};
    fps_graph = gui_line_graph_create(bounds);
    gui_line_graph_enable_grid(fps_graph, GUI_TRUE);

    // Barra de progresso circular
    bounds = (gui_rect_t){420, 10, 100, 100};
    load_progress = gui_circular_progress_create(bounds, 40.0f);

    // Terminal de debug
    bounds = (gui_rect_t){10, 320, 800, 200};
    debug_terminal = gui_terminal_create(bounds, 1000);
    gui_terminal_print(debug_terminal, "Sistema inicializado\n");

    // Loop principal
    gui_bool_t running = GUI_TRUE;
    float fps_data[60] = {0};
    size_t fps_index = 0;

    while (running) {
        // Processar eventos
        gui_result_t result = gui_sdl2_process_events(&backend, NULL);
        if (result == GUI_EVENT_QUIT) {
            running = GUI_FALSE;
        }

        // Atualizar gráfico de FPS (simulado)
        fps_data[fps_index] = 55.0f + (rand() % 10);
        fps_index = (fps_index + 1) % 60;
        gui_line_graph_set_data(fps_graph, fps_data, 60);

        // Renderizar frame
        gui_sdl2_begin_frame(&backend);

        // Renderizar widgets
        start_button->base.render((gui_widget_t*)start_button, &backend);
        rom_path->base.render((gui_widget_t*)rom_path, &backend);
        fps_graph->base.render((gui_widget_t*)fps_graph, &backend);
        load_progress->base.render((gui_widget_t*)load_progress, &backend);
        debug_terminal->base.render((gui_widget_t*)debug_terminal, &backend);

        gui_sdl2_end_frame(&backend);

        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    gui_widget_destroy((gui_widget_t*)start_button);
    gui_widget_destroy((gui_widget_t*)rom_path);
    gui_widget_destroy((gui_widget_t*)fps_graph);
    gui_widget_destroy((gui_widget_t*)load_progress);
    gui_widget_destroy((gui_widget_t*)debug_terminal);

    gui_theme_shutdown();
    gui_sdl2_shutdown(&backend);

    return 0;
}
