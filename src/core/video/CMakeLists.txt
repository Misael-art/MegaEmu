# CMakeLists.txt para o diretório video

# Definir fontes básicas de vídeo
set(VIDEO_SOURCES
    video.c
    video.h
    video_interface.h
)

# Adicionar fontes para gerenciamento de vídeo
list(APPEND VIDEO_SOURCES
    video_system.c
    video_system.h
    video_buffer.c
    video_buffer.h
    video_renderer.c
    video_renderer.h
)

# Adicionar chips de vídeo comuns entre plataformas

# VDP (Mega Drive / Genesis)
if(BUILD_MEGADRIVE)
    set(VDP_MD_SOURCES
        chips/vdp_md/vdp.c
        chips/vdp_md/vdp.h
        chips/vdp_md/vdp_render.c
        chips/vdp_md/vdp_render.h
    )
    list(APPEND VIDEO_SOURCES ${VDP_MD_SOURCES})
    add_definitions(-DVIDEO_VDP_MD_SUPPORT)
endif()

# PPU (Picture Processing Unit - NES)
if(BUILD_NES)
    set(PPU_NES_SOURCES
        chips/ppu_nes/ppu.c
        chips/ppu_nes/ppu.h
        chips/ppu_nes/ppu_render.c
        chips/ppu_nes/ppu_render.h
    )
    list(APPEND VIDEO_SOURCES ${PPU_NES_SOURCES})
    add_definitions(-DVIDEO_PPU_NES_SUPPORT)
endif()

# PPU (Picture Processing Unit - SNES)
if(BUILD_SNES)
    set(PPU_SNES_SOURCES
        chips/ppu_snes/ppu.c
        chips/ppu_snes/ppu.h
        chips/ppu_snes/ppu_render.c
        chips/ppu_snes/ppu_render.h
    )
    list(APPEND VIDEO_SOURCES ${PPU_SNES_SOURCES})
    add_definitions(-DVIDEO_PPU_SNES_SUPPORT)
endif()

# VDP (Master System / Game Gear)
if(BUILD_MASTERSYSTEM OR BUILD_GAMEGEAR)
    set(VDP_SMS_SOURCES
        chips/vdp_sms/vdp.c
        chips/vdp_sms/vdp.h
        chips/vdp_sms/vdp_render.c
        chips/vdp_sms/vdp_render.h
    )
    list(APPEND VIDEO_SOURCES ${VDP_SMS_SOURCES})
    add_definitions(-DVIDEO_VDP_SMS_SUPPORT)
endif()

# TMS9918 (ColecoVision)
if(BUILD_COLECOVISION)
    set(TMS9918_SOURCES
        chips/tms9918/tms9918.c
        chips/tms9918/tms9918.h
    )
    list(APPEND VIDEO_SOURCES ${TMS9918_SOURCES})
    add_definitions(-DVIDEO_TMS9918_SUPPORT)
endif()

# Game Boy PPU
if(BUILD_GAMEBOY OR BUILD_GAMEBOYCOLOR)
    set(GB_PPU_SOURCES
        chips/gb_ppu/gb_ppu.c
        chips/gb_ppu/gb_ppu.h
    )
    list(APPEND VIDEO_SOURCES ${GB_PPU_SOURCES})
    add_definitions(-DVIDEO_GB_PPU_SUPPORT)
endif()

# Game Boy Advance PPU
if(BUILD_GBA)
    set(GBA_PPU_SOURCES
        chips/gba_ppu/gba_ppu.c
        chips/gba_ppu/gba_ppu.h
    )
    list(APPEND VIDEO_SOURCES ${GBA_PPU_SOURCES})
    add_definitions(-DVIDEO_GBA_PPU_SUPPORT)
endif()

# TIA (Atari)
if(BUILD_ATARI7800)
    set(TIA_SOURCES
        chips/tia/tia.c
        chips/tia/tia.h
    )
    list(APPEND VIDEO_SOURCES ${TIA_SOURCES})
    add_definitions(-DVIDEO_TIA_SUPPORT)
endif()

# VDC (PC Engine / TurboGrafx-16)
if(BUILD_PCENGINE)
    set(VDC_SOURCES
        chips/vdc/vdc.c
        chips/vdc/vdc.h
    )
    list(APPEND VIDEO_SOURCES ${VDC_SOURCES})
    add_definitions(-DVIDEO_VDC_SUPPORT)
