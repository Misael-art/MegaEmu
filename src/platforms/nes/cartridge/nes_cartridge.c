/** * @file nes_cartridge.c * @brief Implementação do subsistema de cartuchos do NES */ #include "nes_cartridge.h" #include "utils/enhanced_log.h" #include<stdlib.h> #include<string.h> #include<stdio.h> #include "../ppu/nes_ppu.h" #include "../nes.h" #include "../../../utils/file_utils.h" #include "../../../utils/error_codes.h" #include "platforms/nes/cartridge/nes_cartridge.h" #include "utils/logger/logger.h" #include "platforms/nes/cartridge/mappers/mapper0.h" #include "platforms/nes/cartridge/mappers/mapper1.h" #include "platforms/nes/cartridge/mappers/mapper2.h" #include "platforms/nes/cartridge/mappers/mapper3.h" #include "platforms/nes/cartridge/mappers/mapper4.h" #include "platforms/nes/cartridge/mappers/mapper5.h" #include "platforms/nes/cartridge/mappers/mapper6.h" #include "platforms/nes/cartridge/mappers/mapper7.h" #include "platforms/nes/cartridge/mappers/mapper8.h" #include "platforms/nes/cartridge/mappers/mapper9.h" #include "platforms/nes/cartridge/mappers/mapper10.h" #include "platforms/nes/cartridge/mappers/mapper85.h" // Definição de PATH_MAX para Windows#ifndef PATH_MAX#define PATH_MAX 260#endif// Definição da categoria de log para a plataforma NES (necessário para as macros de log)#define EMU_LOG_CAT_PLATFORM EMU_LOG_CAT_NES// Definição da categoria de log para o cartucho#define EMU_LOG_CAT_CARTRIDGE NES_LOG_CAT_CARTRIDGE// Macros de log específicas para o cartucho#define CART_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)#define CART_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)#define CART_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)#define CART_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)#define CART_LOG_TRACE(...) EMU_LOG_TRACE(EMU_LOG_CAT_CARTRIDGE, __VA_ARGS__)// Tamanho do buffer para nomes de arquivos#define PATH_BUFFER_SIZE 512// Forward declarations para implementações de mappersstatic int nes_mapper_0_init(nes_cartridge_t *cart);static int mapper_mmc1_init(nes_cartridge_t *cartridge);static int nes_mapper_2_init(nes_cartridge_t *cartridge);static int nes_mapper_5_init(nes_cartridge_t *cartridge);static int nes_mapper_6_init(nes_cartridge_t *cartridge);static int nes_mapper_7_init(nes_cartridge_t *cartridge);static int nes_mapper_8_init(nes_cartridge_t *cartridge);static int nes_mapper_9_init(nes_cartridge_t *cartridge);static int nes_mapper_10_init(nes_cartridge_t *cartridge);static int nes_mapper_71_init(nes_cartridge_t *cartridge);static int nes_mapper_85_init(nes_cartridge_t *cartridge);// Estrutura para guardar o cabeçalho iNEStypedef struct{    char magic[4];        // 'N', 'E', 'S', '\x1A'    uint8_t prg_rom_size; // Tamanho da PRG-ROM em unidades de 16KB    uint8_t chr_rom_size; // Tamanho da CHR-ROM em unidades de 8KB    uint8_t flags6;       // Byte de flags 6    uint8_t flags7;       // Byte de flags 7    uint8_t flags8;       // Byte de flags 8 (tamanho da PRG-RAM)    uint8_t flags9;       // Byte de flags 9    uint8_t flags10;      // Byte de flags 10    uint8_t padding[5];   // Padding para completar 16 bytes} nes_ines_header_t;// Estruturas específicas para mapperstypedef struct{    nes_cartridge_t *cart;    // Mapper 0 (NROM) não precisa de estado adicional} mapper0_t;typedef struct{    nes_cartridge_t *cart;    uint8_t shift_register;    uint8_t shift_count;    uint8_t control;    uint8_t chr_bank_0;    uint8_t chr_bank_1;    uint8_t prg_bank;    uint64_t last_write_cycle; // Para ignorar writes muito próximos} mapper1_t;typedef struct{    nes_cartridge_t *cart;    uint8_t prg_bank;} mapper2_t;typedef struct{    nes_cartridge_t *cart;    uint8_t bank_select;} mapper3_t;typedef struct{    nes_cartridge_t *cart;    uint8_t bank_select;    uint8_t bank_data[8];    uint8_t irq_latch;    uint8_t irq_counter;    uint8_t irq_enabled;    uint8_t irq_reload;    int current_scanline; // Armazena o scanline atual para uso na função scanline    void *cpu;            // Para gerar IRQs} mapper4_t;typedef struct{    nes_cartridge_t *cart;    uint8_t bank_select;    uint8_t bank_data[8];    uint8_t irq_latch;    uint8_t irq_counter;    uint8_t irq_enabled;    uint8_t irq_reload;    int current_scanline; // Armazena o scanline atual para uso na função scanline    void *cpu;            // Para gerar IRQs} mapper5_t;typedef struct{    nes_cartridge_t *cart;    uint8_t bank_select;    uint8_t bank_data[8];    uint8_t irq_latch;    uint8_t irq_counter;    uint8_t irq_enabled;    uint8_t irq_reload;    int current_scanline; // Armazena o scanline atual para uso na função scanline    void *cpu;            // Para gerar IRQs} mapper6_t;typedef struct{    nes_cartridge_t *cart;    uint8_t bank_select;    uint8_t bank_data[8];    uint8_t irq_latch;    uint8_t irq_counter;    uint8_t irq_enabled;    uint8_t irq_reload;    int current_scanline; // Armazena o scanline atual para uso na função scanline    void *cpu;            // Para gerar IRQs} mapper7_t;typedef struct{    nes_cartridge_t *cart;    uint8_t bank_select;    uint8_t bank_data[8];    uint8_t irq_latch;    uint8_t irq_counter;    uint8_t irq_enabled;    uint8_t irq_reload;    int current_scanline; // Armazena o scanline atual para uso na função scanline    void *cpu;            // Para gerar IRQs} mapper8_t;typedef struct{    nes_cartridge_t *cart;    uint8_t bank_select;    uint8_t bank_data[8];    uint8_t irq_latch;    uint8_t irq_counter;    uint8_t irq_enabled;    uint8_t irq_reload;    int current_scanline; // Armazena o scanline atual para uso na função scanline    void *cpu;            // Para gerar IRQs} mapper9_t;typedef struct{    nes_cartridge_t *cart;    uint8_t bank_select;    uint8_t bank_data[8];    uint8_t irq_latch;    uint8_t irq_counter;    uint8_t irq_enabled;    uint8_t irq_reload;    int current_scanline; // Armazena o scanline atual para uso na função scanline    void *cpu;            // Para gerar IRQs} mapper10_t;typedef struct{    nes_cartridge_t *cart;    uint8_t bank_select;    uint8_t bank_data[8];    uint8_t irq_latch;    uint8_t irq_counter;    uint8_t irq_enabled;    uint8_t irq_reload;    int current_scanline; // Armazena o scanline atual para uso na função scanline    void *cpu;            // Para gerar IRQs} mapper71_t;typedef struct{    nes_cartridge_t *cart;    uint8_t bank_select;    uint8_t bank_data[8];    uint8_t irq_latch;    uint8_t irq_counter;    uint8_t irq_enabled;    uint8_t irq_reload;    int current_scanline; // Armazena o scanline atual para uso na função scanline    void *cpu;            // Para gerar IRQs} mapper85_t;// Protótipos de funções para os mappers// Mapper 0 (NROM)static uint8_t mapper_0_cpu_read(void *ctx, uint16_t addr);static void mapper_0_cpu_write(void *ctx, uint16_t addr, uint8_t val);static uint8_t mapper_0_chr_read(void *ctx, uint16_t addr);static void mapper_0_chr_write(void *ctx, uint16_t addr, uint8_t val);// Mapper 1 (MMC1)static uint8_t mapper_1_cpu_read(void *ctx, uint16_t addr);static void mapper_1_cpu_write(void *ctx, uint16_t addr, uint8_t val);static uint8_t mapper_1_chr_read(void *ctx, uint16_t addr);static void mapper_1_chr_write(void *ctx, uint16_t addr, uint8_t val);// Mapper 2 (UxROM)static uint8_t mapper_2_cpu_read(void *ctx, uint16_t addr);static void mapper_2_cpu_write(void *ctx, uint16_t addr, uint8_t val);static uint8_t mapper_2_chr_read(void *ctx, uint16_t addr);static void mapper_2_chr_write(void *ctx, uint16_t addr, uint8_t val);// Mapper 4 (MMC3)static uint8_t mapper_4_cpu_read(void *ctx, uint16_t addr);static void mapper_4_cpu_write(void *ctx, uint16_t addr, uint8_t val);static uint8_t mapper_4_chr_read(void *ctx, uint16_t addr);static void mapper_4_chr_write(void *ctx, uint16_t addr, uint8_t val);static void mapper_4_scanline(void *ctx);/** * @brief Inicializa o subsistema de cartucho do NES * * @return nes_cartridge_t* Ponteiro para o cartucho inicializado, ou NULL em caso de erro */nes_cartridge_t *nes_cartridge_init(void){    CART_LOG_INFO("Inicializando subsistema de cartucho do NES");    // Aloca a estrutura principal    nes_cartridge_t *cartridge = (nes_cartridge_t *)malloc(sizeof(nes_cartridge_t));    if (!cartridge)    {        CART_LOG_ERROR("nes_cartridge_init: falha na alocação de memória para o cartucho");        return NULL;    }    // Inicializa a estrutura com zeros    memset(cartridge, 0, sizeof(nes_cartridge_t));    // Inicializa os valores padrão    cartridge->mapper_type = NES_MAPPER_NROM;    cartridge->mapper_number = 0;    cartridge->mirror_mode = NES_MIRROR_HORIZONTAL;    cartridge->prg_rom = NULL;    cartridge->chr_rom = NULL;    cartridge->prg_ram = NULL;    cartridge->chr_ram = NULL;    cartridge->prg_rom_size = 0;    cartridge->chr_rom_size = 0;    cartridge->prg_ram_size = 0;    cartridge->chr_ram_size = 0;    cartridge->has_battery = 0;    cartridge->rom_path = NULL;    cartridge->mapper = NULL;    cartridge->mapper_data = NULL;    cartridge->sram_dirty = 0;    CART_LOG_INFO("Subsistema de cartucho do NES inicializado com sucesso");    return cartridge;}/** * @brief Finaliza e libera recursos do cartucho */void nes_cartridge_shutdown(nes_cartridge_t *cartridge){    if (!cartridge)    {        CART_LOG_WARN("nes_cartridge_shutdown: cartridge já está desligado");        return;    }    CART_LOG_INFO("Desligando subsistema de cartucho do NES");    // Libera a memória ROM/RAM    if (cartridge->prg_rom)    {        free(cartridge->prg_rom);    }    if (cartridge->chr_rom)    {        free(cartridge->chr_rom);    }    if (cartridge->prg_ram)    {        free(cartridge->prg_ram);    }    if (cartridge->chr_ram)    {        free(cartridge->chr_ram);    }    // Libera o caminho da ROM    if (cartridge->rom_path)    {        free(cartridge->rom_path);    }    // Libera o mapper, se existir    if (cartridge->mapper && cartridge->mapper->shutdown)    {        cartridge->mapper->shutdown(cartridge);    }    // Libera a estrutura principal    free(cartridge);}/** * @brief Reseta o cartucho para o estado inicial * * @param cartridge Ponteiro para o cartucho */void nes_cartridge_reset(nes_cartridge_t *cartridge){    if (!cartridge)    {        CART_LOG_ERROR("nes_cartridge_reset: cartridge inválido");        return;    }    CART_LOG_INFO("Resetando subsistema de cartucho do NES");    // Reseta o mapper, se existir    if (cartridge->mapper && cartridge->mapper->reset)    {        cartridge->mapper->reset(cartridge);    }    // A PRG-RAM com bateria não é zerada no reset    // PRG-RAM sem bateria e CHR-RAM podem ou não ser zeradas, dependendo da implementação    CART_LOG_DEBUG("Cartucho resetado: mapper=%d", cartridge->mapper_number);}/** * @brief Carrega uma ROM NES * * @param cartridge Ponteiro para o cartucho * @param rom_path Caminho do arquivo da ROM * @return int 0 em caso de sucesso, código de erro em caso de falha */int nes_cartridge_load(nes_cartridge_t *cartridge, const char *rom_path){    if (!cartridge || !rom_path)    {        CART_LOG_ERROR("nes_cartridge_load: parâmetros inválidos - cartridge=%p, rom_path=%s",                       cartridge, rom_path ? rom_path : "NULL");        return -1;    }    CART_LOG_INFO("Carregando ROM NES: %s", rom_path);    // Abre o arquivo    FILE *file = fopen(rom_path, "rb");    if (!file)    {        CART_LOG_ERROR("nes_cartridge_load: falha ao abrir arquivo: %s", rom_path);        return -1;    }    // Verifica o tamanho do arquivo    fseek(file, 0, SEEK_END);    long file_size = ftell(file);    fseek(file, 0, SEEK_SET);    CART_LOG_INFO("Tamanho do arquivo ROM: %ld bytes", file_size);    if (file_size < 16) // Cabeçalho iNES tem pelo menos 16 bytes    {        CART_LOG_ERROR("nes_cartridge_load: arquivo muito pequeno para ser uma ROM NES");        fclose(file);        return -1;    }    // Lê o cabeçalho iNES    uint8_t header[16];    if (fread(header, 1, 16, file) != 16)    {        CART_LOG_ERROR("nes_cartridge_load: falha ao ler o cabeçalho");        fclose(file);        return -1;    }    // Verifica assinatura "NES^Z"    if (header[0] != 'N' || header[1] != 'E' || header[2] != 'S' || header[3] != 0x1A)    {        CART_LOG_ERROR("nes_cartridge_load: assinatura NES inválida: %c%c%c%02X",                       header[0], header[1], header[2], header[3]);        fclose(file);        return -1;    }    // Extrai informações do cabeçalho    uint8_t prg_rom_16k_units = header[4];    uint8_t chr_rom_8k_units = header[5];    uint8_t flags6 = header[6];    uint8_t flags7 = header[7];    uint8_t prg_ram_8k_units = header[8];    uint8_t flags9 = header[9];    uint8_t flags10 = header[10];    CART_LOG_DEBUG("Informações do cabeçalho iNES:");    CART_LOG_DEBUG("  PRG-ROM units: %d (16KB each)", prg_rom_16k_units);    CART_LOG_DEBUG("  CHR-ROM units: %d (8KB each)", chr_rom_8k_units);    CART_LOG_DEBUG("  Flags6: 0x%02X", flags6);    CART_LOG_DEBUG("  Flags7: 0x%02X", flags7);    CART_LOG_DEBUG("  PRG-RAM units: %d (8KB each)", prg_ram_8k_units);    // Validações adicionais    if (prg_rom_16k_units == 0)    {        CART_LOG_ERROR("nes_cartridge_load: ROM sem PRG-ROM");        fclose(file);        return NES_ERROR_INVALID_ROM;    }    // Calcula tamanhos    int prg_rom_size = prg_rom_16k_units * 16 * 1024;    int chr_rom_size = chr_rom_8k_units * 8 * 1024;    int prg_ram_size = (prg_ram_8k_units > 0 ? prg_ram_8k_units : 1) * 8 * 1024;    int chr_ram_size = (chr_rom_size == 0) ? 8 * 1024 : 0; // CHR-RAM se não tiver CHR-ROM    CART_LOG_DEBUG("Tamanhos calculados:");    CART_LOG_DEBUG("  PRG-ROM: %d bytes", prg_rom_size);    CART_LOG_DEBUG("  CHR-ROM: %d bytes", chr_rom_size);    CART_LOG_DEBUG("  PRG-RAM: %d bytes", prg_ram_size);    CART_LOG_DEBUG("  CHR-RAM: %d bytes", chr_ram_size);    // Extrai informações de espelhamento    int mirror_mode = (flags6 & 0x01) ? NES_MIRROR_VERTICAL : NES_MIRROR_HORIZONTAL;    if (flags6 & 0x08) // Four-screen mirroring    {        mirror_mode = NES_MIRROR_FOUR_SCREEN;    }    // Extrai informações de mapper    int mapper_number = ((flags7 & 0xF0) | (flags6 >> 4)) & 0xFF;    int ines_version = (flags7 >> 2) & 0x03;    CART_LOG_DEBUG("Configurações do cartucho:");    CART_LOG_DEBUG("  Mapper: %d", mapper_number);    CART_LOG_DEBUG("  iNES version: %d", ines_version);    CART_LOG_DEBUG("  Mirror mode: %d", mirror_mode);    // Verifica se o mapper é suportado    switch (mapper_number)    {    case 0: // NROM    case 1: // MMC1    case 2: // UNROM    case 3: // CNROM    case 4: // MMC3    case 5: // MMC5    case 6: // FFE F4xxx    case 7: // AxROM    case 8: // FFE F3xxx    case 9: // MMC2/PxROM    case 10: // MMC4/FxROM    case 71: // Camerica    case 85: // VRC7        // Mappers suportados        CART_LOG_INFO("Mapper %d suportado", mapper_number);        break;    default:        CART_LOG_ERROR("nes_cartridge_load: mapper %d não suportado", mapper_number);        fclose(file);        return NES_ERROR_UNSUPPORTED_MAPPER;    }    // Informações adicionais    int has_battery = (flags6 & 0x02) ? 1 : 0;    int has_trainer = (flags6 & 0x04) ? 1 : 0;    CART_LOG_DEBUG("Características adicionais:");    CART_LOG_DEBUG("  Battery: %s", has_battery ? "Sim" : "Não");    CART_LOG_DEBUG("  Trainer: %s", has_trainer ? "Sim" : "Não");    // Pula o trainer, se presente    if (has_trainer)    {        CART_LOG_DEBUG("Pulando trainer (512 bytes)");        fseek(file, 512, SEEK_CUR);    }    // Libera recursos anteriores    if (cartridge->prg_rom)    {        free(cartridge->prg_rom);    }    if (cartridge->chr_rom)    {        free(cartridge->chr_rom);    }    if (cartridge->prg_ram)    {        free(cartridge->prg_ram);    }    if (cartridge->chr_ram)    {        free(cartridge->chr_ram);    }    if (cartridge->rom_path)    {        free(cartridge->rom_path);    }    // Aloca memória para a PRG-ROM    cartridge->prg_rom = (uint8_t *)malloc(prg_rom_size);    if (!cartridge->prg_rom)    {        CART_LOG_ERROR("nes_cartridge_load: falha ao alocar PRG-ROM");        fclose(file);        return -1;    }    // Lê a PRG-ROM    if (fread(cartridge->prg_rom, 1, prg_rom_size, file) != prg_rom_size)    {        CART_LOG_ERROR("nes_cartridge_load: falha ao ler PRG-ROM");        fclose(file);        return -1;    }    CART_LOG_DEBUG("PRG-ROM carregada: primeiros bytes: %02X %02X %02X %02X",                   cartridge->prg_rom[0], cartridge->prg_rom[1],                   cartridge->prg_rom[2], cartridge->prg_rom[3]);    // Aloca e lê a CHR-ROM, se presente    if (chr_rom_size > 0)    {        cartridge->chr_rom = (uint8_t *)malloc(chr_rom_size);        if (!cartridge->chr_rom)        {            CART_LOG_ERROR("nes_cartridge_load: falha ao alocar CHR-ROM");            fclose(file);            return -1;        }        if (fread(cartridge->chr_rom, 1, chr_rom_size, file) != chr_rom_size)        {            CART_LOG_ERROR("nes_cartridge_load: falha ao ler CHR-ROM");            fclose(file);            return -1;        }        CART_LOG_DEBUG("CHR-ROM carregada: primeiros bytes: %02X %02X %02X %02X",                       cartridge->chr_rom[0], cartridge->chr_rom[1],                       cartridge->chr_rom[2], cartridge->chr_rom[3]);        // Verifica alguns tiles para debug        CART_LOG_DEBUG("Verificando primeiros tiles da CHR-ROM:");        for (int i = 0; i < 2; i++)        {            CART_LOG_DEBUG("Tile %d:", i);            for (int row = 0; row < 8; row++)            {                uint8_t pattern_low = cartridge->chr_rom[i * 16 + row];                uint8_t pattern_high = cartridge->chr_rom[i * 16 + row + 8];                CART_LOG_DEBUG("  Row %d: low=%02X high=%02X", row, pattern_low, pattern_high);            }        }    }    else    {        CART_LOG_INFO("ROM não tem CHR-ROM, usando CHR-RAM");    }    // Aloca a PRG-RAM    cartridge->prg_ram = (uint8_t *)malloc(prg_ram_size);    if (!cartridge->prg_ram)    {        CART_LOG_ERROR("nes_cartridge_load: falha ao alocar PRG-RAM");        fclose(file);        return -1;    }    memset(cartridge->prg_ram, 0, prg_ram_size);    CART_LOG_DEBUG("PRG-RAM alocada e zerada: %d bytes", prg_ram_size);    // Aloca a CHR-RAM, se necessário    if (chr_ram_size > 0)    {        cartridge->chr_ram = (uint8_t *)malloc(chr_ram_size);        if (!cartridge->chr_ram)        {            CART_LOG_ERROR("nes_cartridge_load: falha ao alocar CHR-RAM");            fclose(file);            return -1;        }        memset(cartridge->chr_ram, 0, chr_ram_size);        CART_LOG_DEBUG("CHR-RAM alocada e zerada: %d bytes", chr_ram_size);    }    // Atualiza as informações do cartucho    cartridge->prg_rom_size = prg_rom_size;    cartridge->chr_rom_size = chr_rom_size;    cartridge->prg_ram_size = prg_ram_size;    cartridge->chr_ram_size = chr_ram_size;    cartridge->mapper_number = mapper_number;    cartridge->mirror_mode = mirror_mode;    cartridge->has_battery = has_battery;    cartridge->rom_path = strdup(rom_path);    // Fecha o arquivo    fclose(file);    // Carrega a PRG-RAM com bateria, se existir    if (has_battery)    {        CART_LOG_INFO("Carregando SRAM da bateria");        nes_cartridge_load_sram(cartridge, NULL);    }    // Cria e inicializa o mapper apropriado    int mapper_result = nes_cartridge_create_mapper(cartridge);    if (mapper_result != 0)    {        CART_LOG_ERROR("nes_cartridge_load: falha ao criar mapper");        return mapper_result;    }    CART_LOG_INFO("ROM NES carregada com sucesso: %s", rom_path);    return 0;}/** * @brief Obtém informações sobre a ROM carregada * * @param cartridge Ponteiro para o cartucho * @param info Ponteiro para a estrutura de informações a ser preenchida * @return int 0 em caso de sucesso, código de erro em caso de falha */int nes_cartridge_get_info(nes_cartridge_t *cartridge, nes_rom_info_t *info){    if (!cartridge || !info)    {        CART_LOG_ERROR("nes_cartridge_get_info: parâmetros inválidos");        return -1;    }    // Preenche a estrutura de informações    info->mapper_type = cartridge->mapper_type;    info->mapper_number = cartridge->mapper_number;    info->prg_rom_size = cartridge->prg_rom_size;    info->chr_rom_size = cartridge->chr_rom_size;    info->prg_ram_size = cartridge->prg_ram_size;    info->has_battery = cartridge->has_battery;    info->mirroring = cartridge->mirror_mode;    info->prg_rom = cartridge->prg_rom;    info->chr_rom = cartridge->chr_rom;    return 0;}/** * @brief Lê um byte da ROM ou RAM do cartucho (acesso da CPU) * * @param cartridge Ponteiro para o cartucho * @param address Endereço para leitura * @return uint8_t Valor lido */uint8_t nes_cartridge_cpu_read(nes_cartridge_t *cartridge, uint16_t address){    if (!cartridge)    {        CART_LOG_ERROR("nes_cartridge_cpu_read: cartridge inválido");        return 0;    }    // Verifica se endereço está no range do cartucho    if (address < 0x4020)    {        CART_LOG_WARN("nes_cartridge_cpu_read: endereço fora do range do cartucho: $%04X", address);        return 0;    }    // Se o mapper estiver configurado, delega a leitura para ele    if (cartridge->mapper && cartridge->mapper->cpu_read)    {        return cartridge->mapper->cpu_read(cartridge, address);    }    // Implementação padrão para o mapper NROM (0)    if (address >= 0x8000)    {        // Acesso à PRG-ROM: mapeia para a ROM        uint32_t prg_addr = (address - 0x8000) % cartridge->prg_rom_size;        return cartridge->prg_rom[prg_addr];    }    else if (address >= 0x6000 && address < 0x8000)    {        // Acesso à PRG-RAM (SRAM): mapeia para a RAM        uint32_t ram_addr = (address - 0x6000) % cartridge->prg_ram_size;        return cartridge->prg_ram[ram_addr];    }    // Caso especial para o endereço $5F19 (usado por alguns jogos como Super Mario Bros)    else if (address == 0x5F19)    {        // Retorna um valor fixo ou um comportamento específico para este endereço        // Muitos jogos usam este endereço para detecção de hardware ou características específicas        return 0x00; // Valor padrão que satisfaz a maioria dos jogos    }    CART_LOG_WARN("nes_cartridge_cpu_read: endereço não mapeado: $%04X", address);    return 0;}/** * @brief Escreve um byte na ROM ou RAM do cartucho (acesso da CPU) * * @param cartridge Ponteiro para o cartucho * @param address Endereço para escrita * @param value Valor a ser escrito */void nes_cartridge_cpu_write(nes_cartridge_t *cartridge, uint16_t address, uint8_t value){    if (!cartridge)    {        CART_LOG_ERROR("nes_cartridge_cpu_write: cartridge inválido");        return;    }    // Verifica se endereço está no range do cartucho    if (address < 0x4020)    {        CART_LOG_WARN("nes_cartridge_cpu_write: endereço fora do range do cartucho: $%04X", address);        return;    }    // Se o mapper estiver configurado, delega a escrita para ele    if (cartridge->mapper && cartridge->mapper->cpu_write)    {        cartridge->mapper->cpu_write(cartridge, address, value);        return;    }    // Implementação padrão para o mapper NROM (0)    if (address >= 0x8000)    {        // Acesso à PRG-ROM: não permite escrita (a menos que seja um mapper especial)        // Alguns jogos tentam escrever em PRG-ROM como teste ou para acionar comportamentos especiais        // Caso especial para o endereço $8224 (usado por alguns jogos, incluindo Super Mario Bros)        if (address == 0x8224)        {            // Podemos implementar um comportamento específico aqui se necessário            // Por enquanto, apenas logamos, mas não tratamos como erro crítico            CART_LOG_WARN("nes_cartridge_cpu_write: tentativa de escrita em PRG-ROM: $%04X = $%02X", address, value);            // Opcionalmente, podemos definir uma variável de "flag interna" para rastrear esta escrita            // cartridge->special_flag = value;            return; // Retorna sem erro para não interromper o jogo        }        // Para outros endereços, mantenha o comportamento anterior        CART_LOG_WARN("nes_cartridge_cpu_write: tentativa de escrita em PRG-ROM: $%04X = $%02X", address, value);    }    else if (address >= 0x6000 && address < 0x8000)    {        // Acesso à PRG-RAM (SRAM): permite escrita        uint32_t ram_addr = (address - 0x6000) % cartridge->prg_ram_size;        cartridge->prg_ram[ram_addr] = value;        // Marca PRG-RAM como modificada para salvar depois, se tiver bateria        if (cartridge->has_battery)        {            cartridge->sram_dirty = 1;        }    }    else    {        CART_LOG_WARN("nes_cartridge_cpu_write: endereço não mapeado: $%04X = $%02X", address, value);    }}/** * @brief Lê um byte da CHR-ROM/RAM do cartucho (acesso da PPU) * * @param cartridge Ponteiro para o cartucho * @param address Endereço para leitura * @return uint8_t Valor lido */uint8_t nes_cartridge_chr_read(nes_cartridge_t *cartridge, uint16_t address){    if (!cartridge)    {        CART_LOG_ERROR("nes_cartridge_chr_read: cartridge é NULL");        return 0;    }    CART_LOG_DEBUG("nes_cartridge_chr_read: Iniciando leitura em 0x%04X", address);    if (address >= 0x2000)    {        CART_LOG_WARN("nes_cartridge_chr_read: endereço 0x%04X fora do alcance", address);        return 0;    }    // Verifica se o mapper tem função de leitura CHR    if (cartridge->mapper && cartridge->mapper->chr_read)    {        CART_LOG_DEBUG("nes_cartridge_chr_read: usando mapper %d para ler CHR em 0x%04X",                       cartridge->mapper_number, address);        uint8_t value = cartridge->mapper->chr_read(cartridge->mapper->context, address);        CART_LOG_DEBUG("nes_cartridge_chr_read: mapper retornou 0x%02X para tile %d linha %d",                       value, (address & 0xFF0) >> 4, address & 0x7);        return value;    }    // Fallback para leitura direta    if (cartridge->chr_rom && cartridge->chr_rom_size > 0)    {        // Calcula o endereço real na CHR-ROM        uint16_t bank = address >> 12;     // Seleciona o banco de 4KB (0 ou 1)        uint16_t offset = address & 0xFFF; // Offset dentro do banco        uint16_t chr_addr = (bank * 0x1000 + offset) % cartridge->chr_rom_size;        uint8_t value = cartridge->chr_rom[chr_addr];        CART_LOG_DEBUG("nes_cartridge_chr_read: lendo CHR-ROM[0x%04X] = 0x%02X para tile %d linha %d",                       chr_addr, value, (address & 0xFF0) >> 4, address & 0x7);        return value;    }    else if (cartridge->chr_ram && cartridge->chr_ram_size > 0)    {        // Calcula o endereço real na CHR-RAM        uint16_t bank = address >> 12;     // Seleciona o banco de 4KB (0 ou 1)        uint16_t offset = address & 0xFFF; // Offset dentro do banco        uint16_t chr_addr = (bank * 0x1000 + offset) % cartridge->chr_ram_size;        uint8_t value = cartridge->chr_ram[chr_addr];        CART_LOG_DEBUG("nes_cartridge_chr_read: lendo CHR-RAM[0x%04X] = 0x%02X para tile %d linha %d",                       chr_addr, value, (address & 0xFF0) >> 4, address & 0x7);        return value;    }    CART_LOG_ERROR("nes_cartridge_chr_read: CHR-ROM e CHR-RAM ausentes");    return 0;}/** * @brief Escreve um byte na memória de padrões (VRAM) do cartucho (acesso da PPU) * * @param cartridge Ponteiro para o cartucho * @param address Endereço para escrita * @param value Valor a ser escrito */void nes_cartridge_chr_write(nes_cartridge_t *cartridge, uint16_t address, uint8_t value){    if (!cartridge)    {        CART_LOG_ERROR("nes_cartridge_chr_write: cartridge inválido");        return;    }    // Verifica se o endereço está no range válido    if (address >= 0x2000)    {        CART_LOG_WARN("nes_cartridge_chr_write: endereço fora do range CHR: $%04X", address);        return;    }    // Se o mapper estiver configurado, delega a escrita para ele    if (cartridge->mapper && cartridge->mapper->chr_write)    {        cartridge->mapper->chr_write(cartridge, address, value);        return;    }    // Implementação padrão para o mapper NROM (0)    if (cartridge->chr_rom)    {        // CHR-ROM: não permite escrita        CART_LOG_WARN("nes_cartridge_chr_write: tentativa de escrita em CHR-ROM: $%04X = $%02X", address, value);    }    else if (cartridge->chr_ram)    {        // CHR-RAM: permite escrita        cartridge->chr_ram[address % cartridge->chr_ram_size] = value;    }    else    {        CART_LOG_WARN("nes_cartridge_chr_write: nenhuma ROM/RAM CHR disponível");    }}/** * @brief Cria o mapper apropriado para o cartucho * * @param cartridge Ponteiro para o cartucho * @return int 0 em caso de sucesso, código de erro em caso de falha */int nes_cartridge_create_mapper(nes_cartridge_t *cartridge)
{
    if (!cartridge)
    {
        NES_LOG_ERROR("nes_cartridge_create_mapper: cartridge inválido");
        return -1;
    }

    int mapper_number = cartridge->mapper_number;

    // Verifica se o mapper está registrado
    if (!nes_cartridge_is_mapper_supported(mapper_number))
    {
        NES_LOG_ERROR("nes_cartridge_create_mapper: mapper %d não suportado", mapper_number);
        return NES_ERROR_UNSUPPORTED_MAPPER;
    }

    NES_LOG_INFO("Criando mapper %d (%s)", mapper_number, nes_cartridge_get_mapper_name(mapper_number));

    // Obtém a função de inicialização para o mapper
    nes_mapper_init_func init_func = nes_mapper_init_funcs[mapper_number];
    if (!init_func)
    {
        NES_LOG_ERROR("nes_cartridge_create_mapper: função de inicialização não encontrada para mapper %d", mapper_number);
        return NES_ERROR_UNSUPPORTED_MAPPER;
    }

    // Inicializa o mapper
    cartridge->mapper = init_func(cartridge);
    if (!cartridge->mapper)
    {
        NES_LOG_ERROR("nes_cartridge_create_mapper: falha ao inicializar mapper %d", mapper_number);
        return NES_ERROR_MAPPER_INIT_FAILED;
    }

    NES_LOG_INFO("Mapper %d (%s) inicializado com sucesso",
                 mapper_number, nes_cartridge_get_mapper_name(mapper_number));

    return NES_ERROR_NONE;
}

