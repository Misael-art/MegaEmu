# FindSDL2.cmake
#
# Este módulo define:
#  SDL2_FOUND - se o SDL2 foi encontrado
#  SDL2_INCLUDE_DIRS - diretórios de inclusão do SDL2
#  SDL2_LIBRARIES - bibliotecas do SDL2

# Procura o SDL2 instalado pelo vcpkg
find_path(SDL2_INCLUDE_DIR SDL.h
    PATHS
    ${CMAKE_SOURCE_DIR}/deps/SDL2/include
    $ENV{VCPKG_ROOT}/installed/x64-windows/include
    $ENV{VCPKG_ROOT}/installed/x64-windows/include/SDL2
    PATH_SUFFIXES SDL2
)

find_library(SDL2_LIBRARY
    NAMES SDL2
    PATHS
    ${CMAKE_SOURCE_DIR}/deps/SDL2/lib
    $ENV{VCPKG_ROOT}/installed/x64-windows/lib
)

find_library(SDL2MAIN_LIBRARY
    NAMES SDL2main
    PATHS
    ${CMAKE_SOURCE_DIR}/deps/SDL2/lib
    $ENV{VCPKG_ROOT}/installed/x64-windows/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2 DEFAULT_MSG SDL2_LIBRARY SDL2_INCLUDE_DIR)

if(SDL2_FOUND)
    set(SDL2_LIBRARIES ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY})
    set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
endif()
