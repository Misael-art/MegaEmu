cmake_minimum_required(VERSION 3.15)
project(mega_emu VERSION 1.0.0)

# Configurar vcpkg se VCPKG_ROOT estiver definido
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Configurar opções do compilador
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Definir opções de build
option(BUILD_NES "Build NES emulator" ON)
option(BUILD_MEGADRIVE "Build Mega Drive emulator" ON)
option(BUILD_SDL_FRONTEND "Build SDL frontend" ON)

# Configurar diretórios de saída
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Encontrar pacotes necessários
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(Freetype REQUIRED)

# Verificar SDL2
find_path(SDL2_INCLUDE_DIR SDL.h PATH_SUFFIXES SDL2)
find_package(SDL2 CONFIG REQUIRED)
message(STATUS "SDL2 encontrado em: ${SDL2_DIR}")

# Adicionar Unity para testes
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/third_party/Unity ${CMAKE_BINARY_DIR}/Unity)

# Adicionar biblioteca de ferramentas
add_library(mega_tools STATIC
    tools/src/tools.cpp
)

target_include_directories(mega_tools PUBLIC
    ${CMAKE_SOURCE_DIR}/tools/include
)

# Configurar propriedades da biblioteca de ferramentas
set_target_properties(mega_tools PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(MSVC)
    target_compile_options(mega_tools PRIVATE /W4)
else()
    target_compile_options(mega_tools PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Adicionar emulador NES
if(BUILD_NES)
    add_executable(mega_emu_nes
        emulators/nes/src/nes.cpp
    )

    target_link_libraries(mega_emu_nes PRIVATE
        mega_tools
    )

    set_target_properties(mega_emu_nes PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    if(MSVC)
        target_compile_options(mega_emu_nes PRIVATE /W4)
    else()
        target_compile_options(mega_emu_nes PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

# Adicionar emulador Mega Drive
if(BUILD_MEGADRIVE)
    add_executable(mega_emu_megadrive
        emulators/megadrive/src/megadrive.cpp
    )

    target_link_libraries(mega_emu_megadrive PRIVATE
        mega_tools
    )

    set_target_properties(mega_emu_megadrive PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    if(MSVC)
        target_compile_options(mega_emu_megadrive PRIVATE /W4)
    else()
        target_compile_options(mega_emu_megadrive PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

# Adicionar frontend SDL
if(BUILD_SDL_FRONTEND)
    add_executable(mega_emu_sdl_frontend
        frontend/sdl/src/main.cpp
    )

    target_link_libraries(mega_emu_sdl_frontend PRIVATE
        mega_tools
        SDL2::SDL2
        SDL2::SDL2main
        OpenGL::GL
        ZLIB::ZLIB
        PNG::PNG
        JPEG::JPEG
        Freetype::Freetype
    )

    set_target_properties(mega_emu_sdl_frontend PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    if(MSVC)
        target_compile_options(mega_emu_sdl_frontend PRIVATE /W4)

        # Copiar DLLs do SDL2 para o diretório de saída no Windows
        add_custom_command(TARGET mega_emu_sdl_frontend POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL2::SDL2>
                $<TARGET_FILE_DIR:mega_emu_sdl_frontend>
        )
    else()
        target_compile_options(mega_emu_sdl_frontend PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

# Adicionar subdiretórios
add_subdirectory(core/cpu)
add_subdirectory(core/video)
add_subdirectory(frontend/sdl)
add_subdirectory(platforms/megadrive)
