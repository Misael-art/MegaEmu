# CMakeLists.txt para o componente de cartuchos do NES

cmake_minimum_required(VERSION 3.14)

# Definir arquivos fonte C
set(CARTRIDGE_SOURCES
    nes_cartridge.c
    nes_cartridge.h
    mapper0.c
    mapper0.h
    mapper1.c
    mapper1.h
    # Arquivos C++ existentes
    nes_cartridge.cpp
    nes_cartridge.hpp
    # Nova interface híbrida
    nes_mapper.h
    nes_mapper.cpp
    cartridge.hpp
    mapper.hpp
)

# Definir arquivos de cabeçalho
set(NES_CARTRIDGE_HEADERS
    nes_cartridge.h
)

# Subdiretório de mappers para implementações específicas
add_subdirectory(mappers)

# Criar biblioteca do cartridge
add_library(nes_cartridge STATIC ${CARTRIDGE_SOURCES})

# Habilitar C++ 14 para recursos necessários
set_target_properties(nes_cartridge PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Definir includes
target_include_directories(nes_cartridge PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/mappers
    ${CMAKE_SOURCE_DIR}/src
)

# Definições para compilação
target_compile_definitions(nes_cartridge PRIVATE
    NES_CARTRIDGE_IMPLEMENTATION
)

# Linkar com outras bibliotecas
target_link_libraries(nes_cartridge
    nes_core
    utils
)

# Adicionar flags de compilação específicas
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(nes_cartridge PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
    )
endif()

# Exportar biblioteca
set(NES_CARTRIDGE_LIBRARY nes_cartridge PARENT_SCOPE)
set(NES_CARTRIDGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
