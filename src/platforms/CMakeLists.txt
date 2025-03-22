# CMakeLists.txt para o diretório platforms

# Plataformas base
set(PLATFORMS_COMMON_SOURCES
    common/platform_base.c
    common/platform_types.c
    common/platform_utils.c)

# Definir fontes para o NES
set(NES_SOURCES
    nes/nes.c
    nes/apu/nes_apu.c
    nes/cartridge/mapper5.c
    nes/cartridge/mapper5_original.c
    nes/cartridge/nes_cartridge.c
    nes/cpu/nes_cpu.c
    nes/cpu/rp2a03.c
    nes/debug/nes_debug.c
    nes/input/nes_input.c
    nes/memory/nes_memory.c
    nes/ppu/nes_ppu.c
    nes/ppu/nes_ppu_temp.c
    nes/runner/nes_rom_runner.c
    nes/save/nes_save_state.c)

# Definir fontes para o Mega Drive
set(MEGADRIVE_SOURCES
    megadrive/megadrive.c
    megadrive/md_core.c
    megadrive/audio/ym2612.c
    megadrive/cpu/m68k.c
    megadrive/cpu/m68k_branch.c
    megadrive/cpu/m68k_cache.c
    megadrive/cpu/m68k_dcache.c
    megadrive/cpu/m68k_exceptions.c
    megadrive/cpu/m68k_instructions.c
    megadrive/cpu/z80_adapter.c
    megadrive/video/vdp.c)

# Definir fontes para o Master System
set(MASTERSYSTEM_SOURCES
    mastersystem/mastersystem.c
    mastersystem/cpu/z80_adapter.c
    mastersystem/audio/sms_psg.c
    mastersystem/video/sms_vdp.c
    mastersystem/memory/sms_memory.c
    mastersystem/io/sms_io.c)

# Definir fontes para o SNES
set(SNES_SOURCES
    snes/snes.c
    snes/cpu/w65c816.c
    snes/ppu/snes_ppu.c
    snes/apu/spc700.c
    snes/memory/snes_memory.c)

# Definir fontes para Game Boy
set(GAMEBOY_SOURCES
    gameboy/gameboy.c
    gameboy/cpu/lr35902.c
    gameboy/ppu/gb_ppu.c
    gameboy/apu/gb_apu.c
    gameboy/memory/gb_memory.c)

# Definir fontes para Game Boy Color
set(GAMEBOYCOLOR_SOURCES
    gameboycolor/gameboycolor.c
    gameboycolor/cpu/lr35902_gbc.c
    gameboycolor/ppu/gbc_ppu.c
    gameboycolor/apu/gbc_apu.c
    gameboycolor/memory/gbc_memory.c)

# Definir fontes para Game Boy Advance
set(GBA_SOURCES
    gba/gba.c
    gba/cpu/arm7tdmi.c
    gba/ppu/gba_ppu.c
    gba/apu/gba_apu.c
    gba/memory/gba_memory.c)

# Definir fontes para Atari 7800
set(ATARI7800_SOURCES
    atari7800/atari7800.c
    atari7800/cpu/m6502.c
    atari7800/tia/tia.c
    atari7800/memory/a7800_memory.c)

# Definir fontes para ColecoVision
set(COLECOVISION_SOURCES
    colecovision/colecovision.c
    colecovision/cpu/z80_adapter.c
    colecovision/vdp/tms9918.c
    colecovision/memory/coleco_memory.c)

# Definir fontes para Atari Lynx
set(ATARILYNX_SOURCES
    atarilynx/atarilynx.c
    atarilynx/cpu/wdc65c02.c
    atarilynx/mikey/mikey.c
    atarilynx/suzy/suzy.c
    atarilynx/memory/lynx_memory.c)

# Definir fontes para TurboGrafx-16 / PC Engine
set(PCENGINE_SOURCES
    pcengine/pcengine.c
    pcengine/cpu/huc6280.c
    pcengine/vdc/vdc.c
    pcengine/memory/pce_memory.c)

# Definir fontes para Neo Geo AES
set(NEOGEO_SOURCES
    neogeo/neogeo.c
    neogeo/cpu/m68k_neogeo.c
    neogeo/cpu/z80_adapter.c
    neogeo/video/neogeo_video.c
    neogeo/audio/neogeo_audio.c
    neogeo/memory/neogeo_memory.c)

# Definir fontes para PlayStation
set(PLAYSTATION_SOURCES
    playstation/playstation.c
    playstation/cpu/r3000a.c
    playstation/gpu/gpu.c
    playstation/spu/spu.c
    playstation/memory/ps_memory.c)

# Definir fontes para Sega Saturn
set(SATURN_SOURCES
    saturn/saturn.c
    saturn/cpu/sh2.c
    saturn/vdp/vdp1.c
    saturn/vdp/vdp2.c
    saturn/audio/scsp.c
    saturn/memory/saturn_memory.c)

# Definir fontes para Atari Jaguar
set(ATARIJAGUAR_SOURCES
    atarijaguar/atarijaguar.c
    atarijaguar/cpu/m68000.c
    atarijaguar/gpu/gpu.c
    atarijaguar/dsp/jerry.c
    atarijaguar/memory/jaguar_memory.c)

# Definir fontes para 3DO
set(THREEDO_SOURCES
    3do/3do.c
    3do/cpu/arm60.c
    3do/dsp/dsp.c
    3do/clio/clio.c
    3do/memory/3do_memory.c)

# Definir fontes para Nintendo 64
set(N64_SOURCES
    n64/n64.c
    n64/cpu/vr4300.c
    n64/rsp/rsp.c
    n64/rdp/rdp.c
    n64/memory/n64_memory.c)