/**
 * @brief Finaliza o sistema de mappers
 *
 * Esta função libera os recursos alocados pelo sistema de mappers.
 * Deve ser chamada antes do encerramento do subsistema de cartuchos.
 */
void nes_cartridge_mappers_shutdown(void)
{
    CART_LOG_INFO("Finalizando sistema de registro de mappers");

    // Limpa as arrays de funções de inicialização e nomes
    for (int i = 0; i < 256; i++)
    {
        nes_mapper_init_funcs[i] = NULL;
        nes_mapper_names[i] = NULL;
    }

    CART_LOG_INFO("Sistema de registro de mappers finalizado com sucesso");
}

/**
 * @brief Variáveis globais para o sistema de registro de mappers
 */
static nes_mapper_t *(*nes_mapper_init_funcs[256])(nes_cartridge_t *) = {NULL};
static const char *nes_mapper_names[256] = {NULL};

/**
 * @brief Inicializa o sistema de mappers
 *
 * Esta função inicializa o sistema de registro de mappers.
 * Deve ser chamada antes de qualquer operação com mappers.
 */
int32_t nes_cartridge_mappers_init(void)
{
    CART_LOG_INFO("Inicializando sistema de mappers");

    // Registra todos os mappers suportados
    nes_cartridge_register_mappers();

    CART_LOG_INFO("Sistema de mappers inicializado com sucesso");
    return 0;
}

