#include "state_interface.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Estruturas de mock para teste
typedef struct {
    uint32_t registers[16];
    uint32_t pc;
    uint32_t sp;
    uint32_t flags;
} mock_cpu_state_t;

typedef struct {
    uint8_t vram[16384];
    uint8_t palette[32];
    uint8_t oam[256];
    uint32_t registers[8];
} mock_ppu_state_t;

typedef struct {
    uint8_t ram[2048];
    uint8_t cart_ram[8192];
} mock_memory_state_t;

// Diretório temporário para testes
#define TEST_DIR "test_saves"
#define TEST_FILE "test_saves/test.sav"

// Funções auxiliares
static void create_test_dir(void) {
#ifdef _WIN32
    system("mkdir test_saves 2> nul");
#else
    system("mkdir -p test_saves");
#endif
}

static void cleanup_test_dir(void) {
#ifdef _WIN32
    system("rmdir /s /q test_saves 2> nul");
#else
    system("rm -rf test_saves");
#endif
}

// Testes
static void test_interface_creation(void) {
    printf("Testando criação da interface...\n");

    state_interface_t *interface = create_state_interface();
    assert(interface != NULL);
    assert(interface->context != NULL);

    destroy_state_interface(interface);
    printf("OK\n");
}

static void test_initialization(void) {
    printf("Testando inicialização...\n");

    state_interface_t *interface = create_state_interface();
    state_config_t config = {
        .save_dir = TEST_DIR,
        .compress = false,
        .encrypt = false,
        .include_screenshot = true,
        .log_level = 0
    };

    assert(interface->init(interface->context, &config) == STATE_ERROR_NONE);

    destroy_state_interface(interface);
    printf("OK\n");
}

static void test_save_load_state(void) {
    printf("Testando save/load de estado...\n");

    state_interface_t *interface = create_state_interface();
    state_config_t config = {
        .save_dir = TEST_DIR,
        .compress = false,
        .encrypt = false,
        .include_screenshot = true,
        .log_level = 0
    };

    interface->init(interface->context, &config);

    // Cria estados mock
    mock_cpu_state_t cpu_state = {
        .registers = {1, 2, 3, 4, 5, 6, 7, 8},
        .pc = 0x1000,
        .sp = 0x100,
        .flags = 0x55
    };

    mock_ppu_state_t ppu_state = {0};
    for (int i = 0; i < 16384; i++) ppu_state.vram[i] = i & 0xFF;
    for (int i = 0; i < 32; i++) ppu_state.palette[i] = i;
    for (int i = 0; i < 256; i++) ppu_state.oam[i] = i;
    for (int i = 0; i < 8; i++) ppu_state.registers[i] = i;

    mock_memory_state_t memory_state = {0};
    for (int i = 0; i < 2048; i++) memory_state.ram[i] = i & 0xFF;
    for (int i = 0; i < 8192; i++) memory_state.cart_ram[i] = i & 0xFF;

    // Salva os estados
    assert(interface->save_cpu_state(interface->context, &cpu_state, sizeof(cpu_state)));
    assert(interface->save_ppu_state(interface->context, &ppu_state, sizeof(ppu_state)));
    assert(interface->save_memory_state(interface->context, &memory_state, sizeof(memory_state)));

    // Salva o arquivo
    assert(interface->save_state(interface->context, TEST_FILE));

    // Limpa os estados
    memset(&cpu_state, 0, sizeof(cpu_state));
    memset(&ppu_state, 0, sizeof(ppu_state));
    memset(&memory_state, 0, sizeof(memory_state));

    // Carrega o arquivo
    assert(interface->load_state(interface->context, TEST_FILE));

    // Carrega os estados
    mock_cpu_state_t loaded_cpu = {0};
    mock_ppu_state_t loaded_ppu = {0};
    mock_memory_state_t loaded_memory = {0};

    assert(interface->load_cpu_state(interface->context, &loaded_cpu, sizeof(loaded_cpu)));
    assert(interface->load_ppu_state(interface->context, &loaded_ppu, sizeof(loaded_ppu)));
    assert(interface->load_memory_state(interface->context, &loaded_memory, sizeof(loaded_memory)));

    // Verifica os dados
    assert(memcmp(&loaded_cpu, &cpu_state, sizeof(cpu_state)) == 0);
    assert(memcmp(&loaded_ppu, &ppu_state, sizeof(ppu_state)) == 0);
    assert(memcmp(&loaded_memory, &memory_state, sizeof(memory_state)) == 0);

    destroy_state_interface(interface);
    printf("OK\n");
}

static void test_quick_save_load(void) {
    printf("Testando quick save/load...\n");

    state_interface_t *interface = create_state_interface();
    state_config_t config = {
        .save_dir = TEST_DIR,
        .compress = false,
        .encrypt = false,
        .include_screenshot = false,
        .log_level = 0
    };

    interface->init(interface->context, &config);

    mock_cpu_state_t cpu_state = {
        .registers = {1, 2, 3, 4},
        .pc = 0x2000,
        .sp = 0x200,
        .flags = 0xAA
    };

    // Testa vários slots
    for (int slot = 0; slot < 10; slot++) {
        cpu_state.pc = 0x2000 + slot;
        assert(interface->save_cpu_state(interface->context, &cpu_state, sizeof(cpu_state)));
        assert(interface->quick_save(interface->context, slot));

        memset(&cpu_state, 0, sizeof(cpu_state));
        assert(interface->quick_load(interface->context, slot));

        mock_cpu_state_t loaded_cpu = {0};
        assert(interface->load_cpu_state(interface->context, &loaded_cpu, sizeof(loaded_cpu)));
        assert(loaded_cpu.pc == 0x2000 + slot);
    }

    destroy_state_interface(interface);
    printf("OK\n");
}

