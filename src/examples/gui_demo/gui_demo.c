/**
 * @file gui_demo.c
 * @brief Demonstração do sistema de GUI do Mega_Emu
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "../../src/frontend/common/frontend.h"
#include "../../src/frontend/gui/core/gui_types.h"
#include "../../src/frontend/gui/core/gui_manager.h"
#include "../../src/frontend/gui/widgets/gui_button.h"
#include "../../src/frontend/gui/widgets/gui_label.h"
#include "../../src/frontend/gui/widgets/gui_textbox.h"

// Callback para o botão de saída
void exit_button_callback(gui_element_id_t button_id, void* user_data) {
    bool* running = (bool*)user_data;
    if (running) {
        *running = false;
    }
    printf("Botão de saída clicado! Encerrando aplicação...\n");
}

// Callback para o botão de ação
void action_button_callback(gui_element_id_t button_id, void* user_data) {
    gui_element_id_t textbox_id = *(gui_element_id_t*)user_data;
    char text_buffer[256];
    
    if (gui_textbox_get_text(textbox_id, text_buffer, sizeof(text_buffer))) {
        printf("Texto da caixa: '%s'\n", text_buffer);
    } else {
        printf("Erro ao obter texto da caixa!\n");
    }
}

// Callback para mudança de texto
void textbox_change_callback(gui_element_id_t textbox_id, const char* text, void* user_data) {
    gui_element_id_t status_label_id = *(gui_element_id_t*)user_data;
    char status_text[256];
    
    snprintf(status_text, sizeof(status_text), "Status: Texto alterado (%zu caracteres)", strlen(text));
    gui_element_set_text(status_label_id, status_text);
    
    printf("Texto alterado: '%s'\n", text);
}

int main(int argc, char* argv[]) {
    // Inicializar frontend
    emu_frontend_t frontend = emu_frontend_init("Demonstração de GUI", 800, 600);
    
    if (!frontend) {
        fprintf(stderr, "Erro ao inicializar o frontend!\n");
        return 1;
    }
    
    // Definir cor de fundo
    emu_frontend_set_background_color(frontend, 40, 40, 40, 255);
    
    // Flag para controlar o loop principal
    bool running = true;
    
    // Obter gerenciador de GUI
    gui_manager_t gui_manager = emu_frontend_get_gui_manager(frontend);
    
    if (!gui_manager) {
        fprintf(stderr, "Erro ao obter o gerenciador de GUI!\n");
        emu_frontend_shutdown(frontend);
        return 1;
    }
    
    // Criar botões
    gui_rect_t exit_button_rect = {700, 20, 80, 30};
    gui_rect_t action_button_rect = {350, 350, 100, 40};
    
    gui_element_id_t exit_button_id = gui_button_create(&exit_button_rect, "Sair");
    gui_element_id_t action_button_id = gui_button_create(&action_button_rect, "Ler Texto");
    
    if (exit_button_id == GUI_INVALID_ID || action_button_id == GUI_INVALID_ID) {
        fprintf(stderr, "Erro ao criar botões!\n");
        emu_frontend_shutdown(frontend);
        return 1;
    }
    
    // Criar labels
    gui_rect_t title_label_rect = {300, 50, 200, 40};
    gui_rect_t info_label_rect = {250, 150, 300, 30};
    gui_rect_t status_label_rect = {20, 550, 300, 30};
    gui_rect_t textbox_label_rect = {250, 250, 300, 30};
    
    gui_element_id_t title_label_id = gui_label_create(&title_label_rect, "MEGA EMU GUI DEMO");
    gui_element_id_t info_label_id = gui_label_create(&info_label_rect, "Demonstração de widgets da GUI");
    gui_element_id_t status_label_id = gui_label_create(&status_label_rect, "Status: Pronto");
    gui_element_id_t textbox_label_id = gui_label_create(&textbox_label_rect, "Digite algo na caixa de texto:");
    
    if (title_label_id == GUI_INVALID_ID || info_label_id == GUI_INVALID_ID || 
        status_label_id == GUI_INVALID_ID || textbox_label_id == GUI_INVALID_ID) {
        fprintf(stderr, "Erro ao criar labels!\n");
        emu_frontend_shutdown(frontend);
        return 1;
    }
    
    // Criar caixa de texto
    gui_rect_t textbox_rect = {250, 300, 300, 30};
    gui_element_id_t textbox_id = gui_textbox_create(&textbox_rect, "Texto inicial");
    
    if (textbox_id == GUI_INVALID_ID) {
        fprintf(stderr, "Erro ao criar caixa de texto!\n");
        emu_frontend_shutdown(frontend);
        return 1;
    }
    
    // Configurar labels
    gui_color_t title_color = {255, 255, 0, 255}; // Amarelo
    gui_color_t info_color = {200, 200, 200, 255}; // Cinza claro
    gui_color_t status_color = {0, 255, 0, 255}; // Verde
    
    gui_label_set_text_color(title_label_id, &title_color);
    gui_label_set_text_color(info_label_id, &info_color);
    gui_label_set_text_color(status_label_id, &status_color);
    gui_label_set_text_color(textbox_label_id, &info_color);
    
    // Configurar alinhamentos
    gui_label_set_h_alignment(title_label_id, 1); // Centro
    gui_label_set_h_alignment(info_label_id, 1); // Centro
    gui_label_set_h_alignment(textbox_label_id, 1); // Centro
    
    // Configurar caixa de texto
    gui_color_t textbox_bg_color = {230, 230, 230, 255}; // Cinza claro
    gui_color_t textbox_text_color = {0, 0, 0, 255}; // Preto
    gui_color_t textbox_border_color = {100, 100, 100, 255}; // Cinza
    
    gui_textbox_set_background_color(textbox_id, &textbox_bg_color);
    gui_textbox_set_text_color(textbox_id, &textbox_text_color);
    gui_textbox_set_border_color(textbox_id, &textbox_border_color);
    gui_textbox_set_border_width(textbox_id, 2);
    gui_textbox_set_max_length(textbox_id, 50);
    
    // Definir callback para mudança de texto
    gui_textbox_set_text_change_callback(textbox_id, textbox_change_callback, &status_label_id);
    
    // Configurar callbacks dos botões
    gui_button_set_click_callback(exit_button_id, exit_button_callback, &running);
    gui_button_set_click_callback(action_button_id, action_button_callback, &textbox_id);
    
    // Configurar cores dos botões
    gui_color_t exit_button_color = {180, 60, 60, 255};
    gui_color_t action_button_color = {60, 120, 180, 255};
    
    gui_button_set_background_color(exit_button_id, &exit_button_color);
    gui_button_set_background_color(action_button_id, &action_button_color);
    
    printf("Demonstração de GUI iniciada. Interaja com os widgets:\n");
    printf("- Caixa de texto: Digite para testar a entrada de texto\n");
    printf("- Botão 'Ler Texto': Exibe o texto atual da caixa de texto\n");
    printf("- Botão 'Sair': Encerra a aplicação\n");
    
    // Loop principal
    while (running) {
        // Processar eventos
        running = emu_frontend_process_events(frontend);
        
        // Atualizar janela
        emu_frontend_update_window(frontend);
        
        // Pequeno delay para não sobrecarregar a CPU
        SDL_Delay(16); // ~60 FPS
    }
    
    // Finalizar frontend
    emu_frontend_shutdown(frontend);
    
    printf("Demonstração de GUI encerrada.\n");
    
    return 0;
}
