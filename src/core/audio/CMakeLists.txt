# CMakeLists.txt para o diretório audio

# Definir fontes básicas de áudio
set(AUDIO_SOURCES
    audio.c
    audio.h
    audio_interface.h
)

# Adicionar fontes para gerenciamento de áudio
list(APPEND AUDIO_SOURCES
    audio_system.c
    audio_system.h
    audio_mixer.c
    audio_mixer.h
    audio_buffer.c
    audio_buffer.h
)

# Adicionar chips de áudio comuns entre plataformas

# PSG (Programmable Sound Generator)
set(PSG_SOURCES
    chips/sn76489/sn76489.c
    chips/sn76489/sn76489.h
)
list(APPEND AUDIO_SOURCES ${PSG_SOURCES})

# YM2612 (FM Synthesis - Mega Drive)
if(BUILD_MEGADRIVE)
    set(YM2612_SOURCES
        chips/ym2612/ym2612.c
        chips/ym2612/ym2612.h
        chips/ym2612/ym2612_tables.c
        chips/ym2612/ym2612_tables.h
    )
    list(APPEND AUDIO_SOURCES ${YM2612_SOURCES})
    add_definitions(-DAUDIO_YM2612_SUPPORT)
endif()

# APU (Audio Processing Unit - NES)
if(BUILD_NES)
    set(APU_SOURCES
        chips/apu/nes_apu.c
        chips/apu/nes_apu.h
        chips/apu/nes_apu_pulse.c
        chips/apu/nes_apu_triangle.c
        chips/apu/nes_apu_noise.c
        chips/apu/nes_apu_dmc.c
    )
    list(APPEND AUDIO_SOURCES ${APU_SOURCES})
    add_definitions(-DAUDIO_NES_APU_SUPPORT)
endif()

# SPC700 (SNES)
if(BUILD_SNES)
    set(SPC700_SOURCES
        chips/spc700/spc700.c
        chips/spc700/spc700.h
        chips/spc700/dsp.c
        chips/spc700/dsp.h
    )
    list(APPEND AUDIO_SOURCES ${SPC700_SOURCES})
    add_definitions(-DAUDIO_SPC700_SUPPORT)
endif()

# Game Boy APU
if(BUILD_GAMEBOY OR BUILD_GAMEBOYCOLOR)
    set(GB_APU_SOURCES
        chips/gb_apu/gb_apu.c
        chips/gb_apu/gb_apu.h
    )
    list(APPEND AUDIO_SOURCES ${GB_APU_SOURCES})
    add_definitions(-DAUDIO_GB_APU_SUPPORT)
endif()

# POKEY (Atari)
if(BUILD_ATARI7800 OR BUILD_ATARILYNX)
    set(POKEY_SOURCES
        chips/pokey/pokey.c
        chips/pokey/pokey.h
    )
    list(APPEND AUDIO_SOURCES ${POKEY_SOURCES})
    add_definitions(-DAUDIO_POKEY_SUPPORT)
endif()

# SCSP (Saturn)
if(BUILD_SATURN)
    set(SCSP_SOURCES
        chips/scsp/scsp.c
        chips/scsp/scsp.h
    )
    list(APPEND AUDIO_SOURCES ${SCSP_SOURCES})
    add_definitions(-DAUDIO_SCSP_SUPPORT)
endif()

# SPU (PlayStation)
if(BUILD_PLAYSTATION)
    set(SPU_SOURCES
        chips/spu/spu.c
        chips/spu/spu.h
    )
    list(APPEND AUDIO_SOURCES ${SPU_SOURCES})
    add_definitions(-DAUDIO_SPU_SUPPORT)
endif()

# AICA (Dreamcast)
if(BUILD_DREAMCAST)
    set(AICA_SOURCES
        chips/aica/aica.c
        chips/aica/aica.h
    )
    list(APPEND AUDIO_SOURCES ${AICA_SOURCES})
    add_definitions(-DAUDIO_AICA_SUPPORT)
endif()

# Criar biblioteca
add_library(mega_emu_audio STATIC ${AUDIO_SOURCES})

# Configurar inclusões
target_include_directories(mega_emu_audio PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
)

# Configurar dependências
target_link_libraries(mega_emu_audio PUBLIC
    SDL2::SDL2)
