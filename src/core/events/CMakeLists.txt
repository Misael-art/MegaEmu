# CMakeLists.txt para o diretório events

# Arquivos fonte
set(EVENTS_SOURCES
    events.c
    event_dispatcher.c
    event_logger.c
    node_events.c
    priority_queue.c
    events_enhanced.c
)

# Arquivos de cabeçalho
set(EVENTS_HEADERS
    events_interface.h
    event_logger.h
    node_events.h
    priority_queue.h
    events_enhanced.h
)

# Adicionar biblioteca
add_library(events STATIC ${EVENTS_SOURCES} ${EVENTS_HEADERS})

# Definir include directories
target_include_directories(events
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..  # Para acessar outros módulos do core
)

# Dependências
target_link_libraries(events
    PRIVATE utils
)

# Flags de compilação
if(EVENTS_USE_ENHANCED)
    target_compile_definitions(events PRIVATE ENABLE_ENHANCED_EVENTS)
endif()

# Instalar headers e biblioteca
install(TARGETS events DESTINATION lib)
install(FILES ${EVENTS_HEADERS} DESTINATION include/core/events)