# Definir fontes para Dreamcast
set(DREAMCAST_SOURCES
    dreamcast/dreamcast.c
    dreamcast/cpu/sh4.c
    dreamcast/gpu/powervr2.c
    dreamcast/aica/aica.c
    dreamcast/memory/dc_memory.c)

# Inicializar lista de plataformas
set(PLATFORMS_SOURCES ${PLATFORMS_COMMON_SOURCES})

# Adicionar plataformas conforme configuração
if(BUILD_NES)
    list(APPEND PLATFORMS_SOURCES ${NES_SOURCES})
    add_definitions(-DBUILD_NES)
endif()

if(BUILD_MEGADRIVE)
    list(APPEND PLATFORMS_SOURCES ${MEGADRIVE_SOURCES})
    add_definitions(-DBUILD_MEGADRIVE)
endif()

if(BUILD_MASTERSYSTEM)
    list(APPEND PLATFORMS_SOURCES ${MASTERSYSTEM_SOURCES})
    add_definitions(-DBUILD_MASTERSYSTEM)
endif()

if(BUILD_SNES)
    list(APPEND PLATFORMS_SOURCES ${SNES_SOURCES})
    add_definitions(-DBUILD_SNES)
endif()

if(BUILD_GAMEBOY)
    list(APPEND PLATFORMS_SOURCES ${GAMEBOY_SOURCES})
    add_definitions(-DBUILD_GAMEBOY)
endif()

if(BUILD_GAMEBOYCOLOR)
    list(APPEND PLATFORMS_SOURCES ${GAMEBOYCOLOR_SOURCES})
    add_definitions(-DBUILD_GAMEBOYCOLOR)
endif()

if(BUILD_GBA)
    list(APPEND PLATFORMS_SOURCES ${GBA_SOURCES})
    add_definitions(-DBUILD_GBA)
endif()

if(BUILD_ATARI7800)
    list(APPEND PLATFORMS_SOURCES ${ATARI7800_SOURCES})
    add_definitions(-DBUILD_ATARI7800)
endif()

if(BUILD_COLECOVISION)
    list(APPEND PLATFORMS_SOURCES ${COLECOVISION_SOURCES})
    add_definitions(-DBUILD_COLECOVISION)
endif()

if(BUILD_ATARILYNX)
    list(APPEND PLATFORMS_SOURCES ${ATARILYNX_SOURCES})
    add_definitions(-DBUILD_ATARILYNX)
endif()

if(BUILD_PCENGINE)
    list(APPEND PLATFORMS_SOURCES ${PCENGINE_SOURCES})
    add_definitions(-DBUILD_PCENGINE)
endif()

if(BUILD_NEOGEO)
    list(APPEND PLATFORMS_SOURCES ${NEOGEO_SOURCES})
    add_definitions(-DBUILD_NEOGEO)
endif()

if(BUILD_PLAYSTATION)
    list(APPEND PLATFORMS_SOURCES ${PLAYSTATION_SOURCES})
    add_definitions(-DBUILD_PLAYSTATION)
endif()

if(BUILD_SATURN)
    list(APPEND PLATFORMS_SOURCES ${SATURN_SOURCES})
    add_definitions(-DBUILD_SATURN)
endif()

if(BUILD_ATARIJAGUAR)
    list(APPEND PLATFORMS_SOURCES ${ATARIJAGUAR_SOURCES})
    add_definitions(-DBUILD_ATARIJAGUAR)
endif()

if(BUILD_THREEDO)
    list(APPEND PLATFORMS_SOURCES ${THREEDO_SOURCES})
    add_definitions(-DBUILD_THREEDO)
endif()

if(BUILD_N64)
    list(APPEND PLATFORMS_SOURCES ${N64_SOURCES})
    add_definitions(-DBUILD_N64)
endif()

if(BUILD_DREAMCAST)
    list(APPEND PLATFORMS_SOURCES ${DREAMCAST_SOURCES})
    add_definitions(-DBUILD_DREAMCAST)
endif()

# Adicionar suporte a periféricos
if(BUILD_MEGACD AND BUILD_MEGADRIVE)
    list(APPEND PLATFORMS_SOURCES megadrive/peripherals/megacd/megacd.c)
    add_definitions(-DBUILD_MEGACD)
endif()

if(BUILD_32X AND BUILD_MEGADRIVE)
    list(APPEND PLATFORMS_SOURCES megadrive/peripherals/32x/32x.c)
    add_definitions(-DBUILD_32X)
endif()

if(BUILD_FDS AND BUILD_NES)
    list(APPEND PLATFORMS_SOURCES nes/peripherals/fds/fds.c)
    add_definitions(-DBUILD_FDS)
endif()

if(BUILD_GAMEGEAR AND BUILD_MASTERSYSTEM)
    list(APPEND PLATFORMS_SOURCES mastersystem/peripherals/gamegear/gamegear.c)
    add_definitions(-DBUILD_GAMEGEAR)
endif()

if(BUILD_SEGACD AND BUILD_SATURN)
    list(APPEND PLATFORMS_SOURCES saturn/peripherals/segacd/segacd.c)
    add_definitions(-DBUILD_SEGACD)
endif()

if(BUILD_N64DD AND BUILD_N64)
    list(APPEND PLATFORMS_SOURCES n64/peripherals/64dd/64dd.c)
    add_definitions(-DBUILD_N64DD)
endif()

# Criar biblioteca
add_library(mega_emu_platforms STATIC ${PLATFORMS_SOURCES})

# Configurar inclusões
target_include_directories(mega_emu_platforms PUBLIC
    ${CMAKE_SOURCE_DIR}/src)

# Configurar dependências
target_link_libraries(mega_emu_platforms PUBLIC
    mega_emu_core
    mega_emu_utils
    SDL2::SDL2)
