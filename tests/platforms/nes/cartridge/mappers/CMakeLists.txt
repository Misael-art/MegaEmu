# CMakeLists.txt para testes dos mappers do NES

# Configurar testes dos mappers do NES
message(STATUS "Configurando testes dos mappers do NES...")

# Lista de mappers para testar
set(MAPPER_TESTS
    test_mapper0
    test_mapper1
    test_mapper2
    test_mapper3
    test_mapper4
    test_mapper5
    test_mapper6
    test_mapper7
    test_mapper8
    test_mapper9
    test_mapper10
    test_mapper71
    test_mapper85
)

# Configurar cada teste de mapper
foreach(TEST_NAME ${MAPPER_TESTS})
    add_executable(${TEST_NAME}
        ${TEST_NAME}.c
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        Unity::Unity
        mega_emu_nes
    )

    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/platforms/nes
        ${CMAKE_SOURCE_DIR}/src/platforms/nes/cartridge
        ${CMAKE_SOURCE_DIR}/src/platforms/nes/cartridge/mappers
    )

    add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_tests_properties(${TEST_NAME} PROPERTIES
        LABELS "unit;nes;mapper"
    )
endforeach()

# Configurar testes de integração dos mappers
add_executable(test_mappers_integration
    test_mappers_integration.c
)

target_link_libraries(test_mappers_integration PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_mappers_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/cartridge
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/cartridge/mappers
)

add_test(NAME test_mappers_integration
    COMMAND test_mappers_integration
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_mappers_integration PROPERTIES
    LABELS "integration;nes;mapper"
)

# Configurar testes de compatibilidade dos mappers
add_executable(test_mappers_compatibility
    test_mappers_compatibility.c
)

target_link_libraries(test_mappers_compatibility PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_mappers_compatibility PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/cartridge
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/cartridge/mappers
)

add_test(NAME test_mappers_compatibility
    COMMAND test_mappers_compatibility
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_mappers_compatibility PROPERTIES
    LABELS "compatibility;nes;mapper"
)

# Configurar testes de performance dos mappers
add_executable(test_mappers_performance
    test_mappers_performance.c
)

target_link_libraries(test_mappers_performance PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_mappers_performance PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/cartridge
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/cartridge/mappers
)

add_test(NAME test_mappers_performance
    COMMAND test_mappers_performance
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_mappers_performance PROPERTIES
    LABELS "performance;nes;mapper"
)

# Configurar testes de regressão dos mappers
add_executable(test_mappers_regression
    test_mappers_regression.c
)

target_link_libraries(test_mappers_regression PRIVATE
    Unity::Unity
    mega_emu_nes
)

target_include_directories(test_mappers_regression PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/cartridge
    ${CMAKE_SOURCE_DIR}/src/platforms/nes/cartridge/mappers
)

add_test(NAME test_mappers_regression
    COMMAND test_mappers_regression
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(test_mappers_regression PROPERTIES
    LABELS "regression;nes;mapper"
)

message(STATUS "Configuração de testes dos mappers do NES concluída")
