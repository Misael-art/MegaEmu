set(SOURCES
    m68k.c
    m68k_instructions.c
    m68k_branch.c
    m68k_cache.c
    m68k_dcache.c
    m68k_exceptions.c
    m68000cpu.cpp
    z80_adapter.c
)

set(HEADERS
    m68k.h
    m68k_timing.h
    m68k_instructions.h
    m68k_branch.h
    m68k_cache.h
    m68k_dcache.h
    m68k_exceptions.h
    cpu_sync.h
    m68000cpu.hpp
    z80_adapter.h
)

# Adicionar os arquivos à biblioteca
target_sources(md_cpu_lib
    PRIVATE
        ${SOURCES}
    PUBLIC
        ${HEADERS}
)

# Incluir diretório de headers
target_include_directories(md_cpu_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Criar biblioteca do CPU
add_library(megadrive_cpu STATIC
    ${SOURCES}
    ${HEADERS}
)

# Definir includestarget_include_directories(megadrive_cpu PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/platforms/megadrive
)

# Definir flags de compilação
target_compile_features(megadrive_cpu PRIVATE cxx_std_17)

# Linkar com outras bibliotecas
target_link_libraries(megadrive_cpu
    utils
)

# Exportar biblioteca
set(MEGADRIVE_CPU_LIBRARY megadrive_cpu PARENT_SCOPE)
set(MEGADRIVE_CPU_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
