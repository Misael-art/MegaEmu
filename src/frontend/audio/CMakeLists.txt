cmake_minimum_required(VERSION 3.10)

project(menu_sounds)

# Encontrar SDL2 (opcional)
find_package(SDL2 QUIET)

if(SDL2_FOUND)
    # Adicionar biblioteca com suporte a SDL2
    add_library(menu_sounds STATIC
        menu_sounds.c
        menu_sounds.h
    )

    # Incluir diretórios
    target_include_directories(menu_sounds PUBLIC
        ${SDL2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # Linkar com SDL2
    target_link_libraries(menu_sounds PUBLIC
        ${SDL2_LIBRARIES}
        m  # Biblioteca math para funções trigonométricas
    )

    set(AUDIO_SOURCES
        # Adicione aqui os arquivos fonte do sistema de áudio
    )

    if(AUDIO_SOURCES)
        add_library(audio STATIC ${AUDIO_SOURCES})

        target_include_directories(audio PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/deps/sdl2/include
        )

        target_link_libraries(audio PRIVATE
            SDL2::SDL2
        )
    endif()
else()
    message(STATUS "SDL2 não encontrado - recursos de áudio serão desabilitados")
endif()
