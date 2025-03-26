cmake_minimum_required(VERSION 3.10)

# Adiciona o framework de testes Unity
add_library(unity STATIC
    unity/unity.c
)

target_include_directories(unity PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/unity
)

# Configuração de testes para o Mega_Emu
message(STATUS "Configurando testes do Mega_Emu...")

# Configurar testes
enable_testing()

# Encontrar Unity
find_package(Unity REQUIRED)

# Função para adicionar teste
function(add_unit_test test_name)
    add_executable(${test_name} ${test_name}.c)
    target_link_libraries(${test_name} PRIVATE Unity::Unity)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Adicionar subdiretórios de testes
add_subdirectory(platforms/nes)
add_subdirectory(platforms/megadrive)
add_subdirectory(platforms/mastersystem)
add_subdirectory(frontend/gui)
add_subdirectory(frontend/sdl)
if(BUILD_FRONTEND_QT)
    add_subdirectory(frontend/qt)
endif()
add_subdirectory(tools)

# Configurar cobertura de código
if(CMAKE_COMPILER_IS_GNUCXX)
    include(CodeCoverage)
    append_coverage_compiler_flags()
    setup_target_for_coverage_lcov(
        NAME coverage
        EXECUTABLE ctest
        DEPENDENCIES ${TEST_TARGETS}
    )
endif()

# Configurar relatório de testes
add_custom_target(test_report
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes e gerando relatório"
)

# Configurar testes de memória com Valgrind
find_program(VALGRIND "valgrind")
if(VALGRIND)
    add_custom_target(memcheck
        COMMAND ${VALGRIND} --leak-check=full --error-exitcode=1 ${CMAKE_CTEST_COMMAND}
        DEPENDS ${TEST_TARGETS}
        COMMENT "Executando testes com Valgrind"
    )
endif()

# Configurar testes de sanitizer
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_custom_target(sanitize
        COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug
                                -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined"
                                -DCMAKE_C_FLAGS="-fsanitize=address,undefined"
                                ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} --build .
        COMMAND ${CMAKE_CTEST_COMMAND}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Executando testes com sanitizers"
    )
endif()

# Configurar testes de fuzz
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_custom_target(fuzz
        COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug
                                -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer,address"
                                -DCMAKE_C_FLAGS="-fsanitize=fuzzer,address"
                                ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} --build .
        COMMAND ${CMAKE_CTEST_COMMAND}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Executando testes de fuzzing"
    )
endif()

# Configurar testes de benchmark
add_custom_target(benchmark
    COMMAND ${CMAKE_CTEST_COMMAND} -L benchmark
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de benchmark"
)

# Configurar testes de integração
add_custom_target(integration
    COMMAND ${CMAKE_CTEST_COMMAND} -L integration
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de integração"
)

# Configurar testes de sistema
add_custom_target(system
    COMMAND ${CMAKE_CTEST_COMMAND} -L system
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de sistema"
)

# Configurar testes de regressão
add_custom_target(regression
    COMMAND ${CMAKE_CTEST_COMMAND} -L regression
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de regressão"
)

# Configurar testes de performance
add_custom_target(performance
    COMMAND ${CMAKE_CTEST_COMMAND} -L performance
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de performance"
)

# Configurar testes de stress
add_custom_target(stress
    COMMAND ${CMAKE_CTEST_COMMAND} -L stress
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de stress"
)

# Configurar testes de carga
add_custom_target(load
    COMMAND ${CMAKE_CTEST_COMMAND} -L load
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de carga"
)

# Configurar testes de compatibilidade
add_custom_target(compatibility
    COMMAND ${CMAKE_CTEST_COMMAND} -L compatibility
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de compatibilidade"
)

# Configurar testes de segurança
add_custom_target(security
    COMMAND ${CMAKE_CTEST_COMMAND} -L security
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de segurança"
)

# Configurar testes de usabilidade
add_custom_target(usability
    COMMAND ${CMAKE_CTEST_COMMAND} -L usability
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de usabilidade"
)

# Configurar testes de acessibilidade
add_custom_target(accessibility
    COMMAND ${CMAKE_CTEST_COMMAND} -L accessibility
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de acessibilidade"
)

# Configurar testes de localização
add_custom_target(localization
    COMMAND ${CMAKE_CTEST_COMMAND} -L localization
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de localização"
)

# Configurar testes de instalação
add_custom_target(installation
    COMMAND ${CMAKE_CTEST_COMMAND} -L installation
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de instalação"
)

# Configurar testes de documentação
add_custom_target(documentation
    COMMAND ${CMAKE_CTEST_COMMAND} -L documentation
    DEPENDS ${TEST_TARGETS}
    COMMENT "Executando testes de documentação"
)

# Adicionar arquivos fonte dos testes
file(GLOB_RECURSE TEST_SOURCES
    "*.c"
    "*.cpp"
)

# Criar executável de testes
add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})

# Vincular bibliotecas
target_link_libraries(${PROJECT_NAME}_tests
    ${SDL2_LIBRARIES}
    ${LUA_LIBRARIES}
    ${Boost_LIBRARIES}
)

# Adicionar testes
add_test(NAME cpu_tests COMMAND ${PROJECT_NAME}_tests --gtest_filter=CpuTest.*)
add_test(NAME video_tests COMMAND ${PROJECT_NAME}_tests --gtest_filter=VideoTest.*)
add_test(NAME audio_tests COMMAND ${PROJECT_NAME}_tests --gtest_filter=AudioTest.*)
add_test(NAME memory_tests COMMAND ${PROJECT_NAME}_tests --gtest_filter=MemoryTest.*)

message(STATUS "Configuração de testes concluída")
message(STATUS "Execute 'ctest' para rodar todos os testes")

# Configuração do Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Habilitar testes
enable_testing()

# Adicionar diretórios de testes
add_subdirectory(unit)
add_subdirectory(integration)
add_subdirectory(performance)

# Configurações comuns para todos os testes
set(TEST_COMMON_LIBS
    gtest
    gtest_main
    mega_emu_cpu
    mega_emu_ppu
    mega_emu_frontend_sdl
    mega_emu_megadrive_platform
)

# Configurações de compilação para testes
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Incluir diretórios
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/frontend
    ${CMAKE_SOURCE_DIR}/src/platforms
)
