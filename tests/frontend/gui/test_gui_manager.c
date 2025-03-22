/**
 * @file test_gui_manager.c
 * @brief Testes unitários para o gerenciador de GUI
 */
#include "unity.h"
#include "../../../src/frontend/gui/core/gui_manager.h"
#include "../../../src/frontend/gui/core/gui_element.h"
#include "../../../src/frontend/gui/core/gui_types.h"
#include <stdlib.h>
#include <string.h>

// Estrutura para mock do SDL_Renderer
typedef struct {
    int dummy;
} MockRenderer;

// Mock do SDL_Renderer para testes
static MockRenderer g_mock_renderer;

// Variáveis globais para testes
static gui_manager_t g_manager = NULL;

// Função de setup executada antes de cada teste
void setUp(void) {
    g_manager = gui_manager_init();
    TEST_ASSERT_NOT_NULL(g_manager);
}

// Função de teardown executada após cada teste
void tearDown(void) {
    if (g_manager) {
        gui_manager_shutdown(g_manager);
        g_manager = NULL;
    }
}

// Teste de inicialização do gerenciador
void test_gui_manager_init(void) {
    TEST_ASSERT_NOT_NULL(g_manager);
}

// Teste de adição de elemento
void test_gui_manager_add_element(void) {
    gui_rect_t rect = {10, 10, 100, 30};
    gui_element_id_t id = gui_manager_add_element(g_manager, GUI_ELEMENT_BUTTON, &rect, "Test Button");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Verificar se o elemento foi adicionado corretamente
    gui_element_t element = gui_manager_get_element(g_manager, id);
    TEST_ASSERT_NOT_NULL(element);
    
    // Verificar propriedades do elemento
    gui_rect_t element_rect;
    gui_element_get_rect(element, &element_rect);
    
    TEST_ASSERT_EQUAL(rect.x, element_rect.x);
    TEST_ASSERT_EQUAL(rect.y, element_rect.y);
    TEST_ASSERT_EQUAL(rect.width, element_rect.width);
    TEST_ASSERT_EQUAL(rect.height, element_rect.height);
    
    const char* text = gui_element_get_text(element);
    TEST_ASSERT_NOT_NULL(text);
    TEST_ASSERT_EQUAL_STRING("Test Button", text);
}

// Teste de remoção de elemento
void test_gui_manager_remove_element(void) {
    gui_rect_t rect = {10, 10, 100, 30};
    gui_element_id_t id = gui_manager_add_element(g_manager, GUI_ELEMENT_BUTTON, &rect, "Test Button");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Verificar se o elemento foi adicionado
    gui_element_t element = gui_manager_get_element(g_manager, id);
    TEST_ASSERT_NOT_NULL(element);
    
    // Remover elemento
    gui_manager_remove_element(g_manager, id);
    
    // Verificar se o elemento foi removido
    element = gui_manager_get_element(g_manager, id);
    TEST_ASSERT_NULL(element);
}

// Teste de processamento de eventos
void test_gui_manager_process_event(void) {
    gui_rect_t rect = {10, 10, 100, 30};
    gui_element_id_t id = gui_manager_add_element(g_manager, GUI_ELEMENT_BUTTON, &rect, "Test Button");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Criar evento de mouse
    gui_event_t event;
    memset(&event, 0, sizeof(gui_event_t));
    
    event.type = GUI_EVENT_MOUSE_MOVE;
    event.mouse.position.x = 15;
    event.mouse.position.y = 15;
    
    // Processar evento
    gui_manager_process_event(g_manager, &event);
    
    // Não podemos verificar diretamente o resultado do processamento de eventos
    // sem um mock mais complexo, mas podemos verificar se a função não falha
    TEST_PASS();
}

// Teste de busca de elemento por posição
void test_gui_manager_find_element_at(void) {
    gui_rect_t rect1 = {10, 10, 100, 30};
    gui_rect_t rect2 = {150, 10, 100, 30};
    
    gui_element_id_t id1 = gui_manager_add_element(g_manager, GUI_ELEMENT_BUTTON, &rect1, "Button 1");
    gui_element_id_t id2 = gui_manager_add_element(g_manager, GUI_ELEMENT_BUTTON, &rect2, "Button 2");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id1);
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id2);
    
    // Testar posição dentro do primeiro botão
    gui_element_id_t found_id = gui_manager_find_element_at(g_manager, 15, 15);
    TEST_ASSERT_EQUAL(id1, found_id);
    
    // Testar posição dentro do segundo botão
    found_id = gui_manager_find_element_at(g_manager, 160, 15);
    TEST_ASSERT_EQUAL(id2, found_id);
    
    // Testar posição fora dos botões
    found_id = gui_manager_find_element_at(g_manager, 300, 300);
    TEST_ASSERT_EQUAL(GUI_INVALID_ID, found_id);
}

// Teste de atualização do gerenciador
void test_gui_manager_update(void) {
    // Adicionar alguns elementos
    gui_rect_t rect1 = {10, 10, 100, 30};
    gui_rect_t rect2 = {150, 10, 100, 30};
    
    gui_element_id_t id1 = gui_manager_add_element(g_manager, GUI_ELEMENT_BUTTON, &rect1, "Button 1");
    gui_element_id_t id2 = gui_manager_add_element(g_manager, GUI_ELEMENT_BUTTON, &rect2, "Button 2");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id1);
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id2);
    
    // Atualizar gerenciador
    gui_manager_update(g_manager);
    
    // Não podemos verificar diretamente o resultado da atualização
    // sem um mock mais complexo, mas podemos verificar se a função não falha
    TEST_PASS();
}

// Teste de renderização do gerenciador
void test_gui_manager_render(void) {
    // Adicionar alguns elementos
    gui_rect_t rect1 = {10, 10, 100, 30};
    gui_rect_t rect2 = {150, 10, 100, 30};
    
    gui_element_id_t id1 = gui_manager_add_element(g_manager, GUI_ELEMENT_BUTTON, &rect1, "Button 1");
    gui_element_id_t id2 = gui_manager_add_element(g_manager, GUI_ELEMENT_BUTTON, &rect2, "Button 2");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id1);
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id2);
    
    // Renderizar gerenciador
    gui_manager_render(g_manager, (SDL_Renderer*)&g_mock_renderer);
    
    // Não podemos verificar diretamente o resultado da renderização
    // sem um mock mais complexo, mas podemos verificar se a função não falha
    TEST_PASS();
}

// Função principal para execução dos testes
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_gui_manager_init);
    RUN_TEST(test_gui_manager_add_element);
    RUN_TEST(test_gui_manager_remove_element);
    RUN_TEST(test_gui_manager_process_event);
    RUN_TEST(test_gui_manager_find_element_at);
    RUN_TEST(test_gui_manager_update);
    RUN_TEST(test_gui_manager_render);
    
    return UNITY_END();
}
