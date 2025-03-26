/**
 * @file test_event_system.cpp
 * @brief Testes unitários para o sistema de eventos
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include <gtest/gtest.h>
#include <cstring>
#include "../../src/core/events/events_interface.h"

// Variáveis globais para testes
static int g_callback_count = 0;
static emu_event_type_t g_last_event_type = EMU_EVENT_NONE;
static void* g_last_event_data = nullptr;
static size_t g_last_event_data_size = 0;

// Callback de teste para eventos
void test_event_callback(emu_event_t *event, void *userdata) {
    g_callback_count++;
    g_last_event_type = event->type;
    g_last_event_data = event->data;
    g_last_event_data_size = event->data_size;
}

// Fixture para testes do sistema de eventos
class EventSystemTest : public ::testing::Test {
protected:
    const emu_events_interface_t *events_interface;
    
    void SetUp() override {
        // Obter interface do sistema de eventos
        events_interface = emu_events_get_interface();
        ASSERT_NE(events_interface, nullptr);
        
        // Inicializar o sistema de eventos
        ASSERT_EQ(events_interface->init(), 0);
        
        // Resetar variáveis globais
        g_callback_count = 0;
        g_last_event_type = EMU_EVENT_NONE;
        g_last_event_data = nullptr;
        g_last_event_data_size = 0;
    }
    
    void TearDown() override {
        // Desligar o sistema de eventos
        events_interface->shutdown();
    }
};

// Teste de inicialização
TEST_F(EventSystemTest, Initialization) {
    // Verificar se a inicialização foi bem-sucedida
    // (já testado no SetUp)
    
    // Testar desligamento e reinicialização
    events_interface->shutdown();
    EXPECT_EQ(events_interface->init(), 0);
}

// Teste de registro de callback
TEST_F(EventSystemTest, RegisterCallback) {
    // Registrar callback para evento de frame start
    int32_t result = events_interface->register_callback(
        EMU_EVENT_FRAME_START, 
        test_event_callback, 
        nullptr
    );
    EXPECT_EQ(result, 0);
    
    // Registrar callback para evento de frame end
    result = events_interface->register_callback(
        EMU_EVENT_FRAME_END, 
        test_event_callback, 
        nullptr
    );
    EXPECT_EQ(result, 0);
}

// Teste de disparo de eventos
TEST_F(EventSystemTest, TriggerEvent) {
    // Registrar callback para evento de frame start
    EXPECT_EQ(events_interface->register_callback(
        EMU_EVENT_FRAME_START, 
        test_event_callback, 
        nullptr
    ), 0);
    
    // Disparar evento sem dados
    EXPECT_EQ(events_interface->trigger_event(
        EMU_EVENT_FRAME_START, 
        nullptr, 
        0
    ), 0);
    
    // Processar eventos
    EXPECT_EQ(events_interface->process_events(), 0);
    
    // Verificar se o callback foi chamado
    EXPECT_EQ(g_callback_count, 1);
    EXPECT_EQ(g_last_event_type, EMU_EVENT_FRAME_START);
    EXPECT_EQ(g_last_event_data, nullptr);
    EXPECT_EQ(g_last_event_data_size, 0);
    
    // Disparar evento com dados
    uint32_t test_data = 0x12345678;
    EXPECT_EQ(events_interface->trigger_event(
        EMU_EVENT_FRAME_START, 
        &test_data, 
        sizeof(test_data)
    ), 0);
    
    // Processar eventos
    EXPECT_EQ(events_interface->process_events(), 0);
    
    // Verificar se o callback foi chamado
    EXPECT_EQ(g_callback_count, 2);
    EXPECT_EQ(g_last_event_type, EMU_EVENT_FRAME_START);
    EXPECT_EQ(*(uint32_t*)g_last_event_data, test_data);
    EXPECT_EQ(g_last_event_data_size, sizeof(test_data));
}

// Teste de cancelamento de registro
TEST_F(EventSystemTest, UnregisterCallback) {
    // Registrar callback para evento de frame start
    EXPECT_EQ(events_interface->register_callback(
        EMU_EVENT_FRAME_START, 
        test_event_callback, 
        nullptr
    ), 0);
    
    // Cancelar registro
    EXPECT_EQ(events_interface->unregister_callback(
        EMU_EVENT_FRAME_START, 
        test_event_callback
    ), 0);
    
    // Disparar evento
    EXPECT_EQ(events_interface->trigger_event(
        EMU_EVENT_FRAME_START, 
        nullptr, 
        0
    ), 0);
    
    // Processar eventos
    EXPECT_EQ(events_interface->process_events(), 0);
    
    // Verificar se o callback NÃO foi chamado
    EXPECT_EQ(g_callback_count, 0);
}

// Teste de múltiplos callbacks
TEST_F(EventSystemTest, MultipleCallbacks) {
    static int callback2_count = 0;
    
    // Callback adicional para teste
    auto test_event_callback2 = [](emu_event_t *event, void *userdata) {
        callback2_count++;
    };
    
    // Registrar dois callbacks para o mesmo evento
    EXPECT_EQ(events_interface->register_callback(
        EMU_EVENT_FRAME_START, 
        test_event_callback, 
        nullptr
    ), 0);
    
    EXPECT_EQ(events_interface->register_callback(
        EMU_EVENT_FRAME_START, 
        [](emu_event_t *event, void *userdata) {
            callback2_count++;
        }, 
        nullptr
    ), 0);
    
    // Disparar evento
    EXPECT_EQ(events_interface->trigger_event(
        EMU_EVENT_FRAME_START, 
        nullptr, 
        0
    ), 0);
    
    // Processar eventos
    EXPECT_EQ(events_interface->process_events(), 0);
    
    // Verificar se ambos os callbacks foram chamados
    EXPECT_EQ(g_callback_count, 1);
    EXPECT_EQ(callback2_count, 1);
}

// Teste de controle (pause, resume, reset)
TEST_F(EventSystemTest, ControlFunctions) {
    // Registrar callbacks para eventos de controle
    EXPECT_EQ(events_interface->register_callback(
        EMU_EVENT_PAUSE, 
        test_event_callback, 
        nullptr
    ), 0);
    
    EXPECT_EQ(events_interface->register_callback(
        EMU_EVENT_RESUME, 
        test_event_callback, 
        nullptr
    ), 0);
    
    EXPECT_EQ(events_interface->register_callback(
        EMU_EVENT_RESET, 
        test_event_callback, 
        nullptr
    ), 0);
    
    // Testar funções de controle
    events_interface->pause();
    EXPECT_EQ(events_interface->process_events(), 0);
    EXPECT_EQ(g_callback_count, 1);
    EXPECT_EQ(g_last_event_type, EMU_EVENT_PAUSE);
    
    events_interface->resume();
    EXPECT_EQ(events_interface->process_events(), 0);
    EXPECT_EQ(g_callback_count, 2);
    EXPECT_EQ(g_last_event_type, EMU_EVENT_RESUME);
    
    events_interface->reset();
    EXPECT_EQ(events_interface->process_events(), 0);
    EXPECT_EQ(g_callback_count, 3);
    EXPECT_EQ(g_last_event_type, EMU_EVENT_RESET);
}

// Teste de eventos específicos do emulador
TEST_F(EventSystemTest, EmulatorSpecificEvents) {
    // Registrar callbacks para eventos específicos
    EXPECT_EQ(events_interface->register_callback(
        EMU_EVENT_VBLANK, 
        test_event_callback, 
        nullptr
    ), 0);
    
    EXPECT_EQ(events_interface->register_callback(
        EMU_EVENT_HBLANK, 
        test_event_callback, 
        nullptr
    ), 0);
    
    EXPECT_EQ(events_interface->register_callback(
        EMU_EVENT_CPU_STEP, 
        test_event_callback, 
        nullptr
    ), 0);
    
    // Disparar eventos específicos
    EXPECT_EQ(events_interface->trigger_event(
        EMU_EVENT_VBLANK, 
        nullptr, 
        0
    ), 0);
    
    EXPECT_EQ(events_interface->process_events(), 0);
    EXPECT_EQ(g_callback_count, 1);
    EXPECT_EQ(g_last_event_type, EMU_EVENT_VBLANK);
    
    EXPECT_EQ(events_interface->trigger_event(
        EMU_EVENT_HBLANK, 
        nullptr, 
        0
    ), 0);
    
    EXPECT_EQ(events_interface->process_events(), 0);
    EXPECT_EQ(g_callback_count, 2);
    EXPECT_EQ(g_last_event_type, EMU_EVENT_HBLANK);
    
    EXPECT_EQ(events_interface->trigger_event(
        EMU_EVENT_CPU_STEP, 
        nullptr, 
        0
    ), 0);
    
    EXPECT_EQ(events_interface->process_events(), 0);
    EXPECT_EQ(g_callback_count, 3);
    EXPECT_EQ(g_last_event_type, EMU_EVENT_CPU_STEP);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
