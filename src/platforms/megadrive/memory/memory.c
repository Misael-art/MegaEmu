/** * @file memory.c * @brief Implementação do sistema de memória do Mega
 * Drive */                                                                    \
#include "memory.h" #include "md_mapper.h" #include                            \
         "core/memory/memory_interface.h" #include                             \
         "utils/log_utils.h" #include                                          \
         "utils/validation_utils.h" #include<stdlib.h> #include<               \
             string.h> /* Definições de memória do Mega Drive */               \
    #define MD_ROM_START 0x000000 #define MD_ROM_SIZE 0x400000 // 4MB
                                                               // máximo#define
                                                               // MD_RAM_START
                                                               // 0xFF0000#define
                                                               // MD_RAM_SIZE
                                                               // 0x010000 //
                                                               // 64KB#define
                                                               // MD_VDP_START
                                                               // 0xC00000#define
                                                               // MD_VDP_SIZE
                                                               // 0x020000 //
                                                               // 128KB#define
                                                               // MD_IO_START
                                                               // 0xA10000#define
                                                               // MD_IO_SIZE
                                                               // 0x010000 //
                                                               // 64KB#define
                                                               // MD_Z80_START
                                                               // 0xA00000#define
                                                               // MD_Z80_SIZE
                                                               // 0x010000 //
                                                               // 64KB/*
                                                               // Instância
                                                               // global do
                                                               // sistema de
                                                               // memória
                                                               // */static
                                                               // emu_memory_t
                                                               // g_memory =
                                                               // NULL;/*
                                                               // Buffers de
                                                               // memória
                                                               // */static
                                                               // uint8_t
                                                               // *g_rom_data =
                                                               // NULL;static
                                                               // uint8_t
                                                               // *g_ram_data =
                                                               // NULL;static
                                                               // uint8_t
                                                               // *g_vdp_data =
                                                               // NULL;static
                                                               // uint8_t
                                                               // *g_io_data =
                                                               // NULL;static
                                                               // uint8_t
                                                               // *g_z80_data =
                                                               // NULL;/* Mapper
                                                               // global do Mega
                                                               // Drive */static
                                                               // md_mapper_t
                                                               // g_md_mapper;/*
                                                               // Callbacks para
                                                               // acesso à
                                                               // memória ROM
                                                               // usando o
                                                               // mapper
                                                               // */static
                                                               // uint8_t
                                                               // rom_read_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address);static
                                                               // uint16_t
                                                               // rom_read_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address);static
                                                               // uint32_t
                                                               // rom_read_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address);static
                                                               // void
                                                               // rom_write_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint8_t
                                                               // value);static
                                                               // void
                                                               // rom_write_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint16_t
                                                               // value);static
                                                               // void
                                                               // rom_write_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint32_t
                                                               // value);/*
                                                               // Callbacks para
                                                               // acesso à
                                                               // memória do VDP
                                                               // */static
                                                               // uint8_t
                                                               // vdp_read_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address);static
                                                               // uint16_t
                                                               // vdp_read_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address);static
                                                               // uint32_t
                                                               // vdp_read_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address);static
                                                               // void
                                                               // vdp_write_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint8_t
                                                               // value);static
                                                               // void
                                                               // vdp_write_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint16_t
                                                               // value);static
                                                               // void
                                                               // vdp_write_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint32_t
                                                               // value);/*
                                                               // Callbacks para
                                                               // acesso à
                                                               // memória de I/O
                                                               // */static
                                                               // uint8_t
                                                               // io_read_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address);static
                                                               // uint16_t
                                                               // io_read_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address);static
                                                               // uint32_t
                                                               // io_read_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address);static
                                                               // void
                                                               // io_write_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint8_t
                                                               // value);static
                                                               // void
                                                               // io_write_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint16_t
                                                               // value);static
                                                               // void
                                                               // io_write_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint32_t
                                                               // value);/*
                                                               // Callbacks para
                                                               // acesso à
                                                               // memória do Z80
                                                               // */static
                                                               // uint8_t
                                                               // z80_read_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address);static
                                                               // uint16_t
                                                               // z80_read_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address);static
                                                               // uint32_t
                                                               // z80_read_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address);static
                                                               // void
                                                               // z80_write_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint8_t
                                                               // value);static
                                                               // void
                                                               // z80_write_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint16_t
                                                               // value);static
                                                               // void
                                                               // z80_write_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint32_t
                                                               // value);/** *
                                                               // @brief
                                                               // Inicializa o
                                                               // sistema de
                                                               // memória do
                                                               // Mega Drive
                                                               // */int
                                                               // md_memory_init(void){
                                                               // LOG_INFO("Inicializando
                                                               // sistema de
                                                               // memória do
                                                               // Mega Drive");
                                                               // // Criar
                                                               // instância do
                                                               // sistema de
                                                               // memória
                                                               // g_memory =
                                                               // emu_memory_create();
                                                               // if (!g_memory)
                                                               // {
                                                               // LOG_ERROR("Falha
                                                               // ao criar
                                                               // instância do
                                                               // sistema de
                                                               // memória");
                                                               // return
                                                               // EMU_ERROR_MEMORY;
                                                               // }    //
                                                               // Inicializar o
                                                               // sistema de
                                                               // memória    if
                                                               // (!emu_memory_init(g_memory))
                                                               // {
                                                               // LOG_ERROR("Falha
                                                               // ao inicializar
                                                               // sistema de
                                                               // memória");
                                                               // emu_memory_destroy(g_memory);
                                                               // return
                                                               // EMU_ERROR_MEMORY;
                                                               // }    // Alocar
                                                               // buffers de
                                                               // memória
                                                               // g_rom_data =
                                                               // (uint8_t
                                                               // *)calloc(1,
                                                               // MD_ROM_SIZE);
                                                               // g_ram_data =
                                                               // (uint8_t
                                                               // *)calloc(1,
                                                               // MD_RAM_SIZE);
                                                               // g_vdp_data =
                                                               // (uint8_t
                                                               // *)calloc(1,
                                                               // MD_VDP_SIZE);
                                                               // g_io_data =
                                                               // (uint8_t
                                                               // *)calloc(1,
                                                               // MD_IO_SIZE);
                                                               // g_z80_data =
                                                               // (uint8_t
                                                               // *)calloc(1,
                                                               // MD_Z80_SIZE);
                                                               // if
                                                               // (!g_rom_data
                                                               // || !g_ram_data
                                                               // || !g_vdp_data
                                                               // || !g_io_data
                                                               // ||
                                                               // !g_z80_data)
                                                               // {
                                                               // LOG_ERROR("Falha
                                                               // ao alocar
                                                               // buffers de
                                                               // memória");
                                                               // md_memory_shutdown();
                                                               // return
                                                               // EMU_ERROR_MEMORY;
                                                               // }    //
                                                               // Configurar
                                                               // callbacks para
                                                               // ROM (usando o
                                                               // mapper)
                                                               // memory_callbacks_t
                                                               // rom_callbacks
                                                               // = { .read_8 =
                                                               // rom_read_8,
                                                               // .read_16 =
                                                               // rom_read_16,
                                                               // .read_32 =
                                                               // rom_read_32,
                                                               // .write_8 =
                                                               // rom_write_8,
                                                               // .write_16 =
                                                               // rom_write_16,
                                                               // .write_32 =
                                                               // rom_write_32};
                                                               // // Configurar
                                                               // callbacks para
                                                               // VDP
                                                               // memory_callbacks_t
                                                               // vdp_callbacks
                                                               // = { .read_8 =
                                                               // vdp_read_8,
                                                               // .read_16 =
                                                               // vdp_read_16,
                                                               // .read_32 =
                                                               // vdp_read_32,
                                                               // .write_8 =
                                                               // vdp_write_8,
                                                               // .write_16 =
                                                               // vdp_write_16,
                                                               // .write_32 =
                                                               // vdp_write_32};
                                                               // // Configurar
                                                               // callbacks para
                                                               // I/O
                                                               // memory_callbacks_t
                                                               // io_callbacks =
                                                               // { .read_8 =
                                                               // io_read_8,
                                                               // .read_16 =
                                                               // io_read_16,
                                                               // .read_32 =
                                                               // io_read_32,
                                                               // .write_8 =
                                                               // io_write_8,
                                                               // .write_16 =
                                                               // io_write_16,
                                                               // .write_32 =
                                                               // io_write_32};
                                                               // // Configurar
                                                               // callbacks para
                                                               // Z80
                                                               // memory_callbacks_t
                                                               // z80_callbacks
                                                               // = { .read_8 =
                                                               // z80_read_8,
                                                               // .read_16 =
                                                               // z80_read_16,
                                                               // .read_32 =
                                                               // z80_read_32,
                                                               // .write_8 =
                                                               // z80_write_8,
                                                               // .write_16 =
                                                               // z80_write_16,
                                                               // .write_32 =
                                                               // z80_write_32};
                                                               // // Adicionar
                                                               // regiões de
                                                               // memória    if
                                                               // (!emu_memory_add_region(g_memory,
                                                               // MD_ROM_START,
                                                               // MD_ROM_SIZE,
                                                               // g_rom_data,
                                                               // EMU_MEMORY_ROM,
                                                               // &rom_callbacks)
                                                               // ||
                                                               // !emu_memory_add_region(g_memory,
                                                               // MD_RAM_START,
                                                               // MD_RAM_SIZE,
                                                               // g_ram_data,
                                                               // EMU_MEMORY_RAM,
                                                               // NULL) ||
                                                               // !emu_memory_add_region(g_memory,
                                                               // MD_VDP_START,
                                                               // MD_VDP_SIZE,
                                                               // g_vdp_data,
                                                               // EMU_MEMORY_RAM,
                                                               // &vdp_callbacks)
                                                               // ||
                                                               // !emu_memory_add_region(g_memory,
                                                               // MD_IO_START,
                                                               // MD_IO_SIZE,
                                                               // g_io_data,
                                                               // EMU_MEMORY_RAM,
                                                               // &io_callbacks)
                                                               // ||
                                                               // !emu_memory_add_region(g_memory,
                                                               // MD_Z80_START,
                                                               // MD_Z80_SIZE,
                                                               // g_z80_data,
                                                               // EMU_MEMORY_RAM,
                                                               // &z80_callbacks))
                                                               // {
                                                               // LOG_ERROR("Falha
                                                               // ao adicionar
                                                               // regiões de
                                                               // memória");
                                                               // md_memory_shutdown();
                                                               // return
                                                               // EMU_ERROR_MEMORY;
                                                               // }    //
                                                               // Inicializar o
                                                               // mapper padrão
                                                               // (tipo será
                                                               // detectado
                                                               // quando a ROM
                                                               // for carregada)
                                                               // if
                                                               // (!md_mapper_init(&g_md_mapper,
                                                               // MD_MAPPER_NONE,
                                                               // g_rom_data,
                                                               // MD_ROM_SIZE))
                                                               // {
                                                               // LOG_ERROR("Falha
                                                               // ao inicializar
                                                               // mapper");
                                                               // md_memory_shutdown();
                                                               // return
                                                               // EMU_ERROR_MEMORY;
                                                               // }
                                                               // LOG_INFO("Sistema
                                                               // de memória do
                                                               // Mega Drive
                                                               // inicializado
                                                               // com sucesso");
                                                               // return
                                                               // EMU_ERROR_NONE;}/**
                                                               // * @brief
                                                               // Desliga o
                                                               // sistema de
                                                               // memória do
                                                               // Mega Drive
                                                               // */void
                                                               // md_memory_shutdown(void){
                                                               // // Liberar
                                                               // recursos do
                                                               // mapper
                                                               // md_mapper_shutdown(&g_md_mapper);
                                                               // if (g_memory)
                                                               // {
                                                               // emu_memory_destroy(g_memory);
                                                               // g_memory =
                                                               // NULL;    }
                                                               // free(g_rom_data);
                                                               // free(g_ram_data);
                                                               // free(g_vdp_data);
                                                               // free(g_io_data);
                                                               // free(g_z80_data);
                                                               // g_rom_data =
                                                               // NULL;
                                                               // g_ram_data =
                                                               // NULL;
                                                               // g_vdp_data =
                                                               // NULL;
                                                               // g_io_data =
                                                               // NULL;
                                                               // g_z80_data =
                                                               // NULL;}/** *
                                                               // @brief Lê um
                                                               // byte da
                                                               // memória
                                                               // */uint8_t
                                                               // md_memory_read_8(uint32_t
                                                               // address){
                                                               // return
                                                               // emu_memory_read_8(g_memory,
                                                               // address);}/**
                                                               // * @brief Lê
                                                               // uma word da
                                                               // memória
                                                               // */uint16_t
                                                               // md_memory_read_16(uint32_t
                                                               // address){
                                                               // return
                                                               // emu_memory_read_16(g_memory,
                                                               // address);}/**
                                                               // * @brief Lê
                                                               // uma long word
                                                               // da memória
                                                               // */uint32_t
                                                               // md_memory_read_32(uint32_t
                                                               // address){
                                                               // return
                                                               // emu_memory_read_32(g_memory,
                                                               // address);}/**
                                                               // * @brief
                                                               // Escreve um
                                                               // byte na
                                                               // memória */void
                                                               // md_memory_write_8(uint32_t
                                                               // address,
                                                               // uint8_t
                                                               // value){
                                                               // emu_memory_write_8(g_memory,
                                                               // address,
                                                               // value);}/** *
                                                               // @brief Escreve
                                                               // uma word na
                                                               // memória */void
                                                               // md_memory_write_16(uint32_t
                                                               // address,
                                                               // uint16_t
                                                               // value){
                                                               // emu_memory_write_16(g_memory,
                                                               // address,
                                                               // value);}/** *
                                                               // @brief Escreve
                                                               // uma long word
                                                               // na memória
                                                               // */void
                                                               // md_memory_write_32(uint32_t
                                                               // address,
                                                               // uint32_t
                                                               // value){
                                                               // emu_memory_write_32(g_memory,
                                                               // address,
                                                               // value);}/*
                                                               // Implementação
                                                               // dos callbacks
                                                               // de ROM usando
                                                               // o mapper
                                                               // */static
                                                               // uint8_t
                                                               // rom_read_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address) {
                                                               // return
                                                               // md_mapper_read_rom(&g_md_mapper,
                                                               // address);
                                                               // }static
                                                               // uint16_t
                                                               // rom_read_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address) { //
                                                               // Lê dois bytes
                                                               // consecutivos
                                                               // uint16_t value
                                                               // =
                                                               // rom_read_8(region,
                                                               // address) << 8;
                                                               // value |=
                                                               // rom_read_8(region,
                                                               // address + 1);
                                                               // return value;
                                                               // }static
                                                               // uint32_t
                                                               // rom_read_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address) { //
                                                               // Lê dois words
                                                               // consecutivos
                                                               // uint32_t value
                                                               // =
                                                               // rom_read_16(region,
                                                               // address) <<
                                                               // 16;    value
                                                               // |=
                                                               // rom_read_16(region,
                                                               // address + 2);
                                                               // return value;
                                                               // }static void
                                                               // rom_write_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint8_t value)
                                                               // {
                                                               // md_mapper_write_rom(&g_md_mapper,
                                                               // address,
                                                               // value);
                                                               // }static void
                                                               // rom_write_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint16_t
                                                               // value) {    //
                                                               // Escreve dois
                                                               // bytes
                                                               // consecutivos
                                                               // rom_write_8(region,
                                                               // address,
                                                               // (value >> 8) &
                                                               // 0xFF);
                                                               // rom_write_8(region,
                                                               // address + 1,
                                                               // value & 0xFF);
                                                               // }static void
                                                               // rom_write_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint32_t
                                                               // value) {    //
                                                               // Escreve dois
                                                               // words
                                                               // consecutivos
                                                               // rom_write_16(region,
                                                               // address,
                                                               // (value >> 16)
                                                               // & 0xFFFF);
                                                               // rom_write_16(region,
                                                               // address + 2,
                                                               // value &
                                                               // 0xFFFF); }/*
                                                               // Implementação
                                                               // dos callbacks
                                                               // do VDP
                                                               // */static
                                                               // uint8_t
                                                               // vdp_read_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address){ //
                                                               // Endereços do
                                                               // VDP:    //
                                                               // 0xC00000-0xC00003:
                                                               // Porta de dados
                                                               // //
                                                               // 0xC00004-0xC00007:
                                                               // Porta de
                                                               // controle    //
                                                               // 0xC00008-0xC0000F:
                                                               // Registradores
                                                               // HV counter
                                                               // switch
                                                               // (address &
                                                               // 0xF)    { case
                                                               // 0: // Porta de
                                                               // dados (byte
                                                               // alto) return
                                                               // (g_vdp_data_buffer
                                                               // >> 8) & 0xFF;
                                                               // case 1: //
                                                               // Porta de dados
                                                               // (byte baixo)
                                                               // return
                                                               // g_vdp_data_buffer
                                                               // & 0xFF; case
                                                               // 4: // Porta de
                                                               // controle (byte
                                                               // alto) return
                                                               // (g_vdp_control_buffer
                                                               // >> 8) & 0xFF;
                                                               // case 5: //
                                                               // Porta de
                                                               // controle (byte
                                                               // baixo) return
                                                               // g_vdp_control_buffer
                                                               // & 0xFF; case
                                                               // 8: // HV
                                                               // counter (byte
                                                               // alto) return
                                                               // (g_vdp_hv_counter
                                                               // >> 8) & 0xFF;
                                                               // case 9: // HV
                                                               // counter (byte
                                                               // baixo) return
                                                               // g_vdp_hv_counter
                                                               // & 0xFF;
                                                               // default:
                                                               // return 0xFF;
                                                               // }}static
                                                               // uint16_t
                                                               // vdp_read_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address){ //
                                                               // Lê dois bytes
                                                               // consecutivos
                                                               // uint16_t value
                                                               // =
                                                               // vdp_read_8(region,
                                                               // address) << 8;
                                                               // value |=
                                                               // vdp_read_8(region,
                                                               // address + 1);
                                                               // return
                                                               // value;}static
                                                               // uint32_t
                                                               // vdp_read_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address){ //
                                                               // Lê dois words
                                                               // consecutivos
                                                               // uint32_t value
                                                               // =
                                                               // vdp_read_16(region,
                                                               // address) <<
                                                               // 16;    value
                                                               // |=
                                                               // vdp_read_16(region,
                                                               // address + 2);
                                                               // return
                                                               // value;}static
                                                               // void
                                                               // vdp_write_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint8_t
                                                               // value){ switch
                                                               // (address &
                                                               // 0xF)    { case
                                                               // 0: // Porta de
                                                               // dados (byte
                                                               // alto)
                                                               // g_vdp_data_buffer
                                                               // =
                                                               // (g_vdp_data_buffer
                                                               // & 0x00FF) |
                                                               // (value << 8);
                                                               // break;    case
                                                               // 1: // Porta de
                                                               // dados (byte
                                                               // baixo)
                                                               // g_vdp_data_buffer
                                                               // =
                                                               // (g_vdp_data_buffer
                                                               // & 0xFF00) |
                                                               // value; //
                                                               // Processa o
                                                               // dado após
                                                               // receber o byte
                                                               // baixo
                                                               // emu_video_write_data(g_vdp,
                                                               // g_vdp_data_buffer);
                                                               // break;    case
                                                               // 4: // Porta de
                                                               // controle (byte
                                                               // alto)
                                                               // g_vdp_control_buffer
                                                               // =
                                                               // (g_vdp_control_buffer
                                                               // & 0x00FF) |
                                                               // (value << 8);
                                                               // break;    case
                                                               // 5: // Porta de
                                                               // controle (byte
                                                               // baixo)
                                                               // g_vdp_control_buffer
                                                               // =
                                                               // (g_vdp_control_buffer
                                                               // & 0xFF00) |
                                                               // value; //
                                                               // Processa o
                                                               // comando após
                                                               // receber o byte
                                                               // baixo
                                                               // emu_video_write_control(g_vdp,
                                                               // g_vdp_control_buffer);
                                                               // break;
                                                               // }}static void
                                                               // vdp_write_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint16_t
                                                               // value){    //
                                                               // Escreve dois
                                                               // bytes
                                                               // consecutivos
                                                               // vdp_write_8(region,
                                                               // address,
                                                               // (value >> 8) &
                                                               // 0xFF);
                                                               // vdp_write_8(region,
                                                               // address + 1,
                                                               // value &
                                                               // 0xFF);}static
                                                               // void
                                                               // vdp_write_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint32_t
                                                               // value){    //
                                                               // Escreve dois
                                                               // words
                                                               // consecutivos
                                                               // vdp_write_16(region,
                                                               // address,
                                                               // (value >> 16)
                                                               // & 0xFFFF);
                                                               // vdp_write_16(region,
                                                               // address + 2,
                                                               // value &
                                                               // 0xFFFF);}/*
                                                               // Implementação
                                                               // dos callbacks
                                                               // de I/O
                                                               // */static
                                                               // uint8_t
                                                               // io_read_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address){ //
                                                               // Endereços de
                                                               // I/O:    //
                                                               // 0xA10000-0xA10001:
                                                               // Versão do
                                                               // hardware    //
                                                               // 0xA10002-0xA10003:
                                                               // Controle do
                                                               // pad 1    //
                                                               // 0xA10004-0xA10005:
                                                               // Controle do
                                                               // pad 2    //
                                                               // 0xA10006-0xA10007:
                                                               // Controle do
                                                               // pad 3    //
                                                               // 0xA10008-0xA10009:
                                                               // Controle do
                                                               // modem    //
                                                               // 0xA1000A-0xA1000B:
                                                               // Controle do
                                                               // SRAM    //
                                                               // 0xA1000C-0xA1000F:
                                                               // Controle do
                                                               // Z80    switch
                                                               // (address &
                                                               // 0xF)    { case
                                                               // 0:          //
                                                               // Versão do
                                                               // hardware (byte
                                                               // alto) return
                                                               // 0x00; //
                                                               // Hardware Type
                                                               // =
                                                               // Overseas/Domestic
                                                               // case 1: //
                                                               // Versão do
                                                               // hardware (byte
                                                               // baixo) return
                                                               // 0x00; //
                                                               // Hardware
                                                               // Version =
                                                               // Version 0 case
                                                               // 2: // Controle
                                                               // do pad 1 (byte
                                                               // alto) return
                                                               // (g_pad1_state
                                                               // >> 8) & 0xFF;
                                                               // case 3: //
                                                               // Controle do
                                                               // pad 1 (byte
                                                               // baixo) return
                                                               // g_pad1_state &
                                                               // 0xFF;    case
                                                               // 4: // Controle
                                                               // do pad 2 (byte
                                                               // alto) return
                                                               // (g_pad2_state
                                                               // >> 8) & 0xFF;
                                                               // case 5: //
                                                               // Controle do
                                                               // pad 2 (byte
                                                               // baixo) return
                                                               // g_pad2_state &
                                                               // 0xFF;    case
                                                               // 0xA: //
                                                               // Controle do
                                                               // SRAM (byte
                                                               // alto) return
                                                               // (g_sram_control
                                                               // >> 8) & 0xFF;
                                                               // case 0xB: //
                                                               // Controle do
                                                               // SRAM (byte
                                                               // baixo) return
                                                               // g_sram_control
                                                               // & 0xFF; case
                                                               // 0xC: //
                                                               // Controle do
                                                               // Z80 (byte
                                                               // alto) return
                                                               // (g_z80_control
                                                               // >> 8) & 0xFF;
                                                               // case 0xD: //
                                                               // Controle do
                                                               // Z80 (byte
                                                               // baixo) return
                                                               // g_z80_control
                                                               // & 0xFF;
                                                               // default:
                                                               // return 0xFF;
                                                               // }}static
                                                               // uint16_t
                                                               // io_read_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address){ //
                                                               // Lê dois bytes
                                                               // consecutivos
                                                               // uint16_t value
                                                               // =
                                                               // io_read_8(region,
                                                               // address) << 8;
                                                               // value |=
                                                               // io_read_8(region,
                                                               // address + 1);
                                                               // return
                                                               // value;}static
                                                               // uint32_t
                                                               // io_read_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address){ //
                                                               // Lê dois words
                                                               // consecutivos
                                                               // uint32_t value
                                                               // =
                                                               // io_read_16(region,
                                                               // address) <<
                                                               // 16;    value
                                                               // |=
                                                               // io_read_16(region,
                                                               // address + 2);
                                                               // return
                                                               // value;}static
                                                               // void
                                                               // io_write_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint8_t
                                                               // value){ switch
                                                               // (address &
                                                               // 0xF)    { case
                                                               // 0xA: //
                                                               // Controle do
                                                               // SRAM (byte
                                                               // alto)
                                                               // g_sram_control
                                                               // =
                                                               // (g_sram_control
                                                               // & 0x00FF) |
                                                               // (value << 8);
                                                               // break;    case
                                                               // 0xB: //
                                                               // Controle do
                                                               // SRAM (byte
                                                               // baixo)
                                                               // g_sram_control
                                                               // =
                                                               // (g_sram_control
                                                               // & 0xFF00) |
                                                               // value; break;
                                                               // case 0xC: //
                                                               // Controle do
                                                               // Z80 (byte
                                                               // alto)
                                                               // g_z80_control
                                                               // =
                                                               // (g_z80_control
                                                               // & 0x00FF) |
                                                               // (value << 8);
                                                               // // Processa o
                                                               // controle do
                                                               // Z80        if
                                                               // (value & 0x01)
                                                               // { // Reset Z80
                                                               // emu_cpu_reset(g_z80_cpu);
                                                               // } break; case
                                                               // 0xD: //
                                                               // Controle do
                                                               // Z80 (byte
                                                               // baixo)
                                                               // g_z80_control
                                                               // =
                                                               // (g_z80_control
                                                               // & 0xFF00) |
                                                               // value; break;
                                                               // }}static void
                                                               // io_write_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint16_t
                                                               // value){    //
                                                               // Escreve dois
                                                               // bytes
                                                               // consecutivos
                                                               // io_write_8(region,
                                                               // address,
                                                               // (value >> 8) &
                                                               // 0xFF);
                                                               // io_write_8(region,
                                                               // address + 1,
                                                               // value &
                                                               // 0xFF);}static
                                                               // void
                                                               // io_write_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint32_t
                                                               // value){    //
                                                               // Escreve dois
                                                               // words
                                                               // consecutivos
                                                               // io_write_16(region,
                                                               // address,
                                                               // (value >> 16)
                                                               // & 0xFFFF);
                                                               // io_write_16(region,
                                                               // address + 2,
                                                               // value &
                                                               // 0xFFFF);}/*
                                                               // Implementação
                                                               // dos callbacks
                                                               // do Z80
                                                               // */static
                                                               // uint8_t
                                                               // z80_read_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address){ // O
                                                               // Z80 tem acesso
                                                               // a:    // - 8KB
                                                               // de RAM própria
                                                               // (0x0000-0x1FFF)
                                                               // // -
                                                               // Registradores
                                                               // do YM2612
                                                               // (0x4000-0x4003)
                                                               // // -
                                                               // Registradores
                                                               // do PSG
                                                               // (0x7F11)    //
                                                               // - Banco de ROM
                                                               // do 68000
                                                               // (0x8000-0xFFFF)
                                                               // if (address <=
                                                               // 0x1FFF)    {
                                                               // // RAM do Z80
                                                               // return
                                                               // g_z80_ram[address];
                                                               // }    else if
                                                               // (address >=
                                                               // 0x4000 &&
                                                               // address <=
                                                               // 0x4003)    {
                                                               // //
                                                               // Registradores
                                                               // do YM2612
                                                               // return
                                                               // emu_audio_read_ym2612(g_audio,
                                                               // address &
                                                               // 0x3);    }
                                                               // else if
                                                               // (address ==
                                                               // 0x7F11)    {
                                                               // // Registrador
                                                               // do PSG return
                                                               // emu_audio_read_psg(g_audio);
                                                               // }    else if
                                                               // (address >=
                                                               // 0x8000)    {
                                                               // // Banco de
                                                               // ROM do 68000
                                                               // uint32_t
                                                               // bank_offset =
                                                               // g_z80_bank_register
                                                               // * 0x8000;
                                                               // return
                                                               // emu_memory_read_8(g_memory,
                                                               // bank_offset +
                                                               // (address -
                                                               // 0x8000));    }
                                                               // return
                                                               // 0xFF;}static
                                                               // uint16_t
                                                               // z80_read_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address){ // O
                                                               // Z80 é um
                                                               // processador de
                                                               // 8 bits, então
                                                               // leituras de 16
                                                               // bits    // são
                                                               // feitas como
                                                               // duas leituras
                                                               // de 8 bits
                                                               // consecutivas
                                                               // uint16_t value
                                                               // =
                                                               // z80_read_8(region,
                                                               // address) << 8;
                                                               // value |=
                                                               // z80_read_8(region,
                                                               // address + 1);
                                                               // return
                                                               // value;}static
                                                               // uint32_t
                                                               // z80_read_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address){ // O
                                                               // Z80 é um
                                                               // processador de
                                                               // 8 bits, então
                                                               // leituras de 32
                                                               // bits    // são
                                                               // feitas como
                                                               // quatro
                                                               // leituras de 8
                                                               // bits
                                                               // consecutivas
                                                               // uint32_t value
                                                               // =
                                                               // z80_read_8(region,
                                                               // address) <<
                                                               // 24;    value
                                                               // |=
                                                               // z80_read_8(region,
                                                               // address + 1)
                                                               // << 16; value
                                                               // |=
                                                               // z80_read_8(region,
                                                               // address + 2)
                                                               // << 8;    value
                                                               // |=
                                                               // z80_read_8(region,
                                                               // address + 3);
                                                               // return
                                                               // value;}static
                                                               // void
                                                               // z80_write_8(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint8_t
                                                               // value){    if
                                                               // (address <=
                                                               // 0x1FFF)    {
                                                               // // RAM do Z80
                                                               // g_z80_ram[address]
                                                               // = value;    }
                                                               // else if
                                                               // (address >=
                                                               // 0x4000 &&
                                                               // address <=
                                                               // 0x4003)    {
                                                               // //
                                                               // Registradores
                                                               // do YM2612
                                                               // emu_audio_write_ym2612(g_audio,
                                                               // address & 0x3,
                                                               // value);    }
                                                               // else if
                                                               // (address ==
                                                               // 0x7F11)    {
                                                               // // Registrador
                                                               // do PSG
                                                               // emu_audio_write_psg(g_audio,
                                                               // value);    }
                                                               // else if
                                                               // (address ==
                                                               // 0x6000)    {
                                                               // // Registrador
                                                               // de banco
                                                               // g_z80_bank_register
                                                               // =
                                                               // (g_z80_bank_register
                                                               // & 0xFF00) |
                                                               // value;    }
                                                               // else if
                                                               // (address ==
                                                               // 0x6001)    {
                                                               // // Registrador
                                                               // de banco (byte
                                                               // alto)
                                                               // g_z80_bank_register
                                                               // =
                                                               // (g_z80_bank_register
                                                               // & 0x00FF) |
                                                               // (value << 8);
                                                               // }}static void
                                                               // z80_write_16(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint16_t
                                                               // value){    //
                                                               // O Z80 é um
                                                               // processador de
                                                               // 8 bits, então
                                                               // escritas de 16
                                                               // bits    // são
                                                               // feitas como
                                                               // duas escritas
                                                               // de 8 bits
                                                               // consecutivas
                                                               // z80_write_8(region,
                                                               // address,
                                                               // (value >> 8) &
                                                               // 0xFF);
                                                               // z80_write_8(region,
                                                               // address + 1,
                                                               // value &
                                                               // 0xFF);}static
                                                               // void
                                                               // z80_write_32(struct
                                                               // memory_region
                                                               // *region,
                                                               // uint32_t
                                                               // address,
                                                               // uint32_t
                                                               // value){    //
                                                               // O Z80 é um
                                                               // processador de
                                                               // 8 bits, então
                                                               // escritas de 32
                                                               // bits    // são
                                                               // feitas como
                                                               // quatro
                                                               // escritas de 8
                                                               // bits
                                                               // consecutivas
                                                               // z80_write_8(region,
                                                               // address,
                                                               // (value >> 24)
                                                               // & 0xFF);
                                                               // z80_write_8(region,
                                                               // address + 1,
                                                               // (value >> 16)
                                                               // & 0xFF);
                                                               // z80_write_8(region,
                                                               // address + 2,
                                                               // (value >> 8) &
                                                               // 0xFF);
                                                               // z80_write_8(region,
                                                               // address + 3,
                                                               // value &
                                                               // 0xFF);}/** *
                                                               // @brief Carrega
                                                               // uma ROM no
                                                               // sistema de
                                                               // memória */bool
                                                               // md_memory_load_rom(const
                                                               // uint8_t
                                                               // *rom_data,
                                                               // uint32_t
                                                               // rom_size) { if
                                                               // (!rom_data ||
                                                               // rom_size == 0
                                                               // || rom_size >
                                                               // MD_ROM_SIZE)
                                                               // {
                                                               // LOG_ERROR("Dados
                                                               // de ROM
                                                               // inválidos: %p,
                                                               // %u bytes",
                                                               // rom_data,
                                                               // rom_size);
                                                               // return false;
                                                               // }     //
                                                               // Copiar dados
                                                               // da ROM para a
                                                               // memória
                                                               // memcpy(g_rom_data,
                                                               // rom_data,
                                                               // rom_size); //
                                                               // Detectar o
                                                               // tipo de mapper
                                                               // com base no
                                                               // conteúdo da
                                                               // ROM
                                                               // md_mapper_type_t
                                                               // mapper_type =
                                                               // md_mapper_detect_type(rom_data,
                                                               // rom_size);
                                                               // LOG_INFO("Tipo
                                                               // de mapper
                                                               // detectado:
                                                               // %d",
                                                               // mapper_type);
                                                               // // Inicializar
                                                               // o mapper
                                                               // apropriado if
                                                               // (!md_mapper_init(&g_md_mapper,
                                                               // mapper_type,
                                                               // g_rom_data,
                                                               // rom_size)) {
                                                               // LOG_ERROR("Falha
                                                               // ao inicializar
                                                               // mapper para a
                                                               // ROM"); return
                                                               // false;    }
                                                               // return true;
                                                               // }/** * @brief
                                                               // Salva os dados
                                                               // de SRAM do
                                                               // jogo em um
                                                               // arquivo */bool
                                                               // md_memory_save_sram(const
                                                               // char
                                                               // *filename) {
                                                               // return
                                                               // md_mapper_save_sram(&g_md_mapper,
                                                               // filename);
                                                               // }/** * @brief
                                                               // Carrega os
                                                               // dados de SRAM
                                                               // do jogo de um
                                                               // arquivo */bool
                                                               // md_memory_load_sram(const
                                                               // char
                                                               // *filename) {
                                                               // return
                                                               // md_mapper_load_sram(&g_md_mapper,
                                                               // filename); }
