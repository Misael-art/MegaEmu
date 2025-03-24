# Configurar testes do frontend SDL
message(STATUS "Configurando testes do frontend SDL...")

# Lista de testes do frontend SDL
set(SDL_TESTS
    test_sdl_window
    test_sdl_renderer
    test_sdl_input
    test_sdl_audio
    test_sdl_texture
    test_sdl_events
    test_sdl_timing
    test_sdl_gui
    test_sdl_menu
    test_sdl_config
)

# Configurar cada teste do frontend SDL
foreach(TEST_NAME ${SDL_TESTS})
    add_executable(${TEST_NAME}
        ${TEST_NAME}.c
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        Unity::Unity
        mega_emu_frontend_sdl
        SDL2::SDL2
    )

    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/frontend/sdl
    )

    add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_tests_properties(${TEST_NAME} PROPERTIES
        LABELS "unit;frontend;sdl"
    )
endforeach()

# Configurar testes de integração do frontend SDL
add_executable(test_sdl_integration
    test_sdl_integration.c
)

target_link_libraries(test_sdl_integration PRIVATE
    Unity::Unity
    mega_emu_frontend_sdl
    SDL2::SDL2
)

target_include_directories(test_sdl_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/sdl
)

add_test(NAME test_sdl_integration
    COMMAND test_sdl_integration
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_sdl_integration PROPERTIES
    LABELS "integration;frontend;sdl"
)

# Configurar testes de compatibilidade do frontend SDL
add_executable(test_sdl_compatibility
    test_sdl_compatibility.c
)

target_link_libraries(test_sdl_compatibility PRIVATE
    Unity::Unity
    mega_emu_frontend_sdl
    SDL2::SDL2
)

target_include_directories(test_sdl_compatibility PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/sdl
)

add_test(NAME test_sdl_compatibility
    COMMAND test_sdl_compatibility
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_sdl_compatibility PROPERTIES
    LABELS "compatibility;frontend;sdl"
)

# Configurar testes de performance do frontend SDL
add_executable(test_sdl_performance
    test_sdl_performance.c
)

target_link_libraries(test_sdl_performance PRIVATE
    Unity::Unity
    mega_emu_frontend_sdl
    SDL2::SDL2
)

target_include_directories(test_sdl_performance PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/sdl
)

add_test(NAME test_sdl_performance
    COMMAND test_sdl_performance
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_sdl_performance PROPERTIES
    LABELS "performance;frontend;sdl"
)

# Configurar testes de regressão do frontend SDL
add_executable(test_sdl_regression
    test_sdl_regression.c
)

target_link_libraries(test_sdl_regression PRIVATE
    Unity::Unity
    mega_emu_frontend_sdl
    SDL2::SDL2
)

target_include_directories(test_sdl_regression PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/sdl
)

add_test(NAME test_sdl_regression
    COMMAND test_sdl_regression
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_sdl_regression PROPERTIES
    LABELS "regression;frontend;sdl"
)

# Configurar testes de benchmark do frontend SDL
add_executable(test_sdl_benchmark
    test_sdl_benchmark.c
)

target_link_libraries(test_sdl_benchmark PRIVATE
    Unity::Unity
    mega_emu_frontend_sdl
    SDL2::SDL2
)

target_include_directories(test_sdl_benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/sdl
)

add_test(NAME test_sdl_benchmark
    COMMAND test_sdl_benchmark
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_sdl_benchmark PROPERTIES
    LABELS "benchmark;frontend;sdl"
)

# Configurar testes visuais do frontend SDL
add_executable(test_sdl_visual
    test_sdl_visual.c
)

target_link_libraries(test_sdl_visual PRIVATE
    Unity::Unity
    mega_emu_frontend_sdl
    SDL2::SDL2
)

target_include_directories(test_sdl_visual PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/sdl
)

add_test(NAME test_sdl_visual
    COMMAND test_sdl_visual
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_sdl_visual PROPERTIES
    LABELS "visual;frontend;sdl"
)

message(STATUS "Configuração de testes do frontend SDL concluída")
