# Adicionar testes dos mappers
add_executable(test_mapper0 mappers/test_mapper0.c)
add_executable(test_mapper1 mappers/test_mapper1.c)
add_executable(test_mapper71 mappers/test_mapper71.c)

# Configurar cada teste
target_link_libraries(test_mapper0 unity nes_cartridge)
target_link_libraries(test_mapper1 unity nes_cartridge)
target_link_libraries(test_mapper71 unity nes_cartridge)

# Adicionar testes ao CTest
add_test(NAME test_mapper0 COMMAND test_mapper0)
add_test(NAME test_mapper1 COMMAND test_mapper1)
add_test(NAME test_mapper71 COMMAND test_mapper71)

# Testes para o subsistema de cartuchos do NES

# Testes individuais dos mappers
add_executable(test_mapper0 test_mapper0.c)
target_include_directories(test_mapper0 PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${UNITY_INCLUDE_DIR}
)
target_link_libraries(test_mapper0
    nes_cartridge
    unity
)

add_executable(test_mapper1 test_mapper1.c)
target_include_directories(test_mapper1 PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${UNITY_INCLUDE_DIR}
)
target_link_libraries(test_mapper1
    nes_cartridge
    unity
)

add_executable(test_mapper2 test_mapper2.c)
target_include_directories(test_mapper2 PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${UNITY_INCLUDE_DIR}
)
target_link_libraries(test_mapper2
    nes_cartridge
    unity
)

add_executable(test_mapper3 test_mapper3.c)
target_include_directories(test_mapper3 PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${UNITY_INCLUDE_DIR}
)
target_link_libraries(test_mapper3
    nes_cartridge
    unity
)

add_executable(test_mapper4 test_mapper4.c)
target_include_directories(test_mapper4 PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${UNITY_INCLUDE_DIR}
)
target_link_libraries(test_mapper4
    nes_cartridge
    unity
)

add_executable(test_mapper5 test_mapper5.c)
target_include_directories(test_mapper5 PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${UNITY_INCLUDE_DIR}
)
target_link_libraries(test_mapper5
    nes_cartridge
    unity
)

add_executable(test_mapper71 test_mapper71.c)
target_include_directories(test_mapper71 PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${UNITY_INCLUDE_DIR}
)
target_link_libraries(test_mapper71
    nes_cartridge
    unity
)

# Adiciona os testes ao CTest
add_test(NAME test_mapper0 COMMAND test_mapper0)
add_test(NAME test_mapper1 COMMAND test_mapper1)
add_test(NAME test_mapper2 COMMAND test_mapper2)
add_test(NAME test_mapper3 COMMAND test_mapper3)
add_test(NAME test_mapper4 COMMAND test_mapper4)
add_test(NAME test_mapper5 COMMAND test_mapper5)
add_test(NAME test_mapper71 COMMAND test_mapper71)

# Teste geral do subsistema de cartuchos
add_executable(test_nes_cartridge
    test_nes_cartridge.c
)

target_include_directories(test_nes_cartridge PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${UNITY_INCLUDE_DIR}
)

target_link_libraries(test_nes_cartridge
    nes_cartridge
    unity
)

add_test(NAME test_nes_cartridge COMMAND test_nes_cartridge)

set(NES_CARTRIDGE_TEST_SOURCES
    test_nes_cartridge.c
    test_mmc5.c
    test_mmc3.c
    mappers/test_mapper0.c
    mappers/test_mapper1.c
    mappers/test_mapper2.c
    mappers/test_mapper85.c
)
