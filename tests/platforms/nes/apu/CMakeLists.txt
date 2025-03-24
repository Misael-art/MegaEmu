# Configurar testes da APU do NES
message(STATUS "Configurando testes da APU do NES...")

# Lista de testes da APU
set(APU_TESTS
    test_apu_pulse1
    test_apu_pulse2
    test_apu_triangle
    test_apu_noise
    test_apu_dmc
    test_apu_registers
    test_apu_timing
    test_apu_frame_counter
    test_apu_mixer
    test_apu_filters
)

# Configurar cada teste da APU
foreach(TEST_NAME ${APU_TESTS})
    add_executable(${TEST_NAME}
        ${TEST_NAME}.c
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        Unity::Unity
        mega_emu_nes
    )

    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/platforms/nes
        ${CMAKE_SOURCE_DIR}/src/platforms/nes/apu
    )

    add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_tests_properties(${TEST_NAME} PROPERTIES
        LABELS "unit;nes;apu"
    )
endforeach()

# Configurar testes de integração da APU
add_executable(test_apu_integration
    test_apu_integration.c
)

target_link_libraries(test_apu_integration PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_apu_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/apu
)

add_test(NAME test_apu_integration
    COMMAND test_apu_integration
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_apu_integration PROPERTIES
    LABELS "integration;nes;apu"
)

# Configurar testes de compatibilidade da APU
add_executable(test_apu_compatibility
    test_apu_compatibility.c
)

target_link_libraries(test_apu_compatibility PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_apu_compatibility PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/apu
)

add_test(NAME test_apu_compatibility
    COMMAND test_apu_compatibility
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_apu_compatibility PROPERTIES
    LABELS "compatibility;nes;apu"
)

# Configurar testes de performance da APU
add_executable(test_apu_performance
    test_apu_performance.c
)

target_link_libraries(test_apu_performance PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_apu_performance PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/apu
)

add_test(NAME test_apu_performance
    COMMAND test_apu_performance
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_apu_performance PROPERTIES
    LABELS "performance;nes;apu"
)

# Configurar testes de regressão da APU
add_executable(test_apu_regression
    test_apu_regression.c
)

target_link_libraries(test_apu_regression PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_apu_regression PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/apu
)

add_test(NAME test_apu_regression
    COMMAND test_apu_regression
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_apu_regression PROPERTIES
    LABELS "regression;nes;apu"
)

# Configurar testes de benchmark da APU
add_executable(test_apu_benchmark
    test_apu_benchmark.c
)

target_link_libraries(test_apu_benchmark PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_apu_benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/apu
)

add_test(NAME test_apu_benchmark
    COMMAND test_apu_benchmark
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_apu_benchmark PROPERTIES
    LABELS "benchmark;nes;apu"
)

# Configurar testes de áudio da APU
add_executable(test_apu_audio
    test_apu_audio.c
)

target_link_libraries(test_apu_audio PRIVATE
    Unity::Unity
    mega_emu_nes
    SDL2::SDL2
)

target_include_directories(test_apu_audio PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/apu
)

add_test(NAME test_apu_audio
    COMMAND test_apu_audio
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_apu_audio PROPERTIES
    LABELS "audio;nes;apu"
)

message(STATUS "Configuração de testes da APU do NES concluída")
