#include "ppu_2c02_adapter.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Mock de memória para testes
static uint8_t mock_vram[0x4000];
static uint8_t mock_palette[0x20];
static uint8_t mock_oam[0x100];

static uint8_t mock_read(void *ctx, uint32_t addr) {
    (void)ctx;
    if (addr < 0x2000) {
        return mock_vram[addr];
    }
    else if (addr < 0x3F00) {
        return mock_vram[addr & 0x2FFF];
    }
    else if (addr < 0x4000) {
        return mock_palette[addr & 0x1F];
    }
    return 0;
}

static void mock_write(void *ctx, uint32_t addr, uint8_t val) {
    (void)ctx;
    if (addr < 0x2000) {
        mock_vram[addr] = val;
    }
    else if (addr < 0x3F00) {
        mock_vram[addr & 0x2FFF] = val;
    }
    else if (addr < 0x4000) {
        mock_palette[addr & 0x1F] = val;
    }
}

// Testes
static void test_interface_creation(void) {
    printf("Testando criação da interface...\n");

    ppu_interface_t *ppu = ppu_2c02_create_interface();
    assert(ppu != NULL);
    assert(ppu->context != NULL);

    ppu->shutdown(ppu->context);
    free(ppu);

    printf("OK!\n");
}

static void test_initialization(void) {
    printf("Testando inicialização da PPU...\n");

    ppu_interface_t *ppu = ppu_2c02_create_interface();
    assert(ppu != NULL);

    ppu_config_t config = {
        .read_mem = mock_read,
        .write_mem = mock_write,
        .context = NULL,
        .log_level = 0,
        .screen_width = 256,
        .screen_height = 240
    };

    int32_t result = ppu->init(ppu->context, &config);
    assert(result == PPU_ERROR_NONE);

    ppu->shutdown(ppu->context);
    free(ppu);

    printf("OK!\n");
}

static void test_reset(void) {
    printf("Testando reset da PPU...\n");

    ppu_interface_t *ppu = ppu_2c02_create_interface();
    assert(ppu != NULL);

    ppu_config_t config = {
        .read_mem = mock_read,
        .write_mem = mock_write,
        .context = NULL,
        .log_level = 0,
        .screen_width = 256,
        .screen_height = 240
    };

    ppu->init(ppu->context, &config);
    ppu->reset(ppu->context);

    // Verifica estado após reset
    ppu_state_t state;
    ppu->get_state(ppu->context, &state);

    assert(state.scanline == 0);
    assert(state.cycle == 0);
    assert(state.frame == 0);
    assert(state.flags == 0);

    ppu->shutdown(ppu->context);
    free(ppu);

    printf("OK!\n");
}

static void test_registers(void) {
    printf("Testando registradores da PPU...\n");

    ppu_interface_t *ppu = ppu_2c02_create_interface();
    assert(ppu != NULL);

    ppu_config_t config = {
        .read_mem = mock_read,
        .write_mem = mock_write,
        .context = NULL,
        .log_level = 0,
        .screen_width = 256,
        .screen_height = 240
    };

    ppu->init(ppu->context, &config);

    // Testa escrita/leitura de registradores
    ppu->write_register(ppu->context, 0, 0x80); // PPUCTRL
    ppu->write_register(ppu->context, 1, 0x1E); // PPUMASK

    uint8_t ctrl = ppu->read_register(ppu->context, 0);
    uint8_t mask = ppu->read_register(ppu->context, 1);

    assert(ctrl == 0x80);
    assert(mask == 0x1E);

    ppu->shutdown(ppu->context);
    free(ppu);

    printf("OK!\n");
}

