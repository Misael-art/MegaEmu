# Frontend CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# Tentar encontrar SDL2, mas torná-lo opcional
find_package(SDL2 QUIET)

if(SDL2_FOUND)
    message(STATUS "SDL2 encontrado, habilitando recursos gráficos")
    add_definitions(-DUSE_SDL2)
else()
    message(STATUS "SDL2 não encontrado, recursos gráficos serão desabilitados")
endif()

# Tentar encontrar SDL2_image, mas torná-lo opcional
find_package(SDL2_image CONFIG QUIET)
if(NOT SDL2_image_FOUND)
    find_package(SDL2_image QUIET)
endif()

if(SDL2_image_FOUND)
    message(STATUS "SDL2_image encontrado, habilitando recursos de imagem avançados")
    add_definitions(-DUSE_SDL2_IMAGE)
else()
    message(STATUS "SDL2_image não encontrado, recursos de imagem avançados serão desabilitados")
endif()

# Tentar encontrar SDL2_ttf, mas torná-lo opcional
find_package(SDL2_ttf CONFIG QUIET)
if(NOT SDL2_ttf_FOUND)
    find_package(SDL2_ttf QUIET)
endif()

if(SDL2_ttf_FOUND)
    message(STATUS "SDL2_ttf encontrado, habilitando recursos de texto avançados")
    add_definitions(-DUSE_SDL2_TTF)
else()
    message(STATUS "SDL2_ttf não encontrado, recursos de texto avançados serão desabilitados")
endif()

# Listar os diretórios de inclusão
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/input
    ${CMAKE_CURRENT_SOURCE_DIR}/../core
)

if(SDL2_FOUND)
    include_directories(${SDL2_INCLUDE_DIRS})
endif()

# Adicionar o diretório de inclusão do SDL2_image se disponível
if(TARGET SDL2_image::SDL2_image)
    # Nada a fazer - será usado via target
elseif(SDL2_IMAGE_INCLUDE_DIRS)
    include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
endif()

# Adicionar o diretório de inclusão do SDL2_ttf se disponível
if(TARGET SDL2_ttf::SDL2_ttf)
    # Nada a fazer - será usado via target
elseif(SDL2_TTF_INCLUDE_DIRS)
    include_directories(${SDL2_TTF_INCLUDE_DIRS})
endif()

# Adicionar subdiretórios
add_subdirectory(gui)
add_subdirectory(renderer)
add_subdirectory(audio)
add_subdirectory(input)

# Arquivo fonte principal do frontend
set(FRONTEND_SOURCES
    gui_manager_cpp_bridge.c
    gui_manager_cpp_bridge.cpp
    gui_manager_cpp_bridge.h
    gui_includes.h
    console_types.h
    render_system.cpp
    render_system.hpp
    menu_system.hpp
)

set(FRONTEND_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/deps/sdl2/include
)

add_library(frontend STATIC ${FRONTEND_SOURCES})

target_include_directories(frontend PUBLIC
    ${FRONTEND_INCLUDE_DIRS}
)

if(SDL2_FOUND)
    target_link_libraries(frontend PRIVATE
        SDL2::SDL2
        SDL2::SDL2main
    )
endif()

# Vincular SDL2_ttf com o nome correto
if(TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(frontend PRIVATE SDL2_ttf::SDL2_ttf)
elseif(TARGET sdl2-ttf::SDL2_ttf)
    target_link_libraries(frontend PRIVATE sdl2-ttf::SDL2_ttf)
else()
    # Tentar usar o nome simples como último recurso
    if(SDL2_ttf_FOUND)
        target_link_libraries(frontend PRIVATE SDL2_ttf)
    endif()
endif()

# Configurações específicas para Windows
if(WIN32)
    target_link_libraries(frontend PRIVATE
        wsock32
        ws2_32
    )
endif()

# Configurações específicas para Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(frontend PRIVATE
        pthread
        dl
    )
endif()

# Exportar a biblioteca
set(MEGA_EMU_FRONTEND_LIBRARY frontend PARENT_SCOPE)
