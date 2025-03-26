/**
 * @file test_waveform_display.c
 * @brief Testes unitários para o módulo de exibição
 */

#include "unity.h"
#include "waveform_display.h"
#include <stdlib.h>
#include <string.h>

static waveform_display_t *display = NULL;
static const uint32_t TEST_WIDTH = 800;
static const uint32_t TEST_HEIGHT = 600;
static const char *TEST_TITLE = "Teste do Visualizador";

void setUp(void) {
    TEST_ASSERT_TRUE(waveform_display_init());
    display = waveform_display_create(TEST_WIDTH, TEST_HEIGHT, TEST_TITLE);
    TEST_ASSERT_NOT_NULL(display);
}

void tearDown(void) {
    if (display) {
        waveform_display_destroy(display);
        display = NULL;
    }
    waveform_display_quit();
}

void test_display_initialization(void) {
    TEST_ASSERT_EQUAL_UINT32(TEST_WIDTH, display->width);
    TEST_ASSERT_EQUAL_UINT32(TEST_HEIGHT, display->height);
    TEST_ASSERT_FALSE(display->fullscreen);
    TEST_ASSERT_NOT_NULL(display->window_handle);
    TEST_ASSERT_NOT_NULL(display->renderer_handle);
    TEST_ASSERT_NOT_NULL(display->texture_handle);
}

void test_display_resize(void) {
    const uint32_t new_width = 1024;
    const uint32_t new_height = 768;

    waveform_display_resize(display, new_width, new_height);

    TEST_ASSERT_EQUAL_UINT32(new_width, display->width);
    TEST_ASSERT_EQUAL_UINT32(new_height, display->height);
    TEST_ASSERT_NOT_NULL(display->texture_handle);
}

void test_display_fullscreen_toggle(void) {
    TEST_ASSERT_FALSE(display->fullscreen);

    waveform_display_toggle_fullscreen(display);
    TEST_ASSERT_TRUE(display->fullscreen);

    waveform_display_toggle_fullscreen(display);
    TEST_ASSERT_FALSE(display->fullscreen);
}

void test_display_update(void) {
    // Cria um framebuffer de teste com um padrão simples
    const size_t buffer_size = TEST_WIDTH * TEST_HEIGHT * 4;
    uint8_t *test_buffer = malloc(buffer_size);
    TEST_ASSERT_NOT_NULL(test_buffer);

    // Preenche com um padrão de gradiente
    for (size_t i = 0; i < buffer_size; i += 4) {
        test_buffer[i] = (uint8_t)(i % 256);        // R
        test_buffer[i + 1] = (uint8_t)((i/4) % 256); // G
        test_buffer[i + 2] = (uint8_t)(255 - (i % 256)); // B
        test_buffer[i + 3] = 255;                   // A
    }

    // Testa a atualização do framebuffer
    waveform_display_update(display, test_buffer, TEST_WIDTH, TEST_HEIGHT);

    // Limpa
    free(test_buffer);
}

void test_display_process_events(void) {
    // Testa o processamento de eventos sem eventos reais
    TEST_ASSERT_TRUE(waveform_display_process_events(display));
}

void test_display_null_handling(void) {
    // Testa o tratamento de ponteiros nulos
    waveform_display_destroy(NULL);
    waveform_display_resize(NULL, 100, 100);
    waveform_display_toggle_fullscreen(NULL);
    waveform_display_update(NULL, NULL, 100, 100);
    TEST_ASSERT_FALSE(waveform_display_process_events(NULL));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_display_initialization);
    RUN_TEST(test_display_resize);
    RUN_TEST(test_display_fullscreen_toggle);
    RUN_TEST(test_display_update);
    RUN_TEST(test_display_process_events);
    RUN_TEST(test_display_null_handling);

    return UNITY_END();
}
