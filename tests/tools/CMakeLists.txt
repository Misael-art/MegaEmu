# Configurar testes das ferramentas
message(STATUS "Configurando testes das ferramentas...")

# Lista de testes das ferramentas
set(TOOLS_TESTS
    test_rom_info
    test_rom_dumper
    test_rom_patcher
    test_save_converter
    test_state_manager
    test_debugger
    test_memory_viewer
    test_tile_viewer
    test_palette_viewer
    test_sprite_viewer
)

# Configurar cada teste das ferramentas
foreach(TEST_NAME ${TOOLS_TESTS})
    add_executable(${TEST_NAME}
        ${TEST_NAME}.c
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        Unity::Unity
        mega_emu_tools
    )

    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/tools
    )

    add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_tests_properties(${TEST_NAME} PROPERTIES
        LABELS "unit;tools"
    )
endforeach()

# Configurar testes de integração das ferramentas
add_executable(test_tools_integration
    test_tools_integration.c
)

target_link_libraries(test_tools_integration PRIVATE
    Unity::Unity
    mega_emu_tools
)

target_include_directories(test_tools_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src/tools
)

add_test(NAME test_tools_integration
    COMMAND test_tools_integration
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_tools_integration PROPERTIES
    LABELS "integration;tools"
)

# Configurar testes de compatibilidade das ferramentas
add_executable(test_tools_compatibility
    test_tools_compatibility.c
)

target_link_libraries(test_tools_compatibility PRIVATE
    Unity::Unity
    mega_emu_tools
)

target_include_directories(test_tools_compatibility PRIVATE
    ${CMAKE_SOURCE_DIR}/src/tools
)

add_test(NAME test_tools_compatibility
    COMMAND test_tools_compatibility
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_tools_compatibility PROPERTIES
    LABELS "compatibility;tools"
)

# Configurar testes de performance das ferramentas
add_executable(test_tools_performance
    test_tools_performance.c
)

target_link_libraries(test_tools_performance PRIVATE
    Unity::Unity
    mega_emu_tools
)

target_include_directories(test_tools_performance PRIVATE
    ${CMAKE_SOURCE_DIR}/src/tools
)

add_test(NAME test_tools_performance
    COMMAND test_tools_performance
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_tools_performance PROPERTIES
    LABELS "performance;tools"
)

# Configurar testes de regressão das ferramentas
add_executable(test_tools_regression
    test_tools_regression.c
)

target_link_libraries(test_tools_regression PRIVATE
    Unity::Unity
    mega_emu_tools
)

target_include_directories(test_tools_regression PRIVATE
    ${CMAKE_SOURCE_DIR}/src/tools
)

add_test(NAME test_tools_regression
    COMMAND test_tools_regression
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_tools_regression PROPERTIES
    LABELS "regression;tools"
)

# Configurar testes de benchmark das ferramentas
add_executable(test_tools_benchmark
    test_tools_benchmark.c
)

target_link_libraries(test_tools_benchmark PRIVATE
    Unity::Unity
    mega_emu_tools
)

target_include_directories(test_tools_benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src/tools
)

add_test(NAME test_tools_benchmark
    COMMAND test_tools_benchmark
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_tools_benchmark PROPERTIES
    LABELS "benchmark;tools"
)

message(STATUS "Configuração de testes das ferramentas concluída")
