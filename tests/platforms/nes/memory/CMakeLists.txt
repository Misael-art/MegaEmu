# Configurar testes da memória do NES
message(STATUS "Configurando testes da memória do NES...")

# Lista de testes da memória
set(MEMORY_TESTS
    test_memory_ram
    test_memory_rom
    test_memory_mapper
    test_memory_ppu
    test_memory_apu
    test_memory_io
    test_memory_dma
    test_memory_bank_switching
    test_memory_mirroring
    test_memory_save_states
)

# Configurar cada teste da memória
foreach(TEST_NAME ${MEMORY_TESTS})
    add_executable(${TEST_NAME}
        ${TEST_NAME}.c
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        Unity::Unity
        mega_emu_nes
    )

    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/platforms/nes
        ${CMAKE_SOURCE_DIR}/src/platforms/nes/memory
    )

    add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_tests_properties(${TEST_NAME} PROPERTIES
        LABELS "unit;nes;memory"
    )
endforeach()

# Configurar testes de integração da memória
add_executable(test_memory_integration
    test_memory_integration.c
)

target_link_libraries(test_memory_integration PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_memory_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/memory
)

add_test(NAME test_memory_integration
    COMMAND test_memory_integration
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_memory_integration PROPERTIES
    LABELS "integration;nes;memory"
)

# Configurar testes de compatibilidade da memória
add_executable(test_memory_compatibility
    test_memory_compatibility.c
)

target_link_libraries(test_memory_compatibility PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_memory_compatibility PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/memory
)

add_test(NAME test_memory_compatibility
    COMMAND test_memory_compatibility
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_memory_compatibility PROPERTIES
    LABELS "compatibility;nes;memory"
)

# Configurar testes de performance da memória
add_executable(test_memory_performance
    test_memory_performance.c
)

target_link_libraries(test_memory_performance PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_memory_performance PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/memory
)

add_test(NAME test_memory_performance
    COMMAND test_memory_performance
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_memory_performance PROPERTIES
    LABELS "performance;nes;memory"
)

# Configurar testes de regressão da memória
add_executable(test_memory_regression
    test_memory_regression.c
)

target_link_libraries(test_memory_regression PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_memory_regression PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/memory
)

add_test(NAME test_memory_regression
    COMMAND test_memory_regression
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_memory_regression PROPERTIES
    LABELS "regression;nes;memory"
)

# Configurar testes de benchmark da memória
add_executable(test_memory_benchmark
    test_memory_benchmark.c
)

target_link_libraries(test_memory_benchmark PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_memory_benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/memory
)

add_test(NAME test_memory_benchmark
    COMMAND test_memory_benchmark
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_memory_benchmark PROPERTIES
    LABELS "benchmark;nes;memory"
)

message(STATUS "Configuração de testes da memória do NES concluída")
