/**
 * @file test_gui_label.c
 * @brief Testes unitários para o widget de label
 */
#include "unity.h"
#include "../../../src/frontend/gui/core/gui_manager.h"
#include "../../../src/frontend/gui/core/gui_element.h"
#include "../../../src/frontend/gui/core/gui_types.h"
#include "../../../src/frontend/gui/widgets/gui_label.h"
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

// Teste de criação do label
void test_gui_label_create(void) {
    gui_rect_t rect = {10, 10, 100, 30};
    gui_element_id_t id = gui_label_create(&rect, "Test Label");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Verificar se o elemento foi criado corretamente
    gui_element_t element = gui_element_get_by_id(id);
    TEST_ASSERT_NOT_NULL(element);
    
    // Verificar tipo do elemento
    TEST_ASSERT_EQUAL(GUI_ELEMENT_LABEL, gui_element_get_type(element));
    
    // Verificar propriedades do elemento
    gui_rect_t element_rect;
    gui_element_get_rect(element, &element_rect);
    
    TEST_ASSERT_EQUAL(rect.x, element_rect.x);
    TEST_ASSERT_EQUAL(rect.y, element_rect.y);
    TEST_ASSERT_EQUAL(rect.width, element_rect.width);
    TEST_ASSERT_EQUAL(rect.height, element_rect.height);
    
    const char* text = gui_element_get_text(element);
    TEST_ASSERT_NOT_NULL(text);
    TEST_ASSERT_EQUAL_STRING("Test Label", text);
}

// Teste de configuração da cor do texto
void test_gui_label_set_text_color(void) {
    gui_rect_t rect = {10, 10, 100, 30};
    gui_element_id_t id = gui_label_create(&rect, "Test Label");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Definir cor do texto
    gui_color_t text_color = {255, 0, 0, 255}; // Vermelho
    bool result = gui_label_set_text_color(id, &text_color);
    
    TEST_ASSERT_TRUE(result);
    
    // Verificar se a cor foi definida corretamente
    gui_element_t element = gui_element_get_by_id(id);
    TEST_ASSERT_NOT_NULL(element);
    
    gui_color_t stored_color;
    result = gui_element_get_property_color(element, "text_color", &stored_color);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(text_color.r, stored_color.r);
    TEST_ASSERT_EQUAL(text_color.g, stored_color.g);
    TEST_ASSERT_EQUAL(text_color.b, stored_color.b);
    TEST_ASSERT_EQUAL(text_color.a, stored_color.a);
}

// Teste de configuração da cor de fundo
void test_gui_label_set_background_color(void) {
    gui_rect_t rect = {10, 10, 100, 30};
    gui_element_id_t id = gui_label_create(&rect, "Test Label");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Definir cor de fundo
    gui_color_t bg_color = {0, 0, 255, 255}; // Azul
    bool result = gui_label_set_background_color(id, &bg_color);
    
    TEST_ASSERT_TRUE(result);
    
    // Verificar se a cor foi definida corretamente
    gui_element_t element = gui_element_get_by_id(id);
    TEST_ASSERT_NOT_NULL(element);
    
    gui_color_t stored_color;
    result = gui_element_get_property_color(element, "bg_color", &stored_color);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(bg_color.r, stored_color.r);
    TEST_ASSERT_EQUAL(bg_color.g, stored_color.g);
    TEST_ASSERT_EQUAL(bg_color.b, stored_color.b);
    TEST_ASSERT_EQUAL(bg_color.a, stored_color.a);
}

// Teste de configuração do alinhamento horizontal
void test_gui_label_set_h_alignment(void) {
    gui_rect_t rect = {10, 10, 100, 30};
    gui_element_id_t id = gui_label_create(&rect, "Test Label");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Definir alinhamento horizontal (centro = 1)
    bool result = gui_label_set_h_alignment(id, 1);
    
    TEST_ASSERT_TRUE(result);
    
    // Verificar se o alinhamento foi definido corretamente
    gui_element_t element = gui_element_get_by_id(id);
    TEST_ASSERT_NOT_NULL(element);
    
    int stored_alignment;
    result = gui_element_get_property_int(element, "h_alignment", &stored_alignment);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(1, stored_alignment);
}

// Teste de configuração do alinhamento vertical
void test_gui_label_set_v_alignment(void) {
    gui_rect_t rect = {10, 10, 100, 30};
    gui_element_id_t id = gui_label_create(&rect, "Test Label");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Definir alinhamento vertical (topo = 0)
    bool result = gui_label_set_v_alignment(id, 0);
    
    TEST_ASSERT_TRUE(result);
    
    // Verificar se o alinhamento foi definido corretamente
    gui_element_t element = gui_element_get_by_id(id);
    TEST_ASSERT_NOT_NULL(element);
    
    int stored_alignment;
    result = gui_element_get_property_int(element, "v_alignment", &stored_alignment);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, stored_alignment);
}

// Teste de configuração da transparência
void test_gui_label_set_transparent(void) {
    gui_rect_t rect = {10, 10, 100, 30};
    gui_element_id_t id = gui_label_create(&rect, "Test Label");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Definir transparência (false = com fundo)
    bool result = gui_label_set_transparent(id, false);
    
    TEST_ASSERT_TRUE(result);
    
    // Verificar se a transparência foi definida corretamente
    gui_element_t element = gui_element_get_by_id(id);
    TEST_ASSERT_NOT_NULL(element);
    
    bool stored_transparent;
    result = gui_element_get_property_bool(element, "transparent", &stored_transparent);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(stored_transparent);
}

// Função principal para execução dos testes
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_gui_label_create);
    RUN_TEST(test_gui_label_set_text_color);
    RUN_TEST(test_gui_label_set_background_color);
    RUN_TEST(test_gui_label_set_h_alignment);
    RUN_TEST(test_gui_label_set_v_alignment);
    RUN_TEST(test_gui_label_set_transparent);
    
    return UNITY_END();
}
