# Core Library
cmake_minimum_required(VERSION 3.10)

# Adiciona os subdiretórios
add_subdirectory(cpu)
add_subdirectory(video)
add_subdirectory(memory)
add_subdirectory(cartridge)
add_subdirectory(state)

# Lista de arquivos fonte
set(CORE_SOURCES
    core.c
)

# Lista de arquivos de cabeçalho
set(CORE_HEADERS
    core.h
)

# Cria a biblioteca
add_library(core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

# Inclui diretórios
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Adiciona dependências
target_link_libraries(core
    PRIVATE
        cpu_core
        video_core
        memory_core
        cartridge_core
        state_core
)

# Define flags de compilação
target_compile_definitions(core PRIVATE
    EMU_CORE_INTERNAL
)

# Configurações específicas do compilador
if(MSVC)
    target_compile_options(core PRIVATE /W4 /WX)
else()
    target_compile_options(core PRIVATE -Wall -Wextra -Werror)
endif()

# Habilita testes
enable_testing()

# Cria o executável de teste
add_executable(core_test core_test.c)

# Vincula as bibliotecas necessárias
target_link_libraries(core_test
    PRIVATE
        core
)

# Adiciona o teste ao CTest
add_test(NAME core_test COMMAND core_test)