/**
 * @brief Registra todos os mappers suportados
 */
void nes_cartridge_register_mappers(void)
{
    CART_LOG_INFO("Inicializando sistema de registro de mappers");

    // Limpa as arrays
    memset(nes_mapper_init_funcs, 0, sizeof(nes_mapper_init_funcs));
    memset(nes_mapper_names, 0, sizeof(nes_mapper_names));

    // Registra os mappers suportados
    nes_mapper_init_funcs[0] = nes_mapper_0_init;
    nes_mapper_names[0] = "NROM";

    nes_mapper_init_funcs[1] = nes_mapper_1_init;
    nes_mapper_names[1] = "MMC1";

    nes_mapper_init_funcs[2] = nes_mapper_2_init;
    nes_mapper_names[2] = "UxROM";

    nes_mapper_init_funcs[3] = nes_mapper_3_init;
    nes_mapper_names[3] = "CNROM";

    nes_mapper_init_funcs[4] = nes_mapper_4_init;
    nes_mapper_names[4] = "MMC3";

    nes_mapper_init_funcs[5] = nes_mapper_5_init;
    nes_mapper_names[5] = "MMC5";

    nes_mapper_init_funcs[6] = nes_mapper_6_init;
    nes_mapper_names[6] = "FFE F4xxx";

    nes_mapper_init_funcs[7] = nes_mapper_7_init;
    nes_mapper_names[7] = "AxROM";

    nes_mapper_init_funcs[8] = nes_mapper_8_init;
    nes_mapper_names[8] = "FFE F3xxx";

    nes_mapper_init_funcs[9] = nes_mapper_9_init;
    nes_mapper_names[9] = "MMC2/PxROM";

    nes_mapper_init_funcs[10] = nes_mapper_10_init;
    nes_mapper_names[10] = "MMC4/FxROM";

    nes_mapper_init_funcs[71] = nes_mapper_71_init;
    nes_mapper_names[71] = "Camerica";

    nes_mapper_init_funcs[85] = nes_mapper_85_init;
    nes_mapper_names[85] = "VRC7";

    CART_LOG_INFO("Sistema de registro de mappers inicializado com sucesso");
}

