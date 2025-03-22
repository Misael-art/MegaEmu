# GUI Components CMakeLists.txt

# Coletar arquivos fonte
set(GUI_SOURCES
    gui_common.c
    gui_common.h
    gui_element.h
    gui_manager.c
    gui_manager.h
    gui_rom_selector.h
    gui_save_state.c
    gui_save_state.h
    gui_types.h
    sdl2_backend.c
    sdl2_backend.h
    gui_style.h
)

# Criar biblioteca
add_library(gui STATIC ${GUI_SOURCES})

# Configurar diretórios de inclusão
target_include_directories(gui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/deps/sdl2/include
)

# Vincular dependências
target_link_libraries(gui
    PUBLIC
        SDL2::SDL2
        SDL2::SDL2main
)

# Adicionar SDL2_ttf se disponível
if(TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(gui PUBLIC SDL2_ttf::SDL2_ttf)
    target_compile_definitions(gui PRIVATE USE_SDL2_TTF)
elseif(TARGET sdl2-ttf::SDL2_ttf)
    target_link_libraries(gui PUBLIC sdl2-ttf::SDL2_ttf)
    target_compile_definitions(gui PRIVATE USE_SDL2_TTF)
elseif(SDL2_ttf_FOUND)
    target_link_libraries(gui PUBLIC ${SDL2_TTF_LIBRARIES})
    target_compile_definitions(gui PRIVATE USE_SDL2_TTF)
endif()

# Exportar a biblioteca
set(GUI_LIBRARY gui PARENT_SCOPE)
