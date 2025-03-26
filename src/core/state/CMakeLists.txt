cmake_minimum_required(VERSION 3.10)

# Arquivos fonte e cabeçalho
set(STATE_SOURCES
    state_interface.c
)

set(STATE_HEADERS
    state_interface.h
)

# Biblioteca state_core
add_library(state_core STATIC
    ${STATE_SOURCES}
    ${STATE_HEADERS}
)

# Inclui diretórios para acesso público
target_include_directories(state_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Flags de compilação
if(MSVC)
    target_compile_options(state_core PRIVATE /W4 /WX)
else()
    target_compile_options(state_core PRIVATE -Wall -Wextra -Werror)
endif()

# Testes
enable_testing()

add_executable(state_test
    state_interface_test.c
)

target_link_libraries(state_test
    PRIVATE
        state_core
)

add_test(NAME state_test COMMAND state_test)
