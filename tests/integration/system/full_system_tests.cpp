#include <gtest/gtest.h>
#include "core/cpu/cpu.h"
#include "core/video/ppu.h"
#include "frontend/sdl/sdl_frontend.h"
#include "platforms/megadrive/megadrive.h"

class SystemIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Inicializar componentes do sistema
        system = megadrive_create();
        frontend = sdl_frontend_create();

        // Configurar frontend
        sdl_frontend_init(frontend, 800, 600);

        // Conectar frontend ao sistema
        megadrive_set_frontend(system, frontend);
    }

    void TearDown() override
    {
        // Limpar recursos
        sdl_frontend_destroy(frontend);
        megadrive_destroy(system);
    }

    MegaDrive* system;
    SDLFrontend* frontend;
};

// Teste de inicialização do sistema
TEST_F(SystemIntegrationTest, SystemInitialization)
{
    EXPECT_NE(system, nullptr);
    EXPECT_NE(frontend, nullptr);

    // Verificar estado inicial
    EXPECT_EQ(megadrive_get_state(system), SYSTEM_STATE_STOPPED);
    EXPECT_TRUE(sdl_frontend_is_initialized(frontend));
}

// Teste de carregamento de ROM
TEST_F(SystemIntegrationTest, ROMLoading)
{
    // Carregar ROM de teste
    const char* test_rom = "roms/test.md";
    bool loaded = megadrive_load_rom(system, test_rom);

    EXPECT_TRUE(loaded);
    EXPECT_EQ(megadrive_get_state(system), SYSTEM_STATE_ROM_LOADED);

    // Verificar informações da ROM
    ROMInfo info;
    megadrive_get_rom_info(system, &info);

    EXPECT_STREQ(info.title, "SEGA MEGA DRIVE");
    EXPECT_EQ(info.size, 512);
}

// Teste de execução do sistema
TEST_F(SystemIntegrationTest, SystemExecution)
{
    // Carregar e iniciar ROM
    megadrive_load_rom(system, "roms/test.md");
    megadrive_power_on(system);

    EXPECT_EQ(megadrive_get_state(system), SYSTEM_STATE_RUNNING);

    // Executar alguns frames
    for (int i = 0; i < 60; i++) {
        megadrive_run_frame(system);
    }

    // Verificar estado após execução
    SystemState state = megadrive_get_state(system);
    EXPECT_TRUE(state == SYSTEM_STATE_RUNNING || state == SYSTEM_STATE_PAUSED);
}

// Teste de integração de vídeo
TEST_F(SystemIntegrationTest, VideoIntegration)
{
    megadrive_load_rom(system, "roms/test.md");
    megadrive_power_on(system);

    // Executar um frame
    megadrive_run_frame(system);

    // Verificar frame buffer
    const uint32_t* frame_buffer = sdl_frontend_get_frame_buffer(frontend);
    EXPECT_NE(frame_buffer, nullptr);

    // Verificar dimensões do frame
    int width, height;
    sdl_frontend_get_dimensions(frontend, &width, &height);
    EXPECT_EQ(width, 320);
    EXPECT_EQ(height, 240);
}

// Teste de integração de áudio
TEST_F(SystemIntegrationTest, AudioIntegration)
{
    megadrive_load_rom(system, "roms/test.md");
    megadrive_power_on(system);

    // Verificar estado do áudio
    EXPECT_TRUE(sdl_frontend_is_audio_initialized(frontend));

    // Executar alguns frames para gerar áudio
    for (int i = 0; i < 10; i++) {
        megadrive_run_frame(system);
    }

    // Verificar buffer de áudio
    const int16_t* audio_buffer;
    size_t buffer_size;
    sdl_frontend_get_audio_buffer(frontend, &audio_buffer, &buffer_size);

    EXPECT_NE(audio_buffer, nullptr);
    EXPECT_GT(buffer_size, 0);
}

// Teste de save states
TEST_F(SystemIntegrationTest, SaveStates)
{
    megadrive_load_rom(system, "roms/test.md");
    megadrive_power_on(system);

    // Executar alguns frames
    for (int i = 0; i < 30; i++) {
        megadrive_run_frame(system);
    }

    // Salvar estado
    SaveState state;
    bool saved = megadrive_save_state(system, &state);
    EXPECT_TRUE(saved);

    // Modificar estado
    megadrive_run_frame(system);

    // Restaurar estado
    bool restored = megadrive_load_state(system, &state);
    EXPECT_TRUE(restored);

    // Verificar se o estado foi restaurado corretamente
    SaveState current_state;
    megadrive_save_state(system, &current_state);
    EXPECT_EQ(memcmp(&state, &current_state, sizeof(SaveState)), 0);
}

// Teste de input
TEST_F(SystemIntegrationTest, InputHandling)
{
    megadrive_load_rom(system, "roms/test.md");
    megadrive_power_on(system);

    // Simular input
    InputState input = {0};
    input.buttons[BUTTON_A] = true;
    input.buttons[BUTTON_START] = true;

    sdl_frontend_set_input_state(frontend, &input);
    megadrive_run_frame(system);

    // Verificar se o input foi processado
    const InputState* current_input = megadrive_get_input_state(system);
    EXPECT_TRUE(current_input->buttons[BUTTON_A]);
    EXPECT_TRUE(current_input->buttons[BUTTON_START]);
}

// Teste de performance
TEST_F(SystemIntegrationTest, Performance)
{
    megadrive_load_rom(system, "roms/test.md");
    megadrive_power_on(system);

    // Medir tempo de execução de 60 frames
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 60; i++) {
        megadrive_run_frame(system);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Verificar se a execução está dentro do limite de tempo (16.67ms por frame)
    EXPECT_LT(duration.count(), 1000);  // Menos de 1 segundo para 60 frames
}
