# CMakeLists.txt para a plataforma SNES

# Verificar se a compilação da plataforma está habilitada
if(NOT BUILD_SNES)
    return()
endif()

# Conjunto de arquivos fonte
set(SNES_SOURCES
    snes_placeholder.c
)

# Criar biblioteca
add_library(emu_platform_snes STATIC ${SNES_SOURCES})

# Configurar dependências
target_link_libraries(emu_platform_snes
    emu_core
)

# Configurar diretórios de inclusão
target_include_directories(emu_platform_snes PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Definições de compilação
target_compile_definitions(emu_platform_snes PRIVATE
    EMU_PLATFORM_SNES
)

# Instalação
install(TARGETS emu_platform_snes
    ARCHIVE DESTINATION lib
)
