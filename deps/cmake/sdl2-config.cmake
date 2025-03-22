# SDL2 CMake configuration file

set(SDL2_VERSION "2.32.2")
set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/../sdl2/${SDL2_VERSION}/include")
set(SDL2_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/../sdl2/${SDL2_VERSION}/lib")
set(SDL2_DLL_DIR "${CMAKE_CURRENT_LIST_DIR}/../sdl2/${SDL2_VERSION}/bin")

# Definir os targets do SDL2
if(NOT TARGET SDL2::SDL2)
    add_library(SDL2::SDL2 SHARED IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
        IMPORTED_LOCATION "${SDL2_DLL_DIR}/SDL2.dll"
        IMPORTED_IMPLIB "${SDL2_LIBRARIES}/SDL2.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
    )
endif()

if(NOT TARGET SDL2::SDL2main)
    add_library(SDL2::SDL2main STATIC IMPORTED)
    set_target_properties(SDL2::SDL2main PROPERTIES
        IMPORTED_LOCATION "${SDL2_LIBRARIES}/SDL2main.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
    )
endif()

# Verificar se os arquivos existem
if(NOT EXISTS "${SDL2_INCLUDE_DIRS}/SDL.h")
    message(FATAL_ERROR "SDL2 headers não encontrados em ${SDL2_INCLUDE_DIRS}")
endif()

if(NOT EXISTS "${SDL2_LIBRARIES}/SDL2.lib")
    message(FATAL_ERROR "SDL2.lib não encontrado em ${SDL2_LIBRARIES}")
endif()

if(NOT EXISTS "${SDL2_DLL_DIR}/SDL2.dll")
    message(FATAL_ERROR "SDL2.dll não encontrado em ${SDL2_DLL_DIR}")
endif()

mark_as_advanced(SDL2_INCLUDE_DIRS SDL2_LIBRARIES SDL2_DLL_DIR)
