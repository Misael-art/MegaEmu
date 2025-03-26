# Z80 Core Library CMakeLists.txt

set(SOURCES
    z80.c
    z80_instructions.c
)

set(HEADERS
    z80.h
    z80_instructions.h
)

# Adicionar os arquivos à biblioteca
add_library(z80_core_lib STATIC ${SOURCES} ${HEADERS})

# Incluir diretório de headers
target_include_directories(z80_core_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/utils
)

# Linkar com outras bibliotecas
target_link_libraries(z80_core_lib
    utils
)

# Exportar biblioteca
set(Z80_CORE_LIBRARY z80_core_lib PARENT_SCOPE)
set(Z80_CORE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
