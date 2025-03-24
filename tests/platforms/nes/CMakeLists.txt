# Adiciona o diretório atual ao caminho de inclusão
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Configura o Unity Test Framework
include(${CMAKE_SOURCE_DIR}/cmake/unity.cmake)

# Arquivos fonte dos testes
set(TEST_SOURCES
    test_mappers.c
    test_mmc3.c
    test_mmc5.c
    test_mapper2.c
    test_mapper3.c
    test_mapper4.c
    test_mapper6.c
    cartridge/mappers/test_mapper7.c
)

# Diretórios para encontrar dependências
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/unity/src
)

# Cria a biblioteca de teste
add_library(unity_test STATIC
    ${CMAKE_SOURCE_DIR}/thirdparty/unity/src/unity.c
)

# Define a lista de testes
set(TEST_NAMES
    test_mappers
    test_mmc3
    test_mmc5
    test_mapper2
    test_mapper3
    test_mapper4
    test_mapper6
    test_mapper7
)

# Para cada teste, cria um executável
foreach(TEST_NAME ${TEST_NAMES})
    add_executable(${TEST_NAME} ${TEST_NAME}.c)
    target_link_libraries(${TEST_NAME} unity_test mega_emu_nes)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

# Configurar testes do NES
message(STATUS "Configurando testes do NES...")

# Adicionar subdiretórios
add_subdirectory(cartridge)
add_subdirectory(cpu)
add_subdirectory(ppu)
add_subdirectory(apu)
add_subdirectory(memory)

# Configurar testes de integração do NES
add_executable(test_nes_integration
    test_nes_integration.c
)

target_link_libraries(test_nes_integration PRIVATE
    Unity::Unity
    mega_emu_nes
)

add_test(NAME test_nes_integration
    COMMAND test_nes_integration
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_nes_integration PROPERTIES
    LABELS "integration;nes"
)

# Configurar testes de compatibilidade do NES
add_executable(test_nes_compatibility
    test_nes_compatibility.c
)

target_link_libraries(test_nes_compatibility PRIVATE
    Unity::Unity
    mega_emu_nes
)

add_test(NAME test_nes_compatibility
    COMMAND test_nes_compatibility
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_nes_compatibility PROPERTIES
    LABELS "compatibility;nes"
)

# Configurar testes de performance do NES
add_executable(test_nes_performance
    test_nes_performance.c
)

target_link_libraries(test_nes_performance PRIVATE
    Unity::Unity
    mega_emu_nes
)

add_test(NAME test_nes_performance
    COMMAND test_nes_performance
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_nes_performance PROPERTIES
    LABELS "performance;nes"
)

# Configurar testes de regressão do NES
add_executable(test_nes_regression
    test_nes_regression.c
)

target_link_libraries(test_nes_regression PRIVATE
    Unity::Unity
    mega_emu_nes
)

add_test(NAME test_nes_regression
    COMMAND test_nes_regression
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_nes_regression PROPERTIES
    LABELS "regression;nes"
)

# Configurar testes de stress do NES
add_executable(test_nes_stress
    test_nes_stress.c
)

target_link_libraries(test_nes_stress PRIVATE
    Unity::Unity
    mega_emu_nes
)

add_test(NAME test_nes_stress
    COMMAND test_nes_stress
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_nes_stress PROPERTIES
    LABELS "stress;nes"
)

# Configurar testes de carga do NES
add_executable(test_nes_load
    test_nes_load.c
)

target_link_libraries(test_nes_load PRIVATE
    Unity::Unity
    mega_emu_nes
)

add_test(NAME test_nes_load
    COMMAND test_nes_load
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_nes_load PROPERTIES
    LABELS "load;nes"
)

# Configurar testes de segurança do NES
add_executable(test_nes_security
    test_nes_security.c
)

target_link_libraries(test_nes_security PRIVATE
    Unity::Unity
    mega_emu_nes
)

add_test(NAME test_nes_security
    COMMAND test_nes_security
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_nes_security PROPERTIES
    LABELS "security;nes"
)

# Configurar testes de benchmark do NES
add_executable(test_nes_benchmark
    test_nes_benchmark.c
)

target_link_libraries(test_nes_benchmark PRIVATE
    Unity::Unity
    mega_emu_nes
)

add_test(NAME test_nes_benchmark
    COMMAND test_nes_benchmark
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_nes_benchmark PROPERTIES
    LABELS "benchmark;nes"
)

message(STATUS "Configuração de testes do NES concluída")
