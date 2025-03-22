# CMakeLists.txt para a plataforma Mega Drive# Criar os diretórios para organização do códigoset(MD_DIRS    cpu    video    audio    memory    io)# Criar diretórios se não existiremforeach(DIR ${MD_DIRS})    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DIR}")        file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${DIR}")    endif()endforeach()# Coletar arquivos fonte (separando C e C++)file(GLOB_RECURSE MD_C_SOURCES "*.c")file(GLOB_RECURSE MD_C_HEADERS "*.h")file(GLOB_RECURSE MD_CPP_SOURCES "*.cpp")file(GLOB_RECURSE MD_CPP_HEADERS "*.hpp")# Definir a biblioteca da plataforma Mega Driveset(MEGADRIVE_SOURCE_FILES
    megadrive.c
    md_core.c

    cpu/m68k_adapter.c
    cpu/z80_adapter.c

    video/vdp.c
    video/vdp_dma.c
    video/vdp_sprites.c

    audio/audio_system.c
    audio/ym2612.c
    audio/sn76489.c

    memory/memory.c
    memory/md_mapper.c

    io/controller.c

    state/md_save_state.c
)

set(MEGADRIVE_HEADER_FILES
    megadrive.h
    md_core.h

    cpu/m68k_adapter.h
    cpu/z80_adapter.h

    video/vdp.h
    video/vdp_dma.h

    audio/audio_system.h
    audio/ym2612.h
    audio/sn76489.h
    audio/audio_visualizer.h

    memory/memory.h
    memory/md_mapper.h

    io/controller.h

    state/md_save_state.h
)

add_library(megadrive STATIC ${MEGADRIVE_SOURCE_FILES} ${MEGADRIVE_HEADER_FILES})# Configurar as propriedades da linguagem para compilação mistaset_target_properties(megadrive PROPERTIES    CXX_STANDARD 17    CXX_STANDARD_REQUIRED ON    CXX_EXTENSIONS OFF    POSITION_INDEPENDENT_CODE ON)# Configurar flags específicas de compilação para C e C++target_compile_options(megadrive PRIVATE    $<$<COMPILE_LANGUAGE:C>:${C_FLAGS}>    $<$<COMPILE_LANGUAGE:CXX>:${CXX_FLAGS}>)# Dependênciastarget_link_libraries(megadrive    core    utils)# Exportar headers para outros componentestarget_include_directories(megadrive PUBLIC    ${CMAKE_CURRENT_SOURCE_DIR})# Adicionar flags de compilação específicas para a plataformatarget_compile_definitions(megadrive PRIVATE    EMU_PLATFORM_MEGADRIVE    $<$<COMPILE_LANGUAGE:CXX>:MEGA_EMU_CPP_BUILD>  # Define para código C++)# Instalar biblioteca e headersinstall(TARGETS megadrive DESTINATION lib)install(FILES ${MD_C_HEADERS} DESTINATION include/platforms/megadrive)install(FILES ${MD_CPP_HEADERS} DESTINATION include/platforms/megadrive)