/**
 * @brief Verifica se um mapper específico é suportado
 */
bool nes_cartridge_is_mapper_supported(int32_t mapper_number)
{
    if (mapper_number < 0 || mapper_number >= 256)
    {
        return false;
    }

    return nes_mapper_init_funcs[mapper_number] != NULL;
}

/**
 * @brief Obtém o nome de um mapper específico
 */
const char *nes_cartridge_get_mapper_name(int32_t mapper_number)
{
    if (mapper_number < 0 || mapper_number >= 256 || nes_mapper_names[mapper_number] == NULL)
    {
        return "Unknown";
    }

    return nes_mapper_names[mapper_number];
}

// Protótipos das funções de inicialização dos mappers
nes_mapper_t *nes_mapper_0_init(nes_cartridge_t *cartridge);
nes_mapper_t *nes_mapper_1_init(nes_cartridge_t *cartridge);
nes_mapper_t *nes_mapper_2_init(nes_cartridge_t *cartridge);
nes_mapper_t *nes_mapper_3_init(nes_cartridge_t *cartridge);
nes_mapper_t *nes_mapper_4_init(nes_cartridge_t *cartridge);
nes_mapper_t *nes_mapper_5_init(nes_cartridge_t *cartridge);
nes_mapper_t *nes_mapper_6_init(nes_cartridge_t *cartridge);
nes_mapper_t *nes_mapper_7_init(nes_cartridge_t *cartridge);
nes_mapper_t *nes_mapper_8_init(nes_cartridge_t *cartridge);
nes_mapper_t *nes_mapper_9_init(nes_cartridge_t *cartridge);
nes_mapper_t *nes_mapper_10_init(nes_cartridge_t *cartridge);
nes_mapper_t *nes_mapper_71_init(nes_cartridge_t *cartridge);
nes_mapper_t *nes_mapper_85_init(nes_cartridge_t *cartridge);

// Forward declarations para implementações de mappers
static int nes_mapper_0_init(nes_cartridge_t *cart);
static int mapper_mmc1_init(nes_cartridge_t *cartridge);
