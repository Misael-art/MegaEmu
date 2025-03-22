/** * @file nes.c * @brief Implementação principal para a plataforma NES/Famicom */#include "nes.h"#include <stdlib.h>#include <string.h>#include <stdio.h>#include "../../utils/enhanced_log.h"#include "../../utils/log_categories.h"#include "../../utils/performance.h"#include "core/save_state.h"// Protótipos para componentes internos do NES (serão implementados em arquivos separados)#include "cpu/rp2a03.h"#include "ppu/nes_ppu.h"#include "apu/nes_apu.h"#include "memory/nes_memory.h"#include "input/nes_input.h"#include "cartridge/nes_cartridge.h"#include "cpu/nes_cpu.h"// Definir a categoria de log para a plataforma NES#define EMU_LOG_CAT_PLATFORM EMU_LOG_CAT_NES// Macros de log específicas para a plataforma NES#define NES_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_PLATFORM, __VA_ARGS__)#define NES_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_PLATFORM, __VA_ARGS__)#define NES_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_PLATFORM, __VA_ARGS__)#define NES_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_PLATFORM, __VA_ARGS__)#define NES_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_PLATFORM, __VA_ARGS__)/** * @brief Dimensões da tela do NES */#define NES_SCREEN_WIDTH 256#define NES_SCREEN_HEIGHT 240/** * @brief Estado global do sistema NES */nes_state_t g_nes_state = {0};/** * @brief Contexto de save state para o NES */static save_state_t *g_nes_save_state = NULL;/** * @brief Instância C++ do NES */static void *g_nes_cpp_instance = NULL;/** * @brief Callback chamado quando o estado muda */static void nes_save_state_callback(void *user_data){    NES_LOG_INFO("Estado do NES alterado");    // Atualiza os componentes após um load    if (g_nes_state.cpu)    {        // Possível reset de cache ou outras operações após load    }    if (g_nes_state.ppu)    {        // Atualiza estado interno da PPU, se necessário    }    if (g_nes_state.apu)    {        // Reinicia buffers de áudio, se necessário    }}/** * @brief Inicializa o sistema de save states para o NES * * @return int Código de erro (0 para sucesso) */static int nes_save_state_init(void){    if (!g_nes_state.initialized)    {        NES_LOG_ERROR("Tentativa de inicializar save state com sistema NES não inicializado");        return NES_ERROR_INITIALIZATION;    }    // Libera contexto anterior, se existir    if (g_nes_save_state)    {        save_state_destroy(g_nes_save_state);        g_nes_save_state = NULL;    }    // Calcula o CRC32 da ROM ou usa o já calculado pelo cartucho    uint32_t rom_crc32 = 0;    if (g_nes_state.cartridge)    {        // Usar CRC32 do cartucho se disponível        // rom_crc32 = nes_cartridge_get_crc32(g_nes_state.cartridge);        // Por enquanto, vamos calcular baseado no conteúdo da ROM        if (g_nes_state.rom_info.prg_rom && g_nes_state.rom_info.prg_rom_size > 0)        {            // Cálculo simplificado para exemplo            for (int i = 0; i < g_nes_state.rom_info.prg_rom_size; i++)            {                rom_crc32 = (rom_crc32 + g_nes_state.rom_info.prg_rom[i]) * 1664525 + 1013904223;            }        }    }    // Cria o contexto de save state    g_nes_save_state = save_state_create(        1, // ID da plataforma NES        rom_crc32,        g_nes_state.config.rom_path ? g_nes_state.config.rom_path : "unknown");    if (!g_nes_save_state)    {        LOG_ERROR(EMU_LOG_CAT_PLATFORM, "Falha ao criar contexto de save state");        return NES_ERROR_MEMORY_ALLOCATION;    }    // Registra o callback    save_state_set_callback(g_nes_save_state, nes_save_state_callback, NULL);    LOG_INFO(EMU_LOG_CAT_PLATFORM, "Sistema de save state inicializado");    return NES_ERROR_NONE;}/** * @brief Registra os componentes do NES no sistema de save state * * @return int Código de erro (0 para sucesso) */static int nes_save_state_register_components(void){    if (!g_nes_save_state)    {        LOG_ERROR(EMU_LOG_CAT_PLATFORM, "Contexto de save state não inicializado");        return NES_ERROR_INITIALIZATION;    }    int result = SAVE_STATE_ERROR_NONE;    // Registra CPU se disponível    if (g_nes_state.cpu)    {        // Registra campos específicos da CPU (registradores, PC, flags, etc.)        // A implementação específica depende da estrutura rp2a03_t        // Exemplo (a ser adaptado de acordo com a implementação real):        /*        result = save_state_add_field(            g_nes_save_state, "cpu_registers", SAVE_STATE_TYPE_UINT8,            &((rp2a03_t*)g_nes_state.cpu)->registers,            sizeof(((rp2a03_t*)g_nes_state.cpu)->registers)        );        if (result != SAVE_STATE_ERROR_NONE) {            LOG_ERROR(EMU_LOG_CAT_PLATFORM, "Falha ao registrar registradores da CPU");            return result;        }        result = save_state_add_field(            g_nes_save_state, "cpu_pc", SAVE_STATE_TYPE_UINT16,            &((rp2a03_t*)g_nes_state.cpu)->pc,            sizeof(((rp2a03_t*)g_nes_state.cpu)->pc)        );        if (result != SAVE_STATE_ERROR_NONE) {            LOG_ERROR(EMU_LOG_CAT_PLATFORM, "Falha ao registrar PC da CPU");            return result;        }        */    }    // Registra PPU se disponível    if (g_nes_state.ppu)    {        // Registra campos específicos da PPU        // Exemplo similar ao da CPU    }    // Registra APU se disponível    if (g_nes_state.apu)    {        // Registra campos específicos da APU    }    // Registra memória se disponível    if (g_nes_state.memory)    {        // Registra memória RAM        // Exemplo:        /*        result = save_state_add_field(            g_nes_save_state, "ram", SAVE_STATE_TYPE_UINT8,            nes_memory_get_ram(g_nes_state.memory),            nes_memory_get_ram_size(g_nes_state.memory)        );        if (result != SAVE_STATE_ERROR_NONE) {            LOG_ERROR(EMU_LOG_CAT_PLATFORM, "Falha ao registrar RAM");            return result;        }        */    }    // Registra cartucho se disponível    if (g_nes_state.cartridge)    {        // Registra PRG-RAM (apenas se tiver bateria)        // Exemplo:        /*        if (g_nes_state.rom_info.has_battery) {            result = save_state_add_field(                g_nes_save_state, "prg_ram", SAVE_STATE_TYPE_UINT8,                nes_cartridge_get_prg_ram(g_nes_state.cartridge),                nes_cartridge_get_prg_ram_size(g_nes_state.cartridge)            );            if (result != SAVE_STATE_ERROR_NONE) {                LOG_ERROR(EMU_LOG_CAT_PLATFORM, "Falha ao registrar PRG-RAM");                return result;            }        }        */        // Registra estado do mapper        // Depende do tipo de mapper    }    LOG_INFO(EMU_LOG_CAT_PLATFORM, "Componentes do NES registrados para save state");    return NES_ERROR_NONE;}/** * @brief Finaliza o sistema de save states para o NES */static void nes_save_state_shutdown(void){    if (g_nes_save_state)    {        save_state_destroy(g_nes_save_state);        g_nes_save_state = NULL;        LOG_INFO(EMU_LOG_CAT_PLATFORM, "Sistema de save state finalizado");    }}/** * @brief Inicializa o sistema NES com configurações padrão * * @return nes_config_t Configuração com valores padrão */static nes_config_t nes_get_default_config(void){    nes_config_t config;    memset(&config, 0, sizeof(config));    config.ntsc_mode = 1;                  // NTSC por padrão    config.audio_enabled = 1;              // Áudio habilitado    config.log_level = EMU_LOG_LEVEL_INFO; // Nível de log padrão: INFO    config.rom_path = NULL;    return config;}/** * @brief Inicializa o sistema NES com configurações padrão se não forem fornecidas * * @param config Configuração inicial (pode ser NULL para usar padrões) * @return int Código de erro (0 para sucesso) */int nes_init(const nes_config_t *config){    // Verifica se já foi inicializado    if (g_nes_state.initialized)    {        NES_LOG_WARN("Sistema NES já inicializado");        return NES_ERROR_ALREADY_INITIALIZED;    }    NES_LOG_INFO("Inicializando sistema NES");    // Limpa o estado global    memset(&g_nes_state, 0, sizeof(nes_state_t));    // Usa configurações padrão se não forem fornecidas    if (!config)    {        g_nes_state.config = nes_get_default_config();    }    else    {        g_nes_state.config = *config;    }    // Cria a instância C++ do NES    g_nes_cpp_instance = nes_cpp_create();    if (!g_nes_cpp_instance)    {        NES_LOG_ERROR("Falha ao criar instância C++ do NES");        return NES_ERROR_INITIALIZATION;    }    // Inicializa a instância C++    if (!nes_cpp_initialize(g_nes_cpp_instance))    {        NES_LOG_ERROR("Falha ao inicializar instância C++ do NES");        nes_cpp_destroy(g_nes_cpp_instance);        g_nes_cpp_instance = NULL;        return NES_ERROR_INITIALIZATION;    }    g_nes_state.initialized = 1;    NES_LOG_INFO("Sistema NES inicializado com sucesso");    return NES_ERROR_NONE;}/** * @brief Finaliza o sistema NES e libera recursos */void nes_shutdown(void){    if (!g_nes_state.initialized)    {        NES_LOG_WARN("Sistema NES não inicializado");        return;    }    NES_LOG_INFO("Finalizando sistema NES");    // Finaliza a instância C++    if (g_nes_cpp_instance)    {        nes_cpp_destroy(g_nes_cpp_instance);        g_nes_cpp_instance = NULL;    }    // Finaliza o sistema de save states    nes_save_state_shutdown();    // Reset do estado global    memset(&g_nes_state, 0, sizeof(nes_state_t));    NES_LOG_INFO("Sistema NES finalizado com sucesso");}/** * @brief Reseta o sistema NES (equivalente a pressionar o botão RESET) * * @return int Código de erro (0 para sucesso) */int nes_reset(void){    if (!g_nes_state.initialized)    {        NES_LOG_ERROR("Tentativa de resetar sistema NES não inicializado");        return NES_ERROR_NOT_INITIALIZED;    }    NES_LOG_INFO("Resetando sistema NES");    // Reseta os componentes    if (g_nes_state.cpu)        nes_cpu_reset(g_nes_state.cpu);    if (g_nes_state.ppu)        nes_ppu_reset(g_nes_state.ppu);    if (g_nes_state.apu)        nes_apu_reset(g_nes_state.apu);    if (g_nes_state.memory)        nes_memory_reset(g_nes_state.memory);    if (g_nes_state.input)        nes_input_reset(g_nes_state.input);    if (g_nes_state.cartridge)        nes_cartridge_reset(g_nes_state.cartridge);    // Configura o modo de espelhamento da PPU baseado no cartucho    if (g_nes_state.ppu && g_nes_state.cartridge)    {        nes_mirror_mode_t mode = nes_cartridge_get_mirror_mode(g_nes_state.cartridge);        nes_ppu_set_mirror_mode(g_nes_state.ppu, mode);        NES_LOG_INFO("Modo de espelhamento da PPU configurado: %d", mode);    }    NES_LOG_INFO("Sistema NES resetado com sucesso");    return NES_ERROR_NONE;}/** * @brief Carrega uma ROM no sistema NES * * @param rom_path Caminho para o arquivo ROM * @return int Código de erro (0 para sucesso) */int nes_load_rom(const char *rom_path){    if (!g_nes_state.initialized)    {        NES_LOG_ERROR("Sistema NES não inicializado");        return NES_ERROR_NOT_INITIALIZED;    }    if (!rom_path)    {        NES_LOG_ERROR("Caminho de ROM inválido");        return NES_ERROR_INVALID_PARAMETER;    }    NES_LOG_INFO("Carregando ROM: %s", rom_path);    // Carrega o arquivo ROM    FILE *rom_file = fopen(rom_path, "rb");    if (!rom_file)    {        NES_LOG_ERROR("Não foi possível abrir o arquivo ROM");        return NES_ERROR_FILE_NOT_FOUND;    }    // Obtém o tamanho do arquivo    fseek(rom_file, 0, SEEK_END);    size_t rom_size = ftell(rom_file);    fseek(rom_file, 0, SEEK_SET);    if (rom_size == 0)    {        NES_LOG_ERROR("Arquivo ROM vazio");        fclose(rom_file);        return NES_ERROR_INVALID_ROM;    }    // Aloca memória para o conteúdo da ROM    uint8_t *rom_data = (uint8_t *)malloc(rom_size);    if (!rom_data)    {        NES_LOG_ERROR("Falha ao alocar memória para ROM");        fclose(rom_file);        return NES_ERROR_MEMORY_ALLOCATION;    }    // Lê o conteúdo da ROM    size_t bytes_read = fread(rom_data, 1, rom_size, rom_file);    fclose(rom_file);    if (bytes_read != rom_size)    {        NES_LOG_ERROR("Falha ao ler o arquivo ROM");        free(rom_data);        return NES_ERROR_ROM_LOAD;    }    // Carrega a ROM na instância C++    int result = nes_cpp_load_rom(g_nes_cpp_instance, rom_data, rom_size);    free(rom_data); // Libera a memória alocada para os dados da ROM    if (!result)    {        NES_LOG_ERROR("Falha ao carregar ROM na instância C++");        return NES_ERROR_ROM_LOAD;    }    // Atualiza o caminho da ROM na configuração    if (g_nes_state.config.rom_path)    {        free((void *)g_nes_state.config.rom_path);    }    g_nes_state.config.rom_path = strdup(rom_path);    NES_LOG_INFO("ROM carregada com sucesso");    return NES_ERROR_NONE;}/** * @brief Executa um único frame do sistema NES * * @param frame_buffer Buffer para receber dados do frame renderizado * @param audio_buffer Buffer para receber dados de áudio * @param audio_buffer_size Tamanho do buffer de áudio * @return int Código de erro (0 para sucesso) */int nes_run_frame(uint32_t *frame_buffer, int16_t *audio_buffer, int audio_buffer_size){    if (!g_nes_state.initialized)    {        NES_LOG_ERROR("Sistema NES não inicializado");        return NES_ERROR_NOT_INITIALIZED;    }    if (!g_nes_cpp_instance)    {        NES_LOG_ERROR("Instância C++ do NES não inicializada");        return NES_ERROR_NOT_INITIALIZED;    }    // Executa um frame na instância C++    if (!nes_cpp_run_frame(g_nes_cpp_instance))    {        NES_LOG_ERROR("Falha ao executar frame na instância C++");        return NES_ERROR_NOT_RUNNING;    }    // Copia o buffer de vídeo para o buffer fornecido    if (frame_buffer)    {        const uint32_t *video_buffer = nes_cpp_get_video_buffer(g_nes_cpp_instance);        if (video_buffer)        {            memcpy(frame_buffer, video_buffer, NES_SCREEN_WIDTH * NES_SCREEN_HEIGHT * sizeof(uint32_t));        }    }    // O buffer de áudio seria atualizado aqui    // Mas por enquanto isso não está implementado na classe C++    g_nes_state.frame_count++;    return NES_ERROR_NONE;}/** * @brief Define o estado dos botões do controlador 1 * * @param button_state Estado dos botões (bit 0: A, bit 1: B, etc.) */void nes_set_controller1(uint8_t button_state){    if (!g_nes_state.initialized || !g_nes_state.input)    {        NES_LOG_ERROR("Tentativa de definir estado do controlador 1 com sistema não inicializado");        return;    }    nes_input_set_buttons(g_nes_state.input, 0, button_state);}/** * @brief Define o estado dos botões do controlador 2 * * @param button_state Estado dos botões (bit 0: A, bit 1: B, etc.) */void nes_set_controller2(uint8_t button_state){    if (!g_nes_state.initialized || !g_nes_state.input)    {        NES_LOG_ERROR("Tentativa de definir estado do controlador 2 com sistema não inicializado");        return;    }    nes_input_set_buttons(g_nes_state.input, 1, button_state);}/** * @brief Salva o estado atual do sistema NES * * @param state_path Caminho para o arquivo de estado * @return int Código de erro (0 para sucesso) */int nes_save_state(const char *state_path){    if (!g_nes_state.initialized)    {        NES_LOG_ERROR("Sistema NES não inicializado");        return NES_ERROR_NOT_INITIALIZED;    }    if (!state_path)    {        NES_LOG_ERROR("Caminho de arquivo de estado inválido");        return NES_ERROR_INVALID_PARAMETER;    }    NES_LOG_INFO("Salvando estado em: %s", state_path);    // Salva o estado usando a instância C++    if (!nes_cpp_save_state(g_nes_cpp_instance, state_path))    {        NES_LOG_ERROR("Falha ao salvar estado");        return -1; // Código de erro genérico    }    NES_LOG_INFO("Estado salvo com sucesso");    return NES_ERROR_NONE;}/** * @brief Carrega um estado salvo para o sistema NES * * @param state_path Caminho para o arquivo de estado * @return int Código de erro (0 para sucesso) */int nes_load_state(const char *state_path){    if (!g_nes_state.initialized)    {        NES_LOG_ERROR("Sistema NES não inicializado");        return NES_ERROR_NOT_INITIALIZED;    }    if (!state_path)    {        NES_LOG_ERROR("Caminho de arquivo de estado inválido");        return NES_ERROR_INVALID_PARAMETER;    }    NES_LOG_INFO("Carregando estado de: %s", state_path);    // Carrega o estado usando a instância C++    if (!nes_cpp_load_state(g_nes_cpp_instance, state_path))    {        NES_LOG_ERROR("Falha ao carregar estado");        return -1; // Código de erro genérico    }    NES_LOG_INFO("Estado carregado com sucesso");    return NES_ERROR_NONE;}/** * @brief Obtém o estado atual do sistema NES * * @return nes_state_t* Ponteiro para o estado (não modificar diretamente) */const nes_state_t *nes_get_state(void){    return &g_nes_state;}void nes_step(void){    if (!g_nes_state.initialized)    {        LOG_ERROR(EMU_LOG_CAT_PLATFORM, "NES não inicializado!");        return;    }    EMU_PROFILE_FUNCTION();    // Executa um ciclo da CPU    if (g_nes_state.cpu)    {        EMU_PROFILE_SECTION("CPU Cycle");        nes_cpu_cycle(g_nes_state.cpu);    }    // A PPU executa 3 ciclos para cada ciclo da CPU    if (g_nes_state.ppu)    {        EMU_PROFILE_SECTION("PPU Cycle");        for (int i = 0; i < 3; i++)        {            nes_ppu_step(g_nes_state.ppu);            // Verifica VBlank e gera NMI quando necessário            if (g_nes_state.ppu->nmi_occurred && g_nes_state.ppu->nmi_output && !g_nes_state.ppu->nmi_previous)            {                // VBlank começou, e NMIs estão habilitados                g_nes_state.ppu->nmi_previous = true;                g_nes_state.ppu->nmi_delay = 15; // Atraso de 15 ciclos para NMI            }            if (g_nes_state.ppu->nmi_delay > 0)            {                g_nes_state.ppu->nmi_delay--;                if (g_nes_state.ppu->nmi_delay == 0)                {                    // Gera o NMI                    nes_cpu_trigger_nmi(g_nes_state.cpu);                }            }            // Início de novo frame            if (g_nes_state.ppu->scanline == 0 && g_nes_state.ppu->cycle == 0)            {                // Limpa bits de status                g_nes_state.ppu->reg_status &= ~(NES_PPUSTATUS_VBLANK | NES_PPUSTATUS_SPRITE_ZERO_HIT | NES_PPUSTATUS_SPRITE_OVERFLOW);            }        }    }    // APU ocorre a cada ciclo da CPU também    if (g_nes_state.apu)    {        EMU_PROFILE_SECTION("APU Cycle");        nes_apu_cycle(g_nes_state.apu, 1);    }    // Incrementa o contador de ciclos    g_nes_state.cycles_count++;}