# CMakeLists.txt para o módulo de breakpoints

# Arquivos fonte
set(BREAKPOINTS_SOURCES
    breakpoints.c
)

# Arquivos de cabeçalho
set(BREAKPOINTS_HEADERS
    breakpoints.h
)

# Criar biblioteca
add_library(mega_emu_breakpoints STATIC
    ${BREAKPOINTS_SOURCES}
    ${BREAKPOINTS_HEADERS}
)

# Configurar includes
target_include_directories(mega_emu_breakpoints PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Linkagem com dependências
target_link_libraries(mega_emu_breakpoints PRIVATE
    core
)

# Exportar variável para o diretório pai
set(MEGA_EMU_BREAKPOINTS_LIBRARY mega_emu_breakpoints PARENT_SCOPE)

# Instalar biblioteca (opcional)
install(TARGETS mega_emu_breakpoints DESTINATION lib)
install(FILES ${BREAKPOINTS_HEADERS} DESTINATION include/tools/dev_tools/breakpoints)
