# Configurar testes da CPU do NES
message(STATUS "Configurando testes da CPU do NES...")

# Lista de testes da CPU
set(CPU_TESTS
    test_cpu_instructions
    test_cpu_addressing_modes
    test_cpu_interrupts
    test_cpu_timing
    test_cpu_unofficial_opcodes
    test_cpu_decimal_mode
    test_cpu_flags
    test_cpu_stack
    test_cpu_memory
    test_cpu_dma
)

# Configurar cada teste da CPU
foreach(TEST_NAME ${CPU_TESTS})
    add_executable(${TEST_NAME}
        ${TEST_NAME}.c
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        Unity::Unity
        mega_emu_nes
    )

    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/platforms/nes
        ${CMAKE_SOURCE_DIR}/src/platforms/nes/cpu
    )

    add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_tests_properties(${TEST_NAME} PROPERTIES
        LABELS "unit;nes;cpu"
    )
endforeach()

# Configurar testes de integração da CPU
add_executable(test_cpu_integration
    test_cpu_integration.c
)

target_link_libraries(test_cpu_integration PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_cpu_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/cpu
)

add_test(NAME test_cpu_integration
    COMMAND test_cpu_integration
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_cpu_integration PROPERTIES
    LABELS "integration;nes;cpu"
)

# Configurar testes de compatibilidade da CPU
add_executable(test_cpu_compatibility
    test_cpu_compatibility.c
)

target_link_libraries(test_cpu_compatibility PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_cpu_compatibility PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/cpu
)

add_test(NAME test_cpu_compatibility
    COMMAND test_cpu_compatibility
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_cpu_compatibility PROPERTIES
    LABELS "compatibility;nes;cpu"
)

# Configurar testes de performance da CPU
add_executable(test_cpu_performance
    test_cpu_performance.c
)

target_link_libraries(test_cpu_performance PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_cpu_performance PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/cpu
)

add_test(NAME test_cpu_performance
    COMMAND test_cpu_performance
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_cpu_performance PROPERTIES
    LABELS "performance;nes;cpu"
)

# Configurar testes de regressão da CPU
add_executable(test_cpu_regression
    test_cpu_regression.c
)

target_link_libraries(test_cpu_regression PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_cpu_regression PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/cpu
)

add_test(NAME test_cpu_regression
    COMMAND test_cpu_regression
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_cpu_regression PROPERTIES
    LABELS "regression;nes;cpu"
)

# Configurar testes de benchmark da CPU
add_executable(test_cpu_benchmark
    test_cpu_benchmark.c
)

target_link_libraries(test_cpu_benchmark PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_cpu_benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/cpu
)

add_test(NAME test_cpu_benchmark
    COMMAND test_cpu_benchmark
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_cpu_benchmark PROPERTIES
    LABELS "benchmark;nes;cpu"
)

message(STATUS "Configuração de testes da CPU do NES concluída")
