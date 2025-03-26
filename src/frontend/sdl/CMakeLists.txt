cmake_minimum_required(VERSION 3.10)

# Encontrar bibliotecas SDL
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)

# Incluir diretórios de cabeçalho
include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

# Fontes do frontend SDL
set(SDL_FRONTEND_SOURCES
    # Arquivos existentes
    sdl_frontend.c
    sdl_frontend_adapter.c
    sdl_game_renderer.c
    sdl_menu.c
    frontend_config.c

    # Novos arquivos
    sdl_rom_selector.c
    sdl_save_states.c
    sdl_hotkeys.c
    sdl_screenshot.c
    sdl_video_recorder.c
    sdl_input_config.c
    sdl_audio.c
    src/main.cpp
)

# Cabeçalhos do frontend SDL
set(SDL_FRONTEND_HEADERS
    # Arquivos existentes
    sdl_frontend.h
    sdl_frontend_state.h
    sdl_game_renderer.h
    sdl_menu.h
    frontend_config.h

    # Novos arquivos
    sdl_rom_selector.h
    sdl_save_states.h
    sdl_hotkeys.h
    sdl_screenshot.h
    sdl_video_recorder.h
    sdl_input_config.h
    sdl_audio.h
)

# Criar biblioteca para o frontend SDL
add_library(sdl_frontend STATIC
    ${SDL_FRONTEND_SOURCES}
    ${SDL_FRONTEND_HEADERS}
)

# Vincular bibliotecas SDL
target_link_libraries(sdl_frontend
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    core_lib
    utils_lib
)

# Criar executável separado para testes do SDL
add_executable(sdl_frontend_test
    test_sdl_frontend.c
)

# Vincular com a biblioteca do frontend SDL
target_link_libraries(sdl_frontend_test
    sdl_frontend
    core_lib
    utils_lib
)

# Criar executável principal do frontend SDL
add_executable(mega_emu_sdl
    src/main.cpp
)

# Vincular com a biblioteca do frontend SDL
target_link_libraries(mega_emu_sdl
    sdl_frontend
    core_lib
    utils_lib
    platforms_lib
)

# Copiar arquivos de assets para o diretório de build
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# Configurar diretórios de saída
set_target_properties(sdl_frontend_test mega_emu_sdl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Configurar instalação
install(TARGETS mega_emu_sdl
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets
    DESTINATION share/mega_emu
)

# Regras de compilação específicas para diferentes plataformas
if(WIN32)
    # Adicionar ícone do Windows e recursos
    set(RC_FILE windows/mega_emu.rc)
    target_sources(mega_emu_sdl PRIVATE ${RC_FILE})

    # Configurar para aplicação Windows (não console)
    set_target_properties(mega_emu_sdl PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

if(APPLE)
    # Configurações específicas para macOS
    set_target_properties(mega_emu_sdl PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/macos/Info.plist
        MACOSX_BUNDLE_ICON_FILE mega_emu.icns
    )

    # Copiar ícone para o bundle
    file(COPY ${CMAKE_SOURCE_DIR}/macos/mega_emu.icns
         DESTINATION ${CMAKE_BINARY_DIR}/bin/mega_emu_sdl.app/Contents/Resources)
endif()

# Mensagem informativa
message(STATUS "Frontend SDL configurado com sucesso")

add_library(mega_emu_frontend_sdl
    sdl_frontend.c
)

target_include_directories(mega_emu_frontend_sdl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mega_emu_frontend_sdl PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    OpenGL::GL
)

set_target_properties(mega_emu_frontend_sdl PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON)
