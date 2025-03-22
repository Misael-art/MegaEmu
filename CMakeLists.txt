cmake_minimum_required(VERSION 3.15)
project(Mega_Emu VERSION 1.2.5 LANGUAGES C CXX)

# Opções de compilação para cada plataforma
option(BUILD_MEGADRIVE "Compilar suporte para Mega Drive/Genesis" ON)
option(BUILD_MASTERSYSTEM "Compilar suporte para Master System" ON)
option(BUILD_NES "Compilar suporte para NES/Famicom" ON)
option(BUILD_SNES "Compilar suporte para SNES" OFF)

# Opções para periféricos
option(BUILD_MEGACD "Compilar suporte para Mega CD/Sega CD" OFF)
option(BUILD_32X "Compilar suporte para 32X" OFF)
option(BUILD_FDS "Compilar suporte para Famicom Disk System" OFF)

# Configurações de compilação
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configurar compilador
if(MSVC)
    add_compile_options(/W4)
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_DEPRECATE
    )
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Configurar diretórios de saída
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Encontrar dependências
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf)

# Adicionar subdiretórios
add_subdirectory(src)
add_subdirectory(tests)

# Configurar execução de testes
enable_testing()

# Exibir configuração
message(STATUS "=== Configuração do Mega_Emu ===")
message(STATUS "Versão: ${CMAKE_PROJECT_VERSION}")
message(STATUS "Plataformas:")
message(STATUS "  - Mega Drive/Genesis: ${BUILD_MEGADRIVE}")
message(STATUS "  - Master System: ${BUILD_MASTERSYSTEM}")
message(STATUS "  - NES/Famicom: ${BUILD_NES}")
message(STATUS "  - SNES: ${BUILD_SNES}")
message(STATUS "Periféricos:")
message(STATUS "  - Mega CD/Sega CD: ${BUILD_MEGACD}")
message(STATUS "  - 32X: ${BUILD_32X}")
message(STATUS "  - Famicom Disk System: ${BUILD_FDS}")
message(STATUS "================================")
