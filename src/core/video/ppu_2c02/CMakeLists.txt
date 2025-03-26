# PPU 2C02 Library
cmake_minimum_required(VERSION 3.10)

# Lista de arquivos fonte
set(PPU_2C02_SOURCES
    ppu_2c02_adapter.c
)

# Lista de arquivos de cabeçalho
set(PPU_2C02_HEADERS
    ppu_2c02_adapter.h
)

# Cria a biblioteca
add_library(ppu_2c02_core STATIC ${PPU_2C02_SOURCES} ${PPU_2C02_HEADERS})

# Inclui diretórios
target_include_directories(ppu_2c02_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Adiciona dependências
target_link_libraries(ppu_2c02_core
    video_core
    nes_ppu  # Biblioteca da PPU do NES
)

# Define flags de compilação
target_compile_definitions(ppu_2c02_core PRIVATE
    EMU_PPU_2C02_CORE_INTERNAL
)

# Configurações específicas do compilador
if(MSVC)
    target_compile_options(ppu_2c02_core PRIVATE /W4)
else()
    target_compile_options(ppu_2c02_core PRIVATE -Wall -Wextra)
endif()

# Testes
if(BUILD_TESTING)
    add_executable(ppu_2c02_adapter_test
        ppu_2c02_adapter_test.c
    )

    target_link_libraries(ppu_2c02_adapter_test
        ppu_2c02_core
    )

    add_test(NAME ppu_2c02_adapter_test COMMAND ppu_2c02_adapter_test)
endif()
