# CMakeLists.txt para o módulo DevTools

# Adicionar subdiretórios
add_subdirectory(breakpoints)

# Arquivos fonte
set(DEV_TOOLS_SOURCES
    dev_tools.c
)

# Arquivos de cabeçalho
set(DEV_TOOLS_HEADERS
    dev_tools.h
)

# Criar biblioteca
add_library(mega_emu_dev_tools STATIC
    ${DEV_TOOLS_SOURCES}
    ${DEV_TOOLS_HEADERS}
)

# Configurar includes
target_include_directories(mega_emu_dev_tools PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Linkagem com dependências
target_link_libraries(mega_emu_dev_tools PRIVATE
    core
    ${MEGA_EMU_BREAKPOINTS_LIBRARY}
)

# Exportar variável para o diretório pai
set(MEGA_EMU_DEV_TOOLS_LIBRARY mega_emu_dev_tools PARENT_SCOPE)

# Instalar biblioteca (opcional)
install(TARGETS mega_emu_dev_tools DESTINATION lib)
install(FILES ${DEV_TOOLS_HEADERS} DESTINATION include/tools/dev_tools)