endif()

# Neo Geo Video
if(BUILD_NEOGEO)
    set(NEOGEO_VIDEO_SOURCES
        chips/neogeo/neogeo_video.c
        chips/neogeo/neogeo_video.h
    )
    list(APPEND VIDEO_SOURCES ${NEOGEO_VIDEO_SOURCES})
    add_definitions(-DVIDEO_NEOGEO_SUPPORT)
endif()

# PlayStation GPU
if(BUILD_PLAYSTATION)
    set(PS_GPU_SOURCES
        chips/ps_gpu/gpu.c
        chips/ps_gpu/gpu.h
    )
    list(APPEND VIDEO_SOURCES ${PS_GPU_SOURCES})
    add_definitions(-DVIDEO_PS_GPU_SUPPORT)
endif()

# Saturn VDP1 & VDP2
if(BUILD_SATURN)
    set(SATURN_VDP_SOURCES
        chips/saturn_vdp/vdp1.c
        chips/saturn_vdp/vdp1.h
        chips/saturn_vdp/vdp2.c
        chips/saturn_vdp/vdp2.h
    )
    list(APPEND VIDEO_SOURCES ${SATURN_VDP_SOURCES})
    add_definitions(-DVIDEO_SATURN_VDP_SUPPORT)
endif()

# Dreamcast PowerVR2
if(BUILD_DREAMCAST)
    set(POWERVR2_SOURCES
        chips/powervr2/powervr2.c
        chips/powervr2/powervr2.h
    )
    list(APPEND VIDEO_SOURCES ${POWERVR2_SOURCES})
    add_definitions(-DVIDEO_POWERVR2_SUPPORT)
endif()

# N64 RDP/RSP
if(BUILD_N64)
    set(N64_VIDEO_SOURCES
        chips/n64_video/rdp.c
        chips/n64_video/rdp.h
        chips/n64_video/rsp.c
        chips/n64_video/rsp.h
    )
    list(APPEND VIDEO_SOURCES ${N64_VIDEO_SOURCES})
    add_definitions(-DVIDEO_N64_SUPPORT)
endif()

# Criar biblioteca
add_library(mega_emu_video STATIC ${VIDEO_SOURCES})

# Configurar inclusões
target_include_directories(mega_emu_video PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
)

# Configurar dependências
target_link_libraries(mega_emu_video PUBLIC
    SDL2::SDL2)

# CMakeLists para módulos de vídeo (PPU)

# Definir bibliotecas para cada tipo de PPU
add_library(mega_emu_ppu_core STATIC
    ppu/ppu.c
)

target_include_directories(mega_emu_ppu_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core
)

set_target_properties(mega_emu_ppu_core PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

if(MSVC)
    target_compile_options(mega_emu_ppu_core PRIVATE /W4)
else()
    target_compile_options(mega_emu_ppu_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Adicionar implementações específicas quando necessário
# add_library(mega_emu_ppu_nes STATIC
#    ppu/nes/nes_ppu.c
# )

# Biblioteca combinada para todas as PPUs
add_library(mega_emu_ppu
    ppu.c
)

target_include_directories(mega_emu_ppu PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(mega_emu_ppu PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON)

# Video Core Library
cmake_minimum_required(VERSION 3.10)

# Adiciona os subdiretórios
add_subdirectory(ppu_2c02)

# Lista de arquivos fonte
set(VIDEO_CORE_SOURCES
    ppu_interface.c
)

# Lista de arquivos de cabeçalho
set(VIDEO_CORE_HEADERS
    ppu_interface.h
)

# Cria a biblioteca
add_library(video_core STATIC ${VIDEO_CORE_SOURCES} ${VIDEO_CORE_HEADERS})

# Inclui diretórios
target_include_directories(video_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Define flags de compilação
target_compile_definitions(video_core PRIVATE
    EMU_VIDEO_CORE_INTERNAL
)

# Configurações específicas do compilador
if(MSVC)
    target_compile_options(video_core PRIVATE /W4)
else()
    target_compile_options(video_core PRIVATE -Wall -Wextra)
endif()
