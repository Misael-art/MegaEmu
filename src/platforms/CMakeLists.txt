# CMakeLists.txt para o diretório platforms

# Configuração das plataformas suportadas

# Arquivos comuns
set(COMMON_SOURCES
    common/platform_base.c
)

# Arquivos do NES
set(NES_SOURCES
    nes/cartridge/mapper0.c
    nes/cartridge/mapper1.c
)

# Criar biblioteca para as plataformas
add_library(mega_emu_platforms
    ${COMMON_SOURCES}
    ${NES_SOURCES}
)

# Adicionar diretórios de include
target_include_directories(mega_emu_platforms PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

# Configurar flags de compilação
if(MSVC)
    target_compile_options(mega_emu_platforms PRIVATE /W4)
else()
    target_compile_options(mega_emu_platforms PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Configurar opções de plataforma
if(BUILD_NES)
    target_compile_definitions(mega_emu_platforms PRIVATE BUILD_NES)
endif()
