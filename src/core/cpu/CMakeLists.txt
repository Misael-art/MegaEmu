# CMakeLists para módulos de CPU

# Definir bibliotecas para cada tipo de CPU
add_library(mega_emu_cpu_z80 STATIC
    z80/z80.c
)

target_include_directories(mega_emu_cpu_z80 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core
)

set_target_properties(mega_emu_cpu_z80 PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

if(MSVC)
    target_compile_options(mega_emu_cpu_z80 PRIVATE /W4)
else()
    target_compile_options(mega_emu_cpu_z80 PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Adicionar outros tipos de CPU quando necessário
# add_library(mega_emu_cpu_6502 STATIC
#    6502/6502.c
# )

# Biblioteca combinada para todas as CPUs
add_library(mega_emu_cpu
    cpu.c
)

target_include_directories(mega_emu_cpu PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(mega_emu_cpu PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

# CPU Core Library
cmake_minimum_required(VERSION 3.10)

# Adiciona os subdiretórios
add_subdirectory(6502)

# Lista de arquivos fonte
set(CPU_CORE_SOURCES
    cpu_interface.c
)

# Lista de arquivos de cabeçalho
set(CPU_CORE_HEADERS
    cpu_interface.h
)

# Cria a biblioteca
add_library(cpu_core STATIC ${CPU_CORE_SOURCES} ${CPU_CORE_HEADERS})

# Inclui diretórios
target_include_directories(cpu_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Define flags de compilação
target_compile_definitions(cpu_core PRIVATE
    EMU_CPU_CORE_INTERNAL
)

# Configurações específicas do compilador
if(MSVC)
    target_compile_options(cpu_core PRIVATE /W4)
else()
    target_compile_options(cpu_core PRIVATE -Wall -Wextra)
endif()
