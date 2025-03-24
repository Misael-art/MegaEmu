# Configuração do projeto de testes
cmake_minimum_required(VERSION 3.10)

# Adiciona os arquivos fonte dos testes
set(TEST_SOURCES
    test_mappers.c
    test_mmc3.c
    test_mmc5.c)

# Adiciona o Unity como dependência
include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.5.2
)
FetchContent_MakeAvailable(unity)

# Cria o executável de teste
add_executable(test_mappers ${TEST_SOURCES})

# Adiciona os diretórios de include
target_include_directories(test_mappers PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${unity_SOURCE_DIR}/src)

# Vincula as bibliotecas necessárias
target_link_libraries(test_mappers PRIVATE
    unity
    nes_core)
