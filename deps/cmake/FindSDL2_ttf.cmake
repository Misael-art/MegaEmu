# FindSDL2_ttf.cmake
#
# Este módulo define:
#  SDL2_TTF_INCLUDE_DIR - onde encontrar o SDL_ttf.h
#  SDL2_TTF_LIBRARY - biblioteca para vincular contra o SDL2_ttf
#  SDL2_TTF_FOUND - se SDL2_ttf foi encontrado

# Definir caminhos para o SDL2_ttf
set(SDL2_TTF_SEARCH_PATHS
    "${CMAKE_CURRENT_LIST_DIR}/../SDL2_ttf/SDL2_ttf-2.20.2"
    "${CMAKE_SOURCE_DIR}/deps/SDL2_ttf/SDL2_ttf-2.20.2"
    "${CMAKE_SOURCE_DIR}/deps/SDL2_ttf"
)

# Procurar o cabeçalho SDL_ttf.h
find_path(SDL2_TTF_INCLUDE_DIR
    NAMES SDL_ttf.h
    PATHS ${SDL2_TTF_SEARCH_PATHS}
    PATH_SUFFIXES include include/SDL2
)

# Procurar a biblioteca SDL2_ttf
find_library(SDL2_TTF_LIBRARY
    NAMES SDL2_ttf
    PATHS ${SDL2_TTF_SEARCH_PATHS}
    PATH_SUFFIXES lib lib/x64 lib/x86
)

# Definir SDL2_TTF_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_TTF
    REQUIRED_VARS SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR
)

# Definir variáveis de saída
if(SDL2_TTF_FOUND)
    set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY})
    set(SDL2_TTF_INCLUDE_DIRS ${SDL2_TTF_INCLUDE_DIR})

    # Se encontrou a biblioteca DLL, também define o caminho para ela
    # Importante para sistemas Windows
    get_filename_component(SDL2_TTF_LIBRARY_DIR ${SDL2_TTF_LIBRARY} DIRECTORY)
    find_file(SDL2_TTF_DLL
        NAMES SDL2_ttf.dll
        PATHS ${SDL2_TTF_LIBRARY_DIR} ${SDL2_TTF_LIBRARY_DIR}/../bin
    )

    if(SDL2_TTF_DLL)
        set(SDL2_TTF_DLL_DIR ${SDL2_TTF_DLL})
        message(STATUS "SDL2_ttf DLL encontrado: ${SDL2_TTF_DLL}")
    endif()
endif()

mark_as_advanced(SDL2_TTF_INCLUDE_DIR SDL2_TTF_LIBRARY)
