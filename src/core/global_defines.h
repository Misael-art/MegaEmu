/**
 * @file global_defines.h
 * @brief Definições globais e macros condicionais para todo o projeto
 *
 * Este arquivo contém definições que devem ser acessíveis em todo o projeto,
 * incluindo macros para detecção de plataforma, tipos comuns e configurações
 * de compilação condicional.
 */

#ifndef MEGA_EMU_GLOBAL_DEFINES_H
#define MEGA_EMU_GLOBAL_DEFINES_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Versão do emulador
 */
#define MEGA_EMU_VERSION_MAJOR 0
#define MEGA_EMU_VERSION_MINOR 1
#define MEGA_EMU_VERSION_PATCH 0

/**
 * Macros para detecção de sistema operacional
 */
#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_MACOS 1
#elif defined(__linux__)
#define PLATFORM_LINUX 1
#elif defined(__ANDROID__)
#define PLATFORM_ANDROID 1
#elif defined(__unix__)
#define PLATFORM_UNIX 1
#else
#define PLATFORM_UNKNOWN 1
#endif

/**
 * Macros para detecção de arquitetura
 */
#if defined(__x86_64__) || defined(_M_X64)
#define ARCH_X86_64 1
#elif defined(__i386) || defined(_M_IX86)
#define ARCH_X86 1
#elif defined(__arm__) || defined(_M_ARM)
#define ARCH_ARM 1
#elif defined(__aarch64__)
#define ARCH_ARM64 1
#else
#define ARCH_UNKNOWN 1
#endif

/**
 * Macros para detecção de compilador
 */
#if defined(_MSC_VER)
#define COMPILER_MSVC 1
#elif defined(__clang__)
#define COMPILER_CLANG 1
#elif defined(__GNUC__)
#define COMPILER_GCC 1
#else
#define COMPILER_UNKNOWN 1
#endif

/**
 * Definições de visibilidade de símbolo
 */
#ifdef COMPILER_MSVC
#define API_EXPORT __declspec(dllexport)
#define API_IMPORT __declspec(dllimport)
#else
#define API_EXPORT __attribute__((visibility("default")))
#define API_IMPORT
#endif

#ifdef MEGA_EMU_EXPORTS
#define MEGA_EMU_API API_EXPORT
#else
#define MEGA_EMU_API API_IMPORT
#endif

/**
 * Macros úteis
 */
#define UNUSED(x) (void)(x)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define KB(x) ((x) * 1024)
#define MB(x) ((x) * 1024 * 1024)

/**
 * Plataformas de console suportadas
 */
#ifdef BUILD_MEGADRIVE
#define SUPPORT_MEGADRIVE 1
#endif

#ifdef BUILD_MASTERSYSTEM
#define SUPPORT_MASTERSYSTEM 1
#endif

#ifdef BUILD_NES
#define SUPPORT_NES 1
#endif

#ifdef BUILD_SNES
#define SUPPORT_SNES 1
#endif

#ifdef BUILD_GAMEBOY
#define SUPPORT_GAMEBOY 1
#endif

#ifdef BUILD_GAMEBOYCOLOR
#define SUPPORT_GAMEBOYCOLOR 1
#endif

#ifdef BUILD_GBA
#define SUPPORT_GBA 1
#endif

#ifdef BUILD_ATARI7800
#define SUPPORT_ATARI7800 1
#endif

#ifdef BUILD_COLECOVISION
#define SUPPORT_COLECOVISION 1
#endif

#ifdef BUILD_ATARILYNX
#define SUPPORT_ATARILYNX 1
#endif

#ifdef BUILD_PCENGINE
#define SUPPORT_PCENGINE 1
#endif

#ifdef BUILD_NEOGEO
#define SUPPORT_NEOGEO 1
#endif

#ifdef BUILD_PLAYSTATION
#define SUPPORT_PLAYSTATION 1
#endif

#ifdef BUILD_SATURN
#define SUPPORT_SATURN 1
#endif

#ifdef BUILD_ATARIJAGUAR
#define SUPPORT_ATARIJAGUAR 1
#endif

#ifdef BUILD_THREEDO
#define SUPPORT_THREEDO 1
#endif

#ifdef BUILD_N64
#define SUPPORT_N64 1
#endif

#ifdef BUILD_DREAMCAST
#define SUPPORT_DREAMCAST 1
#endif

/**
 * Periféricos suportados
 */
#ifdef BUILD_MEGACD
#define SUPPORT_MEGACD 1
#endif

#ifdef BUILD_32X
#define SUPPORT_32X 1
#endif

#ifdef BUILD_FDS
#define SUPPORT_FDS 1
#endif

#ifdef BUILD_GAMEGEAR
#define SUPPORT_GAMEGEAR 1
#endif

#ifdef BUILD_SEGACD
#define SUPPORT_SEGACD 1
#endif

#ifdef BUILD_N64DD
#define SUPPORT_N64DD 1
#endif

/**
 * Chips de processador comuns
 */
#ifdef CPU_Z80_SUPPORT
#define CHIP_Z80 1
#endif

#ifdef CPU_6502_SUPPORT
#define CHIP_6502 1
#endif

#ifdef CPU_M68K_SUPPORT
#define CHIP_M68K 1
#endif

#ifdef CPU_65C816_SUPPORT
#define CHIP_65C816 1
#endif

#ifdef CPU_ARM_SUPPORT
#define CHIP_ARM 1
#endif

#ifdef CPU_MIPS_SUPPORT
#define CHIP_MIPS 1
#endif

#ifdef CPU_SH_SUPPORT
#define CHIP_SH 1
#endif

/**
 * Chips de áudio comuns
 */
#ifdef AUDIO_SN76489_SUPPORT
#define CHIP_SN76489 1
#endif

#ifdef AUDIO_YM2612_SUPPORT
#define CHIP_YM2612 1
#endif

#ifdef AUDIO_NES_APU_SUPPORT
#define CHIP_NES_APU 1
#endif

#ifdef AUDIO_SPC700_SUPPORT
#define CHIP_SPC700 1
#endif

/**
 * Chips de vídeo comuns
 */
#ifdef VIDEO_VDP_MD_SUPPORT
#define CHIP_VDP_MD 1
#endif

#ifdef VIDEO_VDP_SMS_SUPPORT
#define CHIP_VDP_SMS 1
#endif

#ifdef VIDEO_PPU_NES_SUPPORT
#define CHIP_PPU_NES 1
#endif

#ifdef VIDEO_PPU_SNES_SUPPORT
#define CHIP_PPU_SNES 1
#endif

/**
 * Modos de depuração
 */
#ifdef DEBUG
#define DEBUG_MODE 1
#define LOG_LEVEL_DEFAULT 4 // Detalhado
#else
#define DEBUG_MODE 0
#define LOG_LEVEL_DEFAULT 2 // Avisos
#endif

/**
 * Níveis de log
 */
#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_TRACE 5

/**
 * Tipos de dados comuns
 */
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

/**
 * Códigos de retorno comuns
 */
#define MEGA_EMU_SUCCESS 0
#define MEGA_EMU_ERROR -1
#define MEGA_EMU_NOT_IMPL -2
#define MEGA_EMU_INVALID_ARG -3
#define MEGA_EMU_OUT_OF_MEM -4
#define MEGA_EMU_FILE_ERROR -5

#endif /* MEGA_EMU_GLOBAL_DEFINES_H */
