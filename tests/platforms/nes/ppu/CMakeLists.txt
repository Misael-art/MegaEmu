# Configurar testes da PPU do NES
message(STATUS "Configurando testes da PPU do NES...")

# Lista de testes da PPU
set(PPU_TESTS
    test_ppu_registers
    test_ppu_rendering
    test_ppu_sprites
    test_ppu_background
    test_ppu_scrolling
    test_ppu_timing
    test_ppu_nmi
    test_ppu_vram
    test_ppu_oam
    test_ppu_palette
)

# Configurar cada teste da PPU
foreach(TEST_NAME ${PPU_TESTS})
    add_executable(${TEST_NAME}
        ${TEST_NAME}.c
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        Unity::Unity
        mega_emu_nes
    )

    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/platforms/nes
        ${CMAKE_SOURCE_DIR}/src/platforms/nes/ppu
    )

    add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_tests_properties(${TEST_NAME} PROPERTIES
        LABELS "unit;nes;ppu"
    )
endforeach()

# Configurar testes de integração da PPU
add_executable(test_ppu_integration
    test_ppu_integration.c
)

target_link_libraries(test_ppu_integration PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_ppu_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/ppu
)

add_test(NAME test_ppu_integration
    COMMAND test_ppu_integration
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_ppu_integration PROPERTIES
    LABELS "integration;nes;ppu"
)

# Configurar testes de compatibilidade da PPU
add_executable(test_ppu_compatibility
    test_ppu_compatibility.c
)

target_link_libraries(test_ppu_compatibility PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_ppu_compatibility PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/ppu
)

add_test(NAME test_ppu_compatibility
    COMMAND test_ppu_compatibility
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_ppu_compatibility PROPERTIES
    LABELS "compatibility;nes;ppu"
)

# Configurar testes de performance da PPU
add_executable(test_ppu_performance
    test_ppu_performance.c
)

target_link_libraries(test_ppu_performance PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_ppu_performance PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/ppu
)

add_test(NAME test_ppu_performance
    COMMAND test_ppu_performance
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_ppu_performance PROPERTIES
    LABELS "performance;nes;ppu"
)

# Configurar testes de regressão da PPU
add_executable(test_ppu_regression
    test_ppu_regression.c
)

target_link_libraries(test_ppu_regression PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_ppu_regression PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/ppu
)

add_test(NAME test_ppu_regression
    COMMAND test_ppu_regression
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_ppu_regression PROPERTIES
    LABELS "regression;nes;ppu"
)

# Configurar testes de benchmark da PPU
add_executable(test_ppu_benchmark
    test_ppu_benchmark.c
)

target_link_libraries(test_ppu_benchmark PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_ppu_benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/ppu
)

add_test(NAME test_ppu_benchmark
    COMMAND test_ppu_benchmark
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_ppu_benchmark PROPERTIES
    LABELS "benchmark;nes;ppu"
)

# Configurar testes visuais da PPU
add_executable(test_ppu_visual
    test_ppu_visual.c
)

target_link_libraries(test_ppu_visual PRIVATE
    Unity::Unity
    mega_emu_nes
    SDL2::SDL2
)

target_include_directories(test_ppu_visual PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/ppu
)

add_test(NAME test_ppu_visual
    COMMAND test_ppu_visual
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_ppu_visual PROPERTIES
    LABELS "visual;nes;ppu"
)

message(STATUS "Configuração de testes da PPU do NES concluída")
