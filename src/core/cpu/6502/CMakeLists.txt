# 6502 CPU Library
cmake_minimum_required(VERSION 3.10)

# Lista de arquivos fonte
set(CPU_6502_SOURCES
    rp2a03_adapter.c
)

# Lista de arquivos de cabeçalho
set(CPU_6502_HEADERS
    rp2a03_adapter.h
)

# Cria a biblioteca
add_library(cpu_6502_core STATIC ${CPU_6502_SOURCES} ${CPU_6502_HEADERS})

# Inclui diretórios
target_include_directories(cpu_6502_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Adiciona dependências
target_link_libraries(cpu_6502_core
    cpu_core
    nes_cpu  # Biblioteca da CPU do NES
)

# Define flags de compilação
target_compile_definitions(cpu_6502_core PRIVATE
    EMU_CPU_6502_CORE_INTERNAL
)

# Configurações específicas do compilador
if(MSVC)
    target_compile_options(cpu_6502_core PRIVATE /W4)
else()
    target_compile_options(cpu_6502_core PRIVATE -Wall -Wextra)
endif()

# Testes
if(BUILD_TESTING)
    add_executable(cpu_6502_adapter_test
        rp2a03_adapter_test.c
    )

    target_link_libraries(cpu_6502_adapter_test
        cpu_6502_core
    )

    add_test(NAME cpu_6502_adapter_test COMMAND cpu_6502_adapter_test)
endif()
