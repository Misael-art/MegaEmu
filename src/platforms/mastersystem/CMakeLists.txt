# CMakeLists.txt para a plataforma Master System

# Criar os diretórios para organização do código
set(MS_DIRS
    cpu
    video
    audio
    memory
    io
)

# Criar diretórios se não existirem
foreach(DIR ${MS_DIRS})
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DIR}")
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${DIR}")
    endif()
endforeach()

# Definir fontes específicas para a plataforma Master System
set(MS_SOURCES
    mastersystem.c
    cpu/z80_adapter.c
    memory/sms_memory.c
    video/sms_vdp.c
    audio/sms_psg.c
    io/sms_io.c
)

# Encontrar cabeçalhos correspondentes
file(GLOB_RECURSE MS_HEADERS "*.h")

# Definir a biblioteca da plataforma Master System
add_library(emu_platform_mastersystem STATIC
    ${MS_SOURCES}
    ${MS_HEADERS})

# Dependências
target_link_libraries(emu_platform_mastersystem
    emu_core
    emu_cpu_z80)

# Exportar headers para outros componentes
target_include_directories(emu_platform_mastersystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src)

# Adicionar flags de compilação específicas para a plataforma
target_compile_definitions(emu_platform_mastersystem PRIVATE
    EMU_PLATFORM_MASTERSYSTEM)

# Instalar biblioteca e headers
install(TARGETS emu_platform_mastersystem DESTINATION lib)
install(FILES ${MS_HEADERS} DESTINATION include/platforms/mastersystem)

# Adicionar uma regra para testes unitários, se solicitado
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
