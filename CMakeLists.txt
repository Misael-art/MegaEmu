cmake_minimum_required(VERSION 3.20)

# Definir projeto
project(Mega_Emu VERSION 0.1.0 LANGUAGES C CXX)

# Configurar vcpkg (deve ser antes de qualquer find_package)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
    message(STATUS "Usando vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Verificar se o toolchain foi configurado
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE não está definido. Configure VCPKG_ROOT ou especifique o toolchain manualmente.")
endif()

# Opções de build
option(BUILD_TESTS "Build tests" OFF)
option(USE_SDL2 "Use SDL2 for graphics and audio" ON)
option(BUILD_NES "Build NES emulation" ON)
option(BUILD_MEGADRIVE "Build Mega Drive emulation" ON)
option(BUILD_MASTERSYSTEM "Build Master System emulation" ON)
option(BUILD_FRONTEND_SDL "Build SDL2 frontend" ON)
option(BUILD_FRONTEND_QT "Build Qt frontend" OFF)
option(BUILD_FRONTEND_REACT "Build React frontend" ON)
option(BUILD_TOOLS "Build Mega_tools" ON)

# Configurar diretórios de saída
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Configurar compilador
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Flags de compilação
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Encontrar dependências
find_package(SDL2 REQUIRED)
if(BUILD_FRONTEND_QT)
    find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
endif()
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(Freetype REQUIRED)

# Configurar testes
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Adicionar subdiretórios
add_subdirectory(src)

# Configurar emuladores
if(BUILD_NES)
    add_subdirectory(src/platforms/nes)
endif()

if(BUILD_MEGADRIVE)
    add_subdirectory(src/platforms/megadrive)
endif()

if(BUILD_MASTERSYSTEM)
    add_subdirectory(src/platforms/mastersystem)
endif()

# Configurar frontends
if(BUILD_FRONTEND_SDL)
    add_subdirectory(src/frontend/sdl)
endif()

if(BUILD_FRONTEND_QT)
    add_subdirectory(src/frontend/qt)
endif()

# Adicionar frontend React se habilitado
if(BUILD_FRONTEND_REACT)
    add_subdirectory(src/frontend/react)
endif()

# Configurar ferramentas
if(BUILD_TOOLS)
    add_subdirectory(src/tools)
endif()

# Configurar documentação
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Gerando documentação com Doxygen"
        VERBATIM)
endif()

# Configurar instalação
include(GNUInstallDirs)
install(TARGETS mega_emu
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Configurar pacote
include(CPack)
set(CPACK_PACKAGE_NAME "mega-emu")
set(CPACK_PACKAGE_VENDOR "MegaEmu Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Multi-platform emulator")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Configurar variáveis de versão
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/version.h
)