static void test_vram_access(void) {
    printf("Testando acesso à VRAM...\n");

    ppu_interface_t *ppu = ppu_2c02_create_interface();
    assert(ppu != NULL);

    ppu_config_t config = {
        .read_mem = mock_read,
        .write_mem = mock_write,
        .context = NULL,
        .log_level = 0,
        .screen_width = 256,
        .screen_height = 240
    };

    ppu->init(ppu->context, &config);

    // Testa escrita/leitura de VRAM
    ppu->write_vram(ppu->context, 0x2000, 0x42);
    uint8_t val = ppu->read_vram(ppu->context, 0x2000);
    assert(val == 0x42);

    // Testa espelhamento
    val = ppu->read_vram(ppu->context, 0x2800);
    assert(val == 0x42);

    ppu->shutdown(ppu->context);
    free(ppu);

    printf("OK!\n");
}

static void test_palette_access(void) {
    printf("Testando acesso à paleta...\n");

    ppu_interface_t *ppu = ppu_2c02_create_interface();
    assert(ppu != NULL);

    ppu_config_t config = {
        .read_mem = mock_read,
        .write_mem = mock_write,
        .context = NULL,
        .log_level = 0,
        .screen_width = 256,
        .screen_height = 240
    };

    ppu->init(ppu->context, &config);

    // Testa escrita/leitura de paleta
    ppu->write_palette(ppu->context, 0, 0x2C);
    uint8_t val = ppu->read_palette(ppu->context, 0);
    assert(val == 0x2C);

    ppu->shutdown(ppu->context);
    free(ppu);

    printf("OK!\n");
}

static void test_oam_access(void) {
    printf("Testando acesso à OAM...\n");

    ppu_interface_t *ppu = ppu_2c02_create_interface();
    assert(ppu != NULL);

    ppu_config_t config = {
        .read_mem = mock_read,
        .write_mem = mock_write,
        .context = NULL,
        .log_level = 0,
        .screen_width = 256,
        .screen_height = 240
    };

    ppu->init(ppu->context, &config);

    // Testa escrita/leitura de OAM
    ppu->write_oam(ppu->context, 0, 0x55);
    uint8_t val = ppu->read_oam(ppu->context, 0);
    assert(val == 0x55);

    // Testa DMA
    uint8_t dma_data[256];
    for (int i = 0; i < 256; i++) {
        dma_data[i] = i;
    }

    ppu->dma_write(ppu->context, dma_data);

    for (int i = 0; i < 256; i++) {
        val = ppu->read_oam(ppu->context, i);
        assert(val == i);
    }

    ppu->shutdown(ppu->context);
    free(ppu);

    printf("OK!\n");
}

static void test_frame_rendering(void) {
    printf("Testando renderização de frame...\n");

    ppu_interface_t *ppu = ppu_2c02_create_interface();
    assert(ppu != NULL);

    ppu_config_t config = {
        .read_mem = mock_read,
        .write_mem = mock_write,
        .context = NULL,
        .log_level = 0,
        .screen_width = 256,
        .screen_height = 240
    };

    ppu->init(ppu->context, &config);

    // Configura PPU para renderização
    ppu->write_register(ppu->context, 0, 0x80); // PPUCTRL - NMI habilitado
    ppu->write_register(ppu->context, 1, 0x1E); // PPUMASK - Rendering habilitado

    // Executa um frame completo
    for (int i = 0; i < 89342; i++) {
        ppu->execute(ppu->context, 1);
    }

    // Verifica estado após o frame
    ppu_state_t state;
    ppu->get_state(ppu->context, &state);

    assert(state.frame == 1);
    assert((state.flags & PPU_FLAG_VBLANK) != 0);

    // Verifica frame buffer
    const uint32_t *frame_buffer = ppu->get_frame_buffer(ppu->context);
    assert(frame_buffer != NULL);

    ppu->shutdown(ppu->context);
    free(ppu);

    printf("OK!\n");
}

int main(void) {
    printf("Iniciando testes do adaptador PPU 2C02...\n\n");

    test_interface_creation();
    test_initialization();
    test_reset();
    test_registers();
    test_vram_access();
    test_palette_access();
    test_oam_access();
    test_frame_rendering();

    printf("\nTodos os testes passaram!\n");
    return 0;
}