static void test_metadata(void) {
    printf("Testando metadados...\n");

    state_interface_t *interface = create_state_interface();
    state_config_t config = {
        .save_dir = TEST_DIR,
        .compress = false,
        .encrypt = false,
        .include_screenshot = true,
        .log_level = 0
    };

    interface->init(interface->context, &config);

    state_metadata_t metadata = {0};
    strncpy(metadata.title, "Test Save", sizeof(metadata.title) - 1);
    strncpy(metadata.description, "Test Description", sizeof(metadata.description) - 1);
    strncpy(metadata.author, "Test Author", sizeof(metadata.author) - 1);
    strncpy(metadata.version, "1.0.0", sizeof(metadata.version) - 1);

    assert(interface->set_metadata(interface->context, &metadata));

    state_metadata_t loaded_metadata = {0};
    assert(interface->get_metadata(interface->context, &loaded_metadata));

    assert(strcmp(loaded_metadata.title, metadata.title) == 0);
    assert(strcmp(loaded_metadata.description, metadata.description) == 0);
    assert(strcmp(loaded_metadata.author, metadata.author) == 0);
    assert(strcmp(loaded_metadata.version, metadata.version) == 0);

    destroy_state_interface(interface);
    printf("OK\n");
}

static void test_header_info(void) {
    printf("Testando informações de cabeçalho...\n");

    state_interface_t *interface = create_state_interface();
    state_config_t config = {
        .save_dir = TEST_DIR,
        .compress = false,
        .encrypt = false,
        .include_screenshot = false,
        .log_level = 0
    };

    interface->init(interface->context, &config);

    mock_cpu_state_t cpu_state = {0};
    assert(interface->save_cpu_state(interface->context, &cpu_state, sizeof(cpu_state)));
    assert(interface->save_state(interface->context, TEST_FILE));

    state_header_t header = {0};
    assert(interface->get_header(interface->context, &header));

    assert(memcmp(header.magic, "SAVE", 4) == 0);
    assert(header.version_major == STATE_VERSION_MAJOR);
    assert(header.version_minor == STATE_VERSION_MINOR);
    assert(header.size > 0);
    assert(header.checksum != 0);

    destroy_state_interface(interface);
    printf("OK\n");
}

static void test_state_verification(void) {
    printf("Testando verificação de estado...\n");

    state_interface_t *interface = create_state_interface();
    state_config_t config = {
        .save_dir = TEST_DIR,
        .compress = false,
        .encrypt = false,
        .include_screenshot = false,
        .log_level = 0
    };

    interface->init(interface->context, &config);

    // Cria um arquivo de estado válido
    mock_cpu_state_t cpu_state = {0};
    assert(interface->save_cpu_state(interface->context, &cpu_state, sizeof(cpu_state)));
    assert(interface->save_state(interface->context, TEST_FILE));

    // Verifica o arquivo
    assert(interface->verify_state(interface->context, TEST_FILE));

    // Tenta verificar um arquivo que não existe
    assert(!interface->verify_state(interface->context, "nonexistent.sav"));

    destroy_state_interface(interface);
    printf("OK\n");
}

static void test_state_comparison(void) {
    printf("Testando comparação de estados...\n");

    state_interface_t *interface = create_state_interface();
    state_config_t config = {
        .save_dir = TEST_DIR,
        .compress = false,
        .encrypt = false,
        .include_screenshot = false,
        .log_level = 0
    };

    interface->init(interface->context, &config);

    // Cria dois arquivos de estado idênticos
    mock_cpu_state_t cpu_state = {
        .registers = {1, 2, 3, 4},
        .pc = 0x1000,
        .sp = 0x100,
        .flags = 0x55
    };

    assert(interface->save_cpu_state(interface->context, &cpu_state, sizeof(cpu_state)));
    assert(interface->save_state(interface->context, TEST_FILE));
    assert(interface->save_state(interface->context, "test_saves/test2.sav"));

    // Compara os arquivos
    assert(interface->compare_states(interface->context, TEST_FILE, "test_saves/test2.sav"));

    // Modifica um dos estados
    cpu_state.pc = 0x2000;
    assert(interface->save_cpu_state(interface->context, &cpu_state, sizeof(cpu_state)));
    assert(interface->save_state(interface->context, "test_saves/test2.sav"));

    // Compara novamente
    assert(!interface->compare_states(interface->context, TEST_FILE, "test_saves/test2.sav"));

    destroy_state_interface(interface);
    printf("OK\n");
}

int main(void) {
    printf("Iniciando testes da interface de save states...\n\n");

    create_test_dir();

    test_interface_creation();
    test_initialization();
    test_save_load_state();
    test_quick_save_load();
    test_metadata();
    test_header_info();
    test_state_verification();
    test_state_comparison();

    cleanup_test_dir();

    printf("\nTodos os testes passaram com sucesso!\n");
    return 0;
}
