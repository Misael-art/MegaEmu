/**
 * @file test_gui_textbox.c
 * @brief Testes unitários para o widget de caixa de texto
 */
#include "unity.h"
#include "../../../src/frontend/gui/core/gui_manager.h"
#include "../../../src/frontend/gui/core/gui_element.h"
#include "../../../src/frontend/gui/core/gui_types.h"
#include "../../../src/frontend/gui/widgets/gui_textbox.h"
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

// Teste de criação da caixa de texto
void test_gui_textbox_create(void) {
    gui_rect_t rect = {10, 10, 200, 30};
    gui_element_id_t id = gui_textbox_create(&rect, "Test TextBox");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Verificar se o elemento foi criado corretamente
    gui_element_t element = gui_element_get_by_id(id);
    TEST_ASSERT_NOT_NULL(element);
    
    // Verificar tipo do elemento
    TEST_ASSERT_EQUAL(GUI_ELEMENT_TEXTBOX, gui_element_get_type(element));
    
    // Verificar propriedades do elemento
    gui_rect_t element_rect;
    gui_element_get_rect(element, &element_rect);
    
    TEST_ASSERT_EQUAL(rect.x, element_rect.x);
    TEST_ASSERT_EQUAL(rect.y, element_rect.y);
    TEST_ASSERT_EQUAL(rect.width, element_rect.width);
    TEST_ASSERT_EQUAL(rect.height, element_rect.height);
    
    const char* text = gui_element_get_text(element);
    TEST_ASSERT_NOT_NULL(text);
    TEST_ASSERT_EQUAL_STRING("Test TextBox", text);
}

// Teste de definição e obtenção de texto
void test_gui_textbox_set_get_text(void) {
    gui_rect_t rect = {10, 10, 200, 30};
    gui_element_id_t id = gui_textbox_create(&rect, "Initial Text");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Definir novo texto
    bool result = gui_textbox_set_text(id, "Updated Text");
    TEST_ASSERT_TRUE(result);
    
    // Verificar se o texto foi atualizado
    char buffer[256];
    result = gui_textbox_get_text(id, buffer, sizeof(buffer));
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING("Updated Text", buffer);
}

// Teste de configuração da cor do texto
void test_gui_textbox_set_text_color(void) {
    gui_rect_t rect = {10, 10, 200, 30};
    gui_element_id_t id = gui_textbox_create(&rect, "Test TextBox");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Definir cor do texto
    gui_color_t text_color = {255, 0, 0, 255}; // Vermelho
    bool result = gui_textbox_set_text_color(id, &text_color);
    
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
void test_gui_textbox_set_background_color(void) {
    gui_rect_t rect = {10, 10, 200, 30};
    gui_element_id_t id = gui_textbox_create(&rect, "Test TextBox");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Definir cor de fundo
    gui_color_t bg_color = {0, 0, 255, 255}; // Azul
    bool result = gui_textbox_set_background_color(id, &bg_color);
    
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

// Teste de configuração da cor da borda
void test_gui_textbox_set_border_color(void) {
    gui_rect_t rect = {10, 10, 200, 30};
    gui_element_id_t id = gui_textbox_create(&rect, "Test TextBox");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Definir cor da borda
    gui_color_t border_color = {0, 255, 0, 255}; // Verde
    bool result = gui_textbox_set_border_color(id, &border_color);
    
    TEST_ASSERT_TRUE(result);
    
    // Verificar se a cor foi definida corretamente
    gui_element_t element = gui_element_get_by_id(id);
    TEST_ASSERT_NOT_NULL(element);
    
    gui_color_t stored_color;
    result = gui_element_get_property_color(element, "border_color", &stored_color);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(border_color.r, stored_color.r);
    TEST_ASSERT_EQUAL(border_color.g, stored_color.g);
    TEST_ASSERT_EQUAL(border_color.b, stored_color.b);
    TEST_ASSERT_EQUAL(border_color.a, stored_color.a);
}

// Teste de configuração da largura da borda
void test_gui_textbox_set_border_width(void) {
    gui_rect_t rect = {10, 10, 200, 30};
    gui_element_id_t id = gui_textbox_create(&rect, "Test TextBox");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Definir largura da borda
    int border_width = 2;
    bool result = gui_textbox_set_border_width(id, border_width);
    
    TEST_ASSERT_TRUE(result);
    
    // Verificar se a largura foi definida corretamente
    gui_element_t element = gui_element_get_by_id(id);
    TEST_ASSERT_NOT_NULL(element);
    
    int stored_width;
    result = gui_element_get_property_int(element, "border_width", &stored_width);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(border_width, stored_width);
}

// Teste de configuração do tamanho máximo
void test_gui_textbox_set_max_length(void) {
    gui_rect_t rect = {10, 10, 200, 30};
    gui_element_id_t id = gui_textbox_create(&rect, "Test TextBox");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Definir tamanho máximo
    size_t max_length = 50;
    bool result = gui_textbox_set_max_length(id, max_length);
    
    TEST_ASSERT_TRUE(result);
    
    // Verificar se o tamanho máximo foi definido corretamente
    gui_element_t element = gui_element_get_by_id(id);
    TEST_ASSERT_NOT_NULL(element);
    
    size_t stored_max_length;
    result = gui_element_get_property_size_t(element, "max_length", &stored_max_length);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(max_length, stored_max_length);
}

// Teste de configuração do modo somente leitura
void test_gui_textbox_set_read_only(void) {
    gui_rect_t rect = {10, 10, 200, 30};
    gui_element_id_t id = gui_textbox_create(&rect, "Test TextBox");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Definir modo somente leitura
    bool read_only = true;
    bool result = gui_textbox_set_read_only(id, read_only);
    
    TEST_ASSERT_TRUE(result);
    
    // Verificar se o modo somente leitura foi definido corretamente
    gui_element_t element = gui_element_get_by_id(id);
    TEST_ASSERT_NOT_NULL(element);
    
    bool stored_read_only;
    result = gui_element_get_property_bool(element, "read_only", &stored_read_only);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(read_only, stored_read_only);
}

// Teste de configuração do foco
void test_gui_textbox_set_focused(void) {
    gui_rect_t rect = {10, 10, 200, 30};
    gui_element_id_t id = gui_textbox_create(&rect, "Test TextBox");
    
    TEST_ASSERT_NOT_EQUAL(GUI_INVALID_ID, id);
    
    // Definir foco
    bool focused = true;
    bool result = gui_textbox_set_focused(id, focused);
    
    TEST_ASSERT_TRUE(result);
    
    // Verificar se o foco foi definido corretamente
    TEST_ASSERT_TRUE(gui_textbox_is_focused(id));
    
    // Remover foco
    result = gui_textbox_set_focused(id, false);
    TEST_ASSERT_TRUE(result);
    
    // Verificar se o foco foi removido corretamente
    TEST_ASSERT_FALSE(gui_textbox_is_focused(id));
}

// Função principal para execução dos testes
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_gui_textbox_create);
    RUN_TEST(test_gui_textbox_set_get_text);
    RUN_TEST(test_gui_textbox_set_text_color);
    RUN_TEST(test_gui_textbox_set_background_color);
    RUN_TEST(test_gui_textbox_set_border_color);
    RUN_TEST(test_gui_textbox_set_border_width);
    RUN_TEST(test_gui_textbox_set_max_length);
    RUN_TEST(test_gui_textbox_set_read_only);
    RUN_TEST(test_gui_textbox_set_focused);
    
    return UNITY_END();
}
