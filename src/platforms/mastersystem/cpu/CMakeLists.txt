set(SOURCES
    z80_adapter.c
)

set(HEADERS
    z80_adapter.h
)

# Adicionar os arquivos à biblioteca
add_library(sms_cpu_lib STATIC ${SOURCES} ${HEADERS})

# Incluir diretório de headers
target_include_directories(sms_cpu_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/utils
        ${CMAKE_SOURCE_DIR}/src/platforms/mastersystem
)

# Linkar com outras bibliotecas
target_link_libraries(sms_cpu_lib
    utils
    z80_core_lib
)

# Exportar biblioteca
set(SMS_CPU_LIBRARY sms_cpu_lib PARENT_SCOPE)
set(SMS_CPU_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
