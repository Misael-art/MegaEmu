/** * @file megadrive.c * @brief Implementação da plataforma Mega Drive/Genesis */ #include<stdio.h> #include<stdlib.h> #include<string.h> #include<stdint.h> #include<stdbool.h> #include<stddef.h> #include "megadrive.h" #include "../../core/core.h" #include "../../core/interfaces/memory_interface.h" #include "../../core/interfaces/video_interface.h" #include "../../core/interfaces/platform_interface.h" #include "cpu/m68k.h" #include "../../video/vdp.h" #include "cpu/z80_adapter.h" // Redefinição só se não estiver já definido#ifndef _CRT_SECURE_NO_WARNINGS#define _CRT_SECURE_NO_WARNINGS#endif// Definições de portas do VDP#define VDP_PORT_DATA 0x00    // Porta de dados#define VDP_PORT_CONTROL 0x04 // Porta de controle// Estrutura opaca do Mega Drivestruct megadrive_t{    md_platform_data_t platform_data;    bool initialized;};// Variável global para armazenar os dados da plataformastatic md_platform_data_t *g_md_data = NULL;// Forward declarations das funções de acesso à memóriastatic uint8_t md_m68k_read_8(uint32_t address);static uint16_t md_m68k_read_16(uint32_t address);static uint32_t md_m68k_read_32(uint32_t address);static void md_m68k_write_8(uint32_t address, uint8_t value);static void md_m68k_write_16(uint32_t address, uint16_t value);static void md_m68k_write_32(uint32_t address, uint32_t value);// Funções externas do VDPextern bool emu_video_write_data(emu_video_t video, uint16_t address, uint8_t value);extern bool emu_video_write_control(emu_video_t video, uint8_t value);extern bool emu_video_read_data(emu_video_t video, uint16_t address, uint8_t *value);extern bool emu_video_read_control(emu_video_t video, uint8_t *value);extern bool emu_video_update(emu_video_t video, int32_t cycles);// Funções externas da CPU M68Kextern int32_t m68k_execute(emu_cpu_t cpu, int32_t cycles);// Forward declarations das funções específicas da plataformastatic bool md_init(emu_platform_t *platform, void *ctx);static bool md_shutdown(emu_platform_t *platform);static bool md_reset(emu_platform_t *platform);static bool md_load_rom(emu_platform_t *platform, const char *filename);static bool md_run_frame(emu_platform_t *platform);static bool md_run_cycles(emu_platform_t *platform, int32_t cycles);/** * @brief Cria uma instância da plataforma Mega Drive/Genesis * @return Nova instância da plataforma ou NULL em caso de erro */emu_platform_t *emu_platform_megadrive_create(void){    // Alocar memória para a plataforma    emu_platform_t *platform = (emu_platform_t *)malloc(sizeof(emu_platform_t));    if (!platform)        return NULL;    // Alocar memória para os dados específicos da plataforma    md_platform_data_t *data = (md_platform_data_t *)malloc(sizeof(md_platform_data_t));    if (!data)    {        free(platform);        return NULL;    }    // Inicializar os dados da plataforma    memset(data, 0, sizeof(md_platform_data_t));    // Configurar a plataforma    platform->platform_data = data;    platform->init = md_init;    platform->shutdown = md_shutdown;    platform->reset = md_reset;    platform->load_rom = md_load_rom;    platform->run_frame = md_run_frame;    platform->run_cycles = md_run_cycles;    return platform;}/** * @brief Inicialização da plataforma Mega Drive */bool md_init(emu_platform_t *platform, void *ctx){    if (!platform || !ctx)        return false;    // Cast dos dados da plataforma    md_platform_data_t *data = (md_platform_data_t *)ctx;    g_md_data = data;    // Configuração da plataforma    strncpy(data->name, "Mega Drive/Genesis", sizeof(data->name) - 1);    strncpy(data->id, "SEGA_MD", sizeof(data->id) - 1);    data->cpu_clock = 7670000;   // 7.67 MHz    data->vdp_clock = 13423294;  // 13.42 MHz    data->sound_clock = 3579545; // 3.58 MHz    data->screen_width = 320;    data->screen_height = 224;    data->has_secondary_cpu = 1;    data->has_color = 1;    data->max_sprites = 80;    data->max_colors = 64;    // Inicializar componentes    data->is_initialized = 1;    data->frame_cycles = 0;    // Inicializar o Z80
data->z80_cpu = md_z80_adapter_create();
if (!data->z80_cpu)
{
    LOG_ERROR("Falha ao criar processador Z80");
    return false;
} // Conectar o Z80 ao sistema de memória e áudio    md_z80_adapter_connect_memory(data->z80_cpu, g_memory);    md_z80_adapter_connect_audio(data->z80_cpu, g_audio);    return true;}/** * @brief Funções de acesso à memória para M68K */static uint8_t md_m68k_read_8(uint32_t address){    if (!g_md_data)        return 0xFF;    // Mapeamento de memória do Mega Drive    if (address < 0x400000)    {        // ROM do cartucho (0x000000-0x3FFFFF)        if (g_md_data->cart_rom && address < g_md_data->cart_rom_size)        {            return g_md_data->cart_rom[address];        }    }    else if (address >= 0xE00000 && address < 0xE10000)    {        // RAM (0xE00000-0xE0FFFF)        if (g_md_data->ram)        {            return g_md_data->ram[address & 0xFFFF];        }    }    else if (address >= 0xC00000 && address < 0xC00020)    {        // Portas do VDP (0xC00000-0xC0001F)        if (g_md_data->vdp)        {            uint8_t value = 0;            if ((address & 0x1F) == VDP_PORT_DATA)            {                if (emu_video_read_data(g_md_data->vdp, (uint16_t)(address & 0x1F), &value))                {                    return value;                }            }            else if ((address & 0x1F) == VDP_PORT_CONTROL)            {                if (emu_video_read_control(g_md_data->vdp, &value))                {                    return value;                }            }        }    }    return 0xFF;}/** * @brief Lê uma palavra de 16 bits da memória do M68K */static uint16_t md_m68k_read_16(uint32_t address){    uint16_t value = (uint16_t)md_m68k_read_8(address) << 8;    value |= md_m68k_read_8(address + 1);    return value;}/** * @brief Lê uma palavra de 32 bits da memória do M68K */static uint32_t md_m68k_read_32(uint32_t address){    uint32_t value = (uint32_t)md_m68k_read_16(address) << 16;    value |= md_m68k_read_16(address + 2);    return value;}/** * @brief Escreve um byte na memória do M68K */static void md_m68k_write_8(uint32_t address, uint8_t value){    if (!g_md_data)        return;    if (address >= 0xE00000 && address < 0xE10000)    {        // RAM (0xE00000-0xE0FFFF)        if (g_md_data->ram)        {            g_md_data->ram[address & 0xFFFF] = value;        }    }    else if (address >= 0xC00000 && address < 0xC00020)    {        // Portas do VDP (0xC00000-0xC0001F)        if (g_md_data->vdp)        {            if ((address & 0x1F) == VDP_PORT_DATA)            {                emu_video_write_data(g_md_data->vdp, (uint16_t)(address & 0x1F), value);            }            else if ((address & 0x1F) == VDP_PORT_CONTROL)            {                emu_video_write_control(g_md_data->vdp, value);            }        }    }}/** * @brief Escreve uma palavra de 16 bits na memória do M68K */static void md_m68k_write_16(uint32_t address, uint16_t value){    md_m68k_write_8(address, (uint8_t)(value >> 8));    md_m68k_write_8(address + 1, (uint8_t)value);}/** * @brief Escreve uma palavra de 32 bits na memória do M68K */static void md_m68k_write_32(uint32_t address, uint32_t value){    md_m68k_write_16(address, (uint16_t)(value >> 16));    md_m68k_write_16(address + 2, (uint16_t)value);}/** * @brief Desliga a plataforma Mega Drive */bool md_shutdown(emu_platform_t *platform){    if (!platform || !platform->platform_data)        return false;    md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;
// Salvar SRAM antes de desligar
if (data->cart_rom)
{
    // Criar nome de arquivo para SRAM baseado no nome da ROM
    char sram_filename[256] = {0};

    // Se temos nome do arquivo da ROM, usamos ele
    if (data->rom_header.overseas_name[0])
    {
        strncpy(sram_filename, data->rom_header.overseas_name, sizeof(sram_filename) - 5);
        sram_filename[sizeof(sram_filename) - 5] = '\0';

        // Remover espaços e caracteres inválidos
        for (char *p = sram_filename; *p; p++)
        {
            if (*p == ' ' || *p == '/' || *p == '\\' || *p == ':' || *p == '*' || *p == '?' || *p == '"' || *p == '<' || *p == '>' || *p == '|')
            {
                *p = '_';
            }
        }

        // Adicionar extensão .srm
        strcat(sram_filename, ".srm");

        // Salvar SRAM
        md_memory_save_sram(sram_filename);
        printf("SRAM salva: %s\n", sram_filename);
    }
}

