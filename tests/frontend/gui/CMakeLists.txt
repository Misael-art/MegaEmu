# Configurar testes da GUI
message(STATUS "Configurando testes da GUI...")

# Lista de testes da GUI
set(GUI_TESTS
    test_gui_manager
    test_gui_element
    test_gui_button
    test_gui_label
    test_gui_textbox
    test_gui_checkbox
    test_gui_combobox
    test_gui_listbox
    test_gui_scrollbar
    test_gui_menu
)

# Configurar cada teste da GUI
foreach(TEST_NAME ${GUI_TESTS})
    add_executable(${TEST_NAME}
        ${TEST_NAME}.c
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        Unity::Unity
        mega_emu_gui
    )

    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/frontend/gui
    )

    add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_tests_properties(${TEST_NAME} PROPERTIES
        LABELS "unit;frontend;gui"
    )
endforeach()

# Configurar testes de integração da GUI
add_executable(test_gui_integration
    test_gui_integration.c
)

target_link_libraries(test_gui_integration PRIVATE
    Unity::Unity
    mega_emu_gui
)

target_include_directories(test_gui_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/gui
)

add_test(NAME test_gui_integration
    COMMAND test_gui_integration
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_gui_integration PROPERTIES
    LABELS "integration;frontend;gui"
)

# Configurar testes de compatibilidade da GUI
add_executable(test_gui_compatibility
    test_gui_compatibility.c
)

target_link_libraries(test_gui_compatibility PRIVATE
    Unity::Unity
    mega_emu_gui
)

target_include_directories(test_gui_compatibility PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/gui
)

add_test(NAME test_gui_compatibility
    COMMAND test_gui_compatibility
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_gui_compatibility PROPERTIES
    LABELS "compatibility;frontend;gui"
)

# Configurar testes de performance da GUI
add_executable(test_gui_performance
    test_gui_performance.c
)

target_link_libraries(test_gui_performance PRIVATE
    Unity::Unity
    mega_emu_gui
)

target_include_directories(test_gui_performance PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/gui
)

add_test(NAME test_gui_performance
    COMMAND test_gui_performance
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_gui_performance PROPERTIES
    LABELS "performance;frontend;gui"
)

# Configurar testes de regressão da GUI
add_executable(test_gui_regression
    test_gui_regression.c
)

target_link_libraries(test_gui_regression PRIVATE
    Unity::Unity
    mega_emu_gui
)

target_include_directories(test_gui_regression PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/gui
)

add_test(NAME test_gui_regression
    COMMAND test_gui_regression
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_gui_regression PROPERTIES
    LABELS "regression;frontend;gui"
)

# Configurar testes de benchmark da GUI
add_executable(test_gui_benchmark
    test_gui_benchmark.c
)

target_link_libraries(test_gui_benchmark PRIVATE
    Unity::Unity
    mega_emu_gui
)

target_include_directories(test_gui_benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/gui
)

add_test(NAME test_gui_benchmark
    COMMAND test_gui_benchmark
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_gui_benchmark PROPERTIES
    LABELS "benchmark;frontend;gui"
)

# Configurar testes visuais da GUI
add_executable(test_gui_visual
    test_gui_visual.c
)

target_link_libraries(test_gui_visual PRIVATE
    Unity::Unity
    mega_emu_gui
)

target_include_directories(test_gui_visual PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/gui
)

add_test(NAME test_gui_visual
    COMMAND test_gui_visual
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_gui_visual PROPERTIES
    LABELS "visual;frontend;gui"
)

message(STATUS "Configuração de testes da GUI concluída")
