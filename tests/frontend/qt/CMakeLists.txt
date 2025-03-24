# Configurar testes do frontend Qt
message(STATUS "Configurando testes do frontend Qt...")

# Encontrar Qt
find_package(Qt5 COMPONENTS Core Gui Widgets Test REQUIRED)

# Lista de testes do frontend Qt
set(QT_TESTS
    test_qt_window
    test_qt_widgets
    test_qt_input
    test_qt_audio
    test_qt_rendering
    test_qt_events
    test_qt_timing
    test_qt_gui
    test_qt_menu
    test_qt_config
)

# Configurar cada teste do frontend Qt
foreach(TEST_NAME ${QT_TESTS})
    # Adicionar arquivos fonte
    add_executable(${TEST_NAME}
        ${TEST_NAME}.cpp
    )

    # Configurar MOC
    set_target_properties(${TEST_NAME} PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        Unity::Unity
        mega_emu_frontend_qt
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Test
    )

    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/frontend/qt
    )

    add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_tests_properties(${TEST_NAME} PROPERTIES
        LABELS "unit;frontend;qt"
    )
endforeach()

# Configurar testes de integração do frontend Qt
add_executable(test_qt_integration
    test_qt_integration.cpp
)

set_target_properties(test_qt_integration PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_link_libraries(test_qt_integration PRIVATE
    Unity::Unity
    mega_emu_frontend_qt
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Test
)

target_include_directories(test_qt_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/qt
)

add_test(NAME test_qt_integration
    COMMAND test_qt_integration
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_qt_integration PROPERTIES
    LABELS "integration;frontend;qt"
)

# Configurar testes de compatibilidade do frontend Qt
add_executable(test_qt_compatibility
    test_qt_compatibility.cpp
)

set_target_properties(test_qt_compatibility PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_link_libraries(test_qt_compatibility PRIVATE
    Unity::Unity
    mega_emu_frontend_qt
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Test
)

target_include_directories(test_qt_compatibility PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/qt
)

add_test(NAME test_qt_compatibility
    COMMAND test_qt_compatibility
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_qt_compatibility PROPERTIES
    LABELS "compatibility;frontend;qt"
)

# Configurar testes de performance do frontend Qt
add_executable(test_qt_performance
    test_qt_performance.cpp
)

set_target_properties(test_qt_performance PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_link_libraries(test_qt_performance PRIVATE
    Unity::Unity
    mega_emu_frontend_qt
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Test
)

target_include_directories(test_qt_performance PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/qt
)

add_test(NAME test_qt_performance
    COMMAND test_qt_performance
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_qt_performance PROPERTIES
    LABELS "performance;frontend;qt"
)

# Configurar testes de regressão do frontend Qt
add_executable(test_qt_regression
    test_qt_regression.cpp
)

set_target_properties(test_qt_regression PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_link_libraries(test_qt_regression PRIVATE
    Unity::Unity
    mega_emu_frontend_qt
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Test
)

target_include_directories(test_qt_regression PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/qt
)

add_test(NAME test_qt_regression
    COMMAND test_qt_regression
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_qt_regression PROPERTIES
    LABELS "regression;frontend;qt"
)

# Configurar testes de benchmark do frontend Qt
add_executable(test_qt_benchmark
    test_qt_benchmark.cpp
)

set_target_properties(test_qt_benchmark PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_link_libraries(test_qt_benchmark PRIVATE
    Unity::Unity
    mega_emu_frontend_qt
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Test
)

target_include_directories(test_qt_benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/qt
)

add_test(NAME test_qt_benchmark
    COMMAND test_qt_benchmark
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_qt_benchmark PROPERTIES
    LABELS "benchmark;frontend;qt"
)

# Configurar testes visuais do frontend Qt
add_executable(test_qt_visual
    test_qt_visual.cpp
)

set_target_properties(test_qt_visual PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_link_libraries(test_qt_visual PRIVATE
    Unity::Unity
    mega_emu_frontend_qt
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Test
)

target_include_directories(test_qt_visual PRIVATE
    ${CMAKE_SOURCE_DIR}/src/frontend/qt
)

add_test(NAME test_qt_visual
    COMMAND test_qt_visual
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_qt_visual PROPERTIES
    LABELS "visual;frontend;qt"
)

message(STATUS "Configuração de testes do frontend Qt concluída")
