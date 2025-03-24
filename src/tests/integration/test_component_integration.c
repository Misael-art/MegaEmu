/** * @file test_component_integration.c * @brief Testes de integração para os componentes do emulador * * Este arquivo contém testes que verificam a integração correta entre * os diferentes componentes do emulador usando as novas interfaces. */ #include<stdio.h> #include<stdlib.h> #include<string.h> #include "../../core/public/component_interfaces.h" #include "../../utils/error_handling.h" #include "../../utils/enhanced_log.h" // Contadores de testestatic int test_result_count = 0;static int test_pass_count = 0;/** * @brief Função auxiliar para reportar resultados de teste */static void report_test(const char *test_name, int result){    test_result_count++;    if (result)    {        printf("[PASS] %s\n", test_name);        test_pass_count++;    }    else    {        printf("[FAIL] %s\n", test_name);    }}// Estruturas de contexto para componentes mocktypedef struct{    int initialized;    int reset_count;    int shutdown_called;    int cycles_executed;    uint32_t registers[16];} mock_cpu_context_t;typedef struct{    int initialized;    int reset_count;    int shutdown_called;    uint8_t *memory;    size_t memory_size;} mock_memory_context_t;// Mock para funções da interface de CPUstatic int mock_cpu_init(void *context){    mock_cpu_context_t *cpu = (mock_cpu_context_t *)context;    cpu->initialized = 1;    return 0;}static void mock_cpu_shutdown(void *context){    mock_cpu_context_t *cpu = (mock_cpu_context_t *)context;    cpu->shutdown_called = 1;}static void mock_cpu_reset(void *context){    mock_cpu_context_t *cpu = (mock_cpu_context_t *)context;    cpu->reset_count++;    cpu->cycles_executed = 0;    memset(cpu->registers, 0, sizeof(cpu->registers));}static const char *mock_cpu_get_name(void *context){    return "MockCPU";}static const char *mock_cpu_get_version(void *context){    return "1.0.0";}static int mock_cpu_run_cycles(void *context, int cycles){    mock_cpu_context_t *cpu = (mock_cpu_context_t *)context;    cpu->cycles_executed += cycles;    return 0;}static uint32_t mock_cpu_read_reg(void *context, int reg_id){    mock_cpu_context_t *cpu = (mock_cpu_context_t *)context;    if (reg_id >= 0 && reg_id < 16)    {        return cpu->registers[reg_id];    }    return 0;}static void mock_cpu_write_reg(void *context, int reg_id, uint32_t value){    mock_cpu_context_t *cpu = (mock_cpu_context_t *)context;    if (reg_id >= 0 && reg_id < 16)    {        cpu->registers[reg_id] = value;    }}// Mock para funções da interface de memóriastatic int mock_memory_init(void *context){    mock_memory_context_t *mem = (mock_memory_context_t *)context;    mem->initialized = 1;    // Alocar memória de teste    mem->memory_size = 1024 * 64; // 64KB    mem->memory = (uint8_t *)malloc(mem->memory_size);    if (mem->memory == NULL)    {        return EMU_ERROR_OUT_OF_MEMORY;    }    memset(mem->memory, 0, mem->memory_size);    return 0;}static void mock_memory_shutdown(void *context){    mock_memory_context_t *mem = (mock_memory_context_t *)context;    mem->shutdown_called = 1;    if (mem->memory != NULL)    {        free(mem->memory);        mem->memory = NULL;    }}static void mock_memory_reset(void *context){    mock_memory_context_t *mem = (mock_memory_context_t *)context;    mem->reset_count++;    if (mem->memory != NULL)    {        memset(mem->memory, 0, mem->memory_size);    }}static const char *mock_memory_get_name(void *context){    return "MockMemory";}static const char *mock_memory_get_version(void *context){    return "1.0.0";}static uint8_t mock_memory_read8(void *context, uint32_t address){    mock_memory_context_t *mem = (mock_memory_context_t *)context;    if (address < mem->memory_size)    {        return mem->memory[address];    }    return 0;}static void mock_memory_write8(void *context, uint32_t address, uint8_t value){    mock_memory_context_t *mem = (mock_memory_context_t *)context;    if (address < mem->memory_size)    {        mem->memory[address] = value;    }}/** * @brief Testa o registro e gerenciamento de componentes */static void test_component_registration(){    // Criar plataforma mock    emu_platform_components_t *platform = (emu_platform_components_t *)calloc(1, sizeof(emu_platform_components_t));    // Criar contextos para CPU e memória    mock_cpu_context_t *cpu_context = (mock_cpu_context_t *)calloc(1, sizeof(mock_cpu_context_t));    mock_memory_context_t *memory_context = (mock_memory_context_t *)calloc(1, sizeof(mock_memory_context_t));    // Criar interfaces para CPU e memória    emu_cpu_interface_t *cpu_interface = (emu_cpu_interface_t *)calloc(1, sizeof(emu_cpu_interface_t));    emu_memory_interface_t *memory_interface = (emu_memory_interface_t *)calloc(1, sizeof(emu_memory_interface_t));    // Configurar interface de CPU    cpu_interface->base.init = mock_cpu_init;    cpu_interface->base.shutdown = mock_cpu_shutdown;    cpu_interface->base.reset = mock_cpu_reset;    cpu_interface->base.get_name = mock_cpu_get_name;    cpu_interface->base.get_version = mock_cpu_get_version;    cpu_interface->run_cycles = mock_cpu_run_cycles;    cpu_interface->read_reg = mock_cpu_read_reg;    cpu_interface->write_reg = mock_cpu_write_reg;    // Configurar interface de memória    memory_interface->base.init = mock_memory_init;    memory_interface->base.shutdown = mock_memory_shutdown;    memory_interface->base.reset = mock_memory_reset;    memory_interface->base.get_name = mock_memory_get_name;    memory_interface->base.get_version = mock_memory_get_version;    memory_interface->read8 = mock_memory_read8;    memory_interface->write8 = mock_memory_write8;    // Registrar componentes    int result_cpu = emu_register_component(platform, EMU_COMPONENT_CPU,                                            cpu_interface, cpu_context);    int result_memory = emu_register_component(platform, EMU_COMPONENT_MEMORY,                                               memory_interface, memory_context);    // Verificar registro    report_test("Registro de CPU", result_cpu == 0);    report_test("Registro de memória", result_memory == 0);    // Inicializar componentes    int init_result = emu_init_all_components(platform);    report_test("Inicialização de componentes", init_result == 0);    // Verificar se os componentes foram inicializados    report_test("CPU inicializada", cpu_context->initialized == 1);    report_test("Memória inicializada", memory_context->initialized == 1);    // Interação entre componentes    mock_cpu_context_t *cpu = (mock_cpu_context_t *)emu_get_component(platform, EMU_COMPONENT_CPU);    mock_memory_context_t *memory = (mock_memory_context_t *)emu_get_component(platform, EMU_COMPONENT_MEMORY);    report_test("Obter contexto da CPU", cpu != NULL);    report_test("Obter contexto da memória", memory != NULL);    // Testar escrita em memória    memory->memory[0x1000] = 0;    memory_interface->write8(memory, 0x1000, 0x42);    report_test("Escrita em memória", memory->memory[0x1000] == 0x42);    // Testar leitura de memória    uint8_t value = memory_interface->read8(memory, 0x1000);    report_test("Leitura de memória", value == 0x42);    // Testar escrita em registrador da CPU    cpu_interface->write_reg(cpu, 0, 0x12345678);    report_test("Escrita em registrador", cpu->registers[0] == 0x12345678);    // Testar leitura de registrador da CPU    uint32_t reg_value = cpu_interface->read_reg(cpu, 0);    report_test("Leitura de registrador", reg_value == 0x12345678);    // Testar execução de ciclos    cpu_interface->run_cycles(cpu, 100);    report_test("Execução de ciclos", cpu->cycles_executed == 100);    // Testar reset    emu_reset_all_components(platform);    report_test("Reset de CPU", cpu->reset_count == 1);    report_test("Reset de memória", memory->reset_count == 1);    report_test("CPU zerada após reset", cpu->cycles_executed == 0);    report_test("Registrador zerado após reset", cpu->registers[0] == 0);    // Testar desligamento    emu_shutdown_all_components(platform);    report_test("Desligamento de CPU", cpu->shutdown_called == 1);    report_test("Desligamento de memória", memory->shutdown_called == 1);    // Liberar recursos    free(cpu_interface);    free(memory_interface);    free(platform);    free(cpu_context);    // Nota: memory_context->memory já foi liberado em mock_memory_shutdown    free(memory_context);}/** * @brief Função principal para execução dos testes */int main(void){    printf("\n=== Testes de Integração de Componentes ===\n");    // Executar testes    test_component_registration();    // Resumo dos resultados    printf("\n=== Resultados ===\n");    printf("Total de testes: %d\n", test_result_count);    printf("Testes com sucesso: %d\n", test_pass_count);    printf("Testes com falha: %d\n", test_result_count - test_pass_count);    return (test_pass_count == test_result_count) ? 0 : 1;}
