# CMakeLists.txt para o diretório memory

# Especificar arquivos fonte explicitamente
set(MEMORY_SOURCES
    memory.c
    memory_interface.c
    memory_cache.c
    memory_optimization.c
)

set(MEMORY_HEADERS
    memory_interface.h
    optimized_memory.h
    memory_cache.h
    memory_optimization.h
)

# Adicionar biblioteca
add_library(memory STATIC ${MEMORY_SOURCES} ${MEMORY_HEADERS})

# Definir include directories
target_include_directories(memory
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..  # Para acessar outros módulos do core
)

# Dependências
target_link_libraries(memory
    PRIVATE utils
)

# Flags de compilação
if(MEMORY_USE_OPTIMIZATION)
    target_compile_definitions(memory PRIVATE ENABLE_MEMORY_OPTIMIZATION)
endif()

# Instalar headers e biblioteca
install(TARGETS memory DESTINATION lib)
install(FILES ${MEMORY_HEADERS} DESTINATION include/core/memory)
