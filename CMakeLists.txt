cmake_minimum_required(VERSION 3.10)

# Definir projeto
project(mega_emu VERSION 1.0.0 LANGUAGES C CXX)

# Configurar vcpkg
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Configurar opções de compilação
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configurar opções de build
option(BUILD_NES "Build NES emulator" ON)
option(BUILD_MEGADRIVE "Build Mega Drive emulator" ON)
option(BUILD_SDL_FRONTEND "Build SDL frontend" ON)
option(BUILD_CORE_CPU "Build core CPU components" ON)
option(BUILD_CORE_PPU "Build core PPU components" ON)

# Configurar diretórios de saída
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Encontrar pacotes necessários
find_package(OpenGL REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(Lua REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

# Incluir diretórios
include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src
)

# Adicionar componentes do core
if(BUILD_CORE_CPU)
    add_subdirectory(src/core/cpu)
endif()

if(BUILD_CORE_PPU)
    add_subdirectory(src/core/video)
endif()

# Adicionar subdiretórios
add_subdirectory(src/tools)
add_subdirectory(src/frontend/sdl)
add_subdirectory(src/platforms/megadrive)

# Adicionar arquivos fonte
file(GLOB_RECURSE SOURCES
    "src/*.c"
    "src/*.cpp"
)

# Criar executável
add_executable(${PROJECT_NAME} ${SOURCES})

# Vincular bibliotecas
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    ${LUA_LIBRARIES}
    Boost::system
    Boost::filesystem
    OpenGL::GL
)

# Copiar recursos para o diretório de build
file(COPY ${PROJECT_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})

# Configurar testes
enable_testing()

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

# Instalar
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/resources/
    DESTINATION share/${PROJECT_NAME}
)

# Configurar pacote
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Emulador multi-plataforma para sistemas retro")
set(CPACK_PACKAGE_VENDOR "MegaEmu Team")

include(CPack)