// Liberar memória alocada
if (data->cart_rom)
{
    free(data->cart_rom);
    data->cart_rom = NULL;
}

if (data->ram)
{
    free(data->ram);
    data->ram = NULL;
}

data->is_initialized = 0;
g_md_data = NULL;

// Liberar o Z80
if (data->z80_cpu)
{
    md_z80_adapter_destroy(data->z80_cpu);
    data->z80_cpu = NULL;
}

return true;
} /** * @brief Reseta a plataforma Mega Drive */
bool md_reset(emu_platform_t *platform)
{
    if (!platform || !platform->platform_data)
        return false;
    md_platform_data_t *data = (md_platform_data_t *)platform->platform_data; // Resetar CPUs    if (data->m68k_cpu)    {        emu_cpu_reset(data->m68k_cpu);    }    if (data->z80_cpu)    {        md_z80_adapter_reset(data->z80_cpu);        md_z80_adapter_set_reset(data->z80_cpu, true);  // Z80 começa em reset        md_z80_adapter_set_busreq(data->z80_cpu, true); // Barramento começa solicitado    }    // Resetar VDP    if (data->vdp)    {        emu_video_reset(data->vdp);    }    // Resetar memória    if (data->memory)    {        emu_memory_reset(data->memory);    }    // Resetar contadores    data->frame_cycles = 0;    // Resetar buffers do VDP    data->vdp_data_buffer = 0;    data->vdp_control_buffer = 0;    return true;}/** * @brief Carrega uma ROM do Mega Drive */bool md_load_rom(emu_platform_t *platform, const char *filename){    if (!platform || !platform->platform_data || !filename)        return false;    md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;    // Abrir o arquivo da ROM
    FILE *rom_file = fopen(filename, "rb");
    if (!rom_file)
        return false; // Obter o tamanho do arquivo
    fseek(rom_file, 0, SEEK_END);
    size_t rom_size = ftell(rom_file);
    fseek(rom_file, 0, SEEK_SET); // Alocar memória para a ROM
    uint8_t *rom_data = (uint8_t *)malloc(rom_size);
    if (!rom_data)
    {
        fclose(rom_file);
        return false;
    } // Ler a ROM
    if (fread(rom_data, 1, rom_size, rom_file) != rom_size)
    {
        free(rom_data);
        fclose(rom_file);
        return false;
    }
    fclose(rom_file); // Liberar ROM anterior se existir
    if (data->cart_rom)
    {
        free(data->cart_rom);
    } // Atualizar dados da ROM
    data->cart_rom = rom_data;
    data->cart_rom_size = rom_size; // Ler cabeçalho da ROM
    if (rom_size >= 0x200)
    {
        memcpy(data->rom_header.console_name, &rom_data[0x100], 16);
        memcpy(data->rom_header.copyright, &rom_data[0x110], 16);
        memcpy(data->rom_header.domestic_name, &rom_data[0x120], 48);
        memcpy(data->rom_header.overseas_name, &rom_data[0x150], 48);
        memcpy(data->rom_header.serial_number, &rom_data[0x180], 14);
        data->rom_header.checksum = (rom_data[0x18E] << 8) | rom_data[0x18F];
        memcpy(data->rom_header.io_support, &rom_data[0x190], 16);
        data->rom_header.rom_start = (rom_data[0x1A0] << 24) | (rom_data[0x1A1] << 16) | (rom_data[0x1A2] << 8) | rom_data[0x1A3];
        data->rom_header.rom_end = (rom_data[0x1A4] << 24) | (rom_data[0x1A5] << 16) | (rom_data[0x1A6] << 8) | rom_data[0x1A7];
        data->rom_header.ram_start = (rom_data[0x1A8] << 24) | (rom_data[0x1A9] << 16) | (rom_data[0x1AA] << 8) | rom_data[0x1AB];
        data->rom_header.ram_end = (rom_data[0x1AC] << 24) | (rom_data[0x1AD] << 16) | (rom_data[0x1AE] << 8) | rom_data[0x1AF];
        memcpy(data->rom_header.sram_info, &rom_data[0x1B0], 12);
        memcpy(data->rom_header.modem_info, &rom_data[0x1BC], 12);
        memcpy(data->rom_header.notes, &rom_data[0x1C8], 40);
        memcpy(data->rom_header.region, &rom_data[0x1F0], 16);
        // Log de informações da ROM
        printf("ROM carregada: %s\n", data->rom_header.overseas_name);
        printf("  Tamanho: %zu bytes\n", rom_size);
        printf("  Região: %s\n", data->rom_header.region);
        printf("  Serial: %s\n", data->rom_header.serial_number);
    }
    // Carregar a ROM no sistema de memória e detectar o mapper apropriado
    if (!md_memory_load_rom(rom_data, rom_size))
    {
        printf("Erro ao carregar ROM no sistema de memória\n");
        free(rom_data);
        return false;
    }
    // Extrair nome do arquivo sem extensão para o arquivo SRAM
    char sram_filename[256];
    strncpy(sram_filename, filename, sizeof(sram_filename) - 5);
    sram_filename[sizeof(sram_filename) - 5] = '\0'; // Remover extensão
    char *dot = strrchr(sram_filename, '.');
    if (dot)
    {
        *dot = '\0';
    }
    // Adicionar extensão .srm
    strcat(sram_filename, ".srm");
    // Tentar carregar SRAM existente
    if (md_memory_load_sram(sram_filename))
    {
        printf("SRAM carregada: %s\n", sram_filename);
    }
    // Resetar a plataforma
    return md_reset(platform);
} /** * @brief Executa um frame da plataforma Mega Drive */
bool md_run_frame(emu_platform_t *platform)
{
    if (!platform || !platform->platform_data)
        return false;
    md_platform_data_t *data = (md_platform_data_t *)platform->platform_data; // Ciclos por frame (PAL: 59.94 Hz, NTSC: 50 Hz)    const int32_t cycles_per_frame = 128000; // Aproximadamente 7.67 MHz / 60 Hz    // Loop principal de execução    while (data->frame_cycles < cycles_per_frame) {        // Executar ciclos da CPU principal (M68K)        if (data->m68k_cpu) {            uint32_t cycles = emu_cpu_step(data->m68k_cpu);            data->frame_cycles += cycles;
                                                                              // Executar o Z80 (se não estiver em reset ou busreq)            if (data->z80_cpu) {                // Converter ciclos M68K para ciclos Z80 (M68K é ~3.5x mais rápido)                uint32_t z80_cycles = cycles / 4;                md_z80_adapter_run(data->z80_cpu, z80_cycles);            }        }        // Atualizar VDP        if (data->vdp)        {            emu_video_update(data->vdp, cycles_per_frame);        }    }    return true;}/** * @brief Executa um número específico de ciclos na plataforma Mega Drive */bool md_run_cycles(emu_platform_t *platform, int32_t cycles){    if (!platform || !platform->platform_data || cycles <= 0)        return false;    md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;    // Executar ciclos da CPU principal    if (data->m68k_cpu)    {        emu_cpu_step(data->m68k_cpu);    }    // Executar ciclos da CPU secundária (Z80 roda a 3.58 MHz, então divide por ~2)    if (data->z80_cpu)    {        emu_cpu_step(data->z80_cpu);    }    // Atualizar VDP    if (data->vdp)    {        emu_video_update(data->vdp, cycles);    }    // Atualizar contadores    data->frame_cycles += cycles;    return true;}void md_set_z80_reset(bool reset) {    if (g_md_data->z80_cpu) {        md_z80_adapter_set_reset(g_md_data->z80_cpu, reset);    }}void md_set_z80_busreq(bool request) {    if (g_md_data->z80_cpu) {        md_z80_adapter_set_busreq(g_md_data->z80_cpu, request);    }}bool md_get_z80_busreq(void) {    if (g_md_data->z80_cpu) {        return md_z80_adapter_get_busreq(g_md_data->z80_cpu);    }    return true; // Valor padrão seguro}void md_set_z80_bank(uint16_t bank) {    if (g_md_data->z80_cpu) {        md_z80_adapter_set_bank(g_md_data->z80_cpu, bank);    }}/**
    *@brief Salva o estado da plataforma Mega Drive
            * /
        bool md_save_state_save(emu_platform_t * platform, const char *filename,
                                const uint8_t *screenshot_data, uint32_t width,
                                uint32_t height, uint32_t stride)
    {
        if (!platform || !platform->platform_data || !filename)
            return false;

        md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;

        // Criar save state
        save_state_t *state = md_save_state_create(
            platform,
            screenshot_data,
            width,
            height,
            stride,
            true,  // com thumbnail
            NULL,  // sem descrição específica
            NULL); // sem tags específicas

        if (!state)
        {
            printf("Falha ao criar save state\n");
            return false;
        }

        // Salvar em arquivo
        int32_t result = md_save_state_save(state, filename);
        save_state_destroy(state);

        if (result != SAVE_STATE_ERROR_NONE)
        {
            printf("Falha ao salvar save state: %d\n", result);
            return false;
        }

        // Atualizar contador
        data->save_state_counter++;
        data->has_save_state = true;

        printf("Save state salvo com sucesso: %s\n", filename);
        return true;
    }

    /**
     * @brief Carrega um save state da plataforma Mega Drive
     */
    bool md_save_state_load(emu_platform_t * platform, const char *filename)
    {
        if (!platform || !platform->platform_data || !filename)
            return false;

        md_platform_data_t *data = (md_platform_data_t *)platform->platform_data;

        // Carregar save state
        save_state_t *state = md_save_state_load(filename, platform);
        if (!state)
        {
            printf("Falha ao carregar save state: %s\n", filename);
            return false;
        }

        // Aplicar save state
        int32_t result = md_save_state_apply(state, platform);
        save_state_destroy(state);

        if (result != SAVE_STATE_ERROR_NONE)
        {
            printf("Falha ao aplicar save state: %d\n", result);
            return false;
        }

        // Atualizar estado
        data->has_save_state = true;

        printf("Save state carregado com sucesso: %s\n", filename);
        return true;
    }

    /**
     * @brief Configura o sistema de rewind da plataforma Mega Drive
     */
    bool md_save_state_config_rewind(emu_platform_t * platform, uint32_t capacity,
                                     uint32_t frames_per_snapshot)
    {
        if (!platform)
            return false;

        int32_t result = md_save_state_config_rewind(capacity, frames_per_snapshot);
        if (result != SAVE_STATE_ERROR_NONE)
        {
            printf("Falha ao configurar sistema de rewind: %d\n", result);
            return false;
        }

        printf("Sistema de rewind configurado: %u estados, %u frames/snapshot\n",
               capacity, frames_per_snapshot);
        return true;
    }

    /**
     * @brief Captura um snapshot para o sistema de rewind
     */
    bool md_save_state_capture_rewind(emu_platform_t * platform)
    {
        if (!platform)
            return false;

        int32_t result = md_save_state_capture_rewind(platform);
        if (result != SAVE_STATE_ERROR_NONE)
        {
            // Não logar erros aqui para evitar spam no console
            return false;
        }

        return true;
    }

    /**
     * @brief Aplica rewind de um estado
     */
    bool md_save_state_rewind(emu_platform_t * platform)
    {
        if (!platform)
            return false;

        int32_t result = md_save_state_rewind(platform);
        if (result != SAVE_STATE_ERROR_NONE)
        {
            printf("Falha ao aplicar rewind: %d\n", result);
            return false;
        }

        return true;
    }

    /**
     * @brief Cria um save state para o Mega Drive
     * @param md Ponteiro para a estrutura do Mega Drive
     * @param state Ponteiro para a estrutura de save state
     * @param slot Slot para o save state (0-9)
     * @param create_thumbnail Se verdadeiro, cria uma thumbnail
     * @return Código de erro do save state
     */
    int32_t megadrive_create_save_state(megadrive_t * md, save_state_t * state, int32_t slot, bool create_thumbnail)
    {
        if (!md || !state)
        {
            LOG_ERROR("Parâmetros inválidos para save state");
            return SAVE_STATE_ERROR_INVALID;
        }

        // Inicializar estado e metadados básicos
        if (save_state_init(state, "MEGADRIVE", MD_VERSION, slot) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao inicializar save state");
            return SAVE_STATE_ERROR_INIT;
        }

        // Registrar metadados do estado
        char rom_name[128] = {0};
        md_rom_get_name(md->rom, rom_name, sizeof(rom_name));
        save_state_set_metadata(state, "game_name", rom_name);

        char rom_region[16] = {0};
        md_rom_get_region_name(md->rom, rom_region, sizeof(rom_region));
        save_state_set_metadata(state, "game_region", rom_region);

        char checksum[16] = {0};
        sprintf(checksum, "%04X", md->rom->checksum);
        save_state_set_metadata(state, "rom_checksum", checksum);

        // Registrar timestamp e informações do jogo
        time_t now = time(NULL);
        save_state_set_timestamp(state, now);

        // Registrar contadores
        save_state_set_counter(state, "play_time_seconds", md->statistics.play_time_seconds);
        save_state_set_counter(state, "save_count", md->statistics.save_count + 1);
        save_state_set_counter(state, "load_count", md->statistics.load_count);

        // Criar thumbnail se solicitado
        if (create_thumbnail)
        {
            uint8_t *screen_buffer = md->vdp.framebuffer;
            int32_t width = VDP_SCREEN_WIDTH;
            int32_t height = VDP_SCREEN_HEIGHT;

            // Criar thumbnail WebP com tarja "Save"
            save_state_create_thumbnail_webp(state, screen_buffer, width, height, "Save", slot);
        }

        // Registrar dados do estado atual da emulação

        // 1. Mapper (ROM, SRAM, EEPROM)
        if (md_mapper_register_save_state(state) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao registrar mapper no save state");
            return SAVE_STATE_ERROR_REGISTER;
        }

        // 2. CPU M68K
        if (m68k_register_save_state(state, &md->m68k) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao registrar M68K no save state");
            return SAVE_STATE_ERROR_REGISTER;
        }

        // 3. CPU Z80
        if (z80_register_save_state(state, &md->z80) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao registrar Z80 no save state");
            return SAVE_STATE_ERROR_REGISTER;
        }

        // 4. VDP
        if (vdp_register_save_state(state, &md->vdp) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao registrar VDP no save state");
            return SAVE_STATE_ERROR_REGISTER;
        }

        // 5. PSG e FM
        if (psg_register_save_state(state, &md->psg) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao registrar PSG no save state");
            return SAVE_STATE_ERROR_REGISTER;
        }

        if (ym2612_register_save_state(state, &md->fm) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao registrar YM2612 no save state");
            return SAVE_STATE_ERROR_REGISTER;
        }

        // 6. Controler IO
        if (io_register_save_state(state, &md->io) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao registrar IO no save state");
            return SAVE_STATE_ERROR_REGISTER;
        }

        // Incrementar contador de saves
        md->statistics.save_count++;

        LOG_INFO("Save state criado com sucesso (slot %d)", slot);
        return SAVE_STATE_ERROR_NONE;
    }

    /**
     * @brief Carrega um save state para o Mega Drive
     * @param md Ponteiro para a estrutura do Mega Drive
     * @param state Ponteiro para a estrutura de save state
     * @param slot Slot para o save state (0-9)
     * @return Código de erro do save state
     */
    int32_t megadrive_load_save_state(megadrive_t * md, save_state_t * state, int32_t slot)
    {
        if (!md || !state)
        {
            LOG_ERROR("Parâmetros inválidos para load save state");
            return SAVE_STATE_ERROR_INVALID;
        }

        // Carregar estado
        if (save_state_load(state, "MEGADRIVE", slot) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao carregar save state");
            return SAVE_STATE_ERROR_LOAD;
        }

        // Verificar compatibilidade da ROM
        char checksum[16] = {0};
        if (save_state_get_metadata(state, "rom_checksum", checksum, sizeof(checksum)) == SAVE_STATE_ERROR_NONE)
        {
            uint16_t state_checksum = 0;
            sscanf(checksum, "%hx", &state_checksum);

            if (state_checksum != md->rom->checksum)
            {
                LOG_WARNING("Checksum da ROM não corresponde ao save state (%04X != %04X)",
                            md->rom->checksum, state_checksum);

                // Continuar mesmo com checksum diferente, mas alertar
            }
        }

        // Restaurar dados do estado

        // 1. Mapper (ROM, SRAM, EEPROM)
        if (md_mapper_restore_save_state(state) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao restaurar mapper do save state");
            return SAVE_STATE_ERROR_RESTORE;
        }

        // 2. CPU M68K
        if (m68k_restore_save_state(state, &md->m68k) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao restaurar M68K do save state");
            return SAVE_STATE_ERROR_RESTORE;
        }

        // 3. CPU Z80
        if (z80_restore_save_state(state, &md->z80) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao restaurar Z80 do save state");
            return SAVE_STATE_ERROR_RESTORE;
        }

        // 4. VDP
        if (vdp_restore_save_state(state, &md->vdp) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao restaurar VDP do save state");
            return SAVE_STATE_ERROR_RESTORE;
        }

        // 5. PSG e FM
        if (psg_restore_save_state(state, &md->psg) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao restaurar PSG do save state");
            return SAVE_STATE_ERROR_RESTORE;
        }

        if (ym2612_restore_save_state(state, &md->fm) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao restaurar YM2612 do save state");
            return SAVE_STATE_ERROR_RESTORE;
        }

        // 6. Controler IO
        if (io_restore_save_state(state, &md->io) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao restaurar IO do save state");
            return SAVE_STATE_ERROR_RESTORE;
        }

        // Incrementar contador de loads
        md->statistics.load_count++;

        LOG_INFO("Save state carregado com sucesso (slot %d)", slot);
        return SAVE_STATE_ERROR_NONE;
    }

    /**
     * @brief Cria um snapshot para o sistema de rewind
     * @param md Ponteiro para a estrutura do Mega Drive
     * @param rewind_state Ponteiro para a estrutura de rewind
     * @return Código de erro do save state
     */
    int32_t megadrive_create_rewind_snapshot(megadrive_t * md, rewind_state_t * rewind_state)
    {
        if (!md || !rewind_state)
        {
            LOG_ERROR("Parâmetros inválidos para rewind snapshot");
            return SAVE_STATE_ERROR_INVALID;
        }

        save_state_t temp_state;

        // Inicializar save state temporário (sem thumbnail para economizar memória)
        if (megadrive_create_save_state(md, &temp_state, 0, false) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao criar save state temporário para rewind");
            return SAVE_STATE_ERROR_INIT;
        }

        // Adicionar ao buffer circular de rewind
        int32_t result = rewind_add_snapshot(rewind_state, &temp_state);

        // Liberar recursos temporários
        save_state_cleanup(&temp_state);

        return result;
    }

    /**
     * @brief Aplica um snapshot de rewind
     * @param md Ponteiro para a estrutura do Mega Drive
     * @param rewind_state Ponteiro para a estrutura de rewind
     * @param steps Número de passos para retroceder (negativo) ou avançar (positivo)
     * @return Código de erro do save state
     */
    int32_t megadrive_apply_rewind_snapshot(megadrive_t * md, rewind_state_t * rewind_state, int32_t steps)
    {
        if (!md || !rewind_state)
        {
            LOG_ERROR("Parâmetros inválidos para aplicar rewind");
            return SAVE_STATE_ERROR_INVALID;
        }

        save_state_t temp_state;

        // Obter snapshot do buffer circular
        if (rewind_get_snapshot(rewind_state, &temp_state, steps) != SAVE_STATE_ERROR_NONE)
        {
            LOG_ERROR("Falha ao obter snapshot de rewind");
            return SAVE_STATE_ERROR_LOAD;
        }

        // Aplicar estado sem incrementar contadores

        // 1. Mapper (ROM, SRAM, EEPROM)
        md_mapper_restore_save_state(&temp_state);

        // 2. CPU M68K
        m68k_restore_save_state(&temp_state, &md->m68k);

        // 3. CPU Z80
        z80_restore_save_state(&temp_state, &md->z80);

        // 4. VDP
        vdp_restore_save_state(&temp_state, &md->vdp);

        // 5. PSG e FM
        psg_restore_save_state(&temp_state, &md->psg);
        ym2612_restore_save_state(&temp_state, &md->fm);

        // 6. Controler IO
        io_restore_save_state(&temp_state, &md->io);

        // Aplicar efeito visual de rewind (escala de cinza)
        if (steps < 0)
        {
            vdp_apply_grayscale_effect(&md->vdp);
        }

        // Liberar recursos temporários
        save_state_cleanup(&temp_state);

        return SAVE_STATE_ERROR_NONE;
    }
