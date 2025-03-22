cmake_minimum_required(VERSION 3.10)
project(NESTest VERSION 1.0.0 LANGUAGES CXX)

# Configurar o padrão C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Adicionar executável
add_executable(nes_test nes_test.cpp)

# Configurar a saída
set_target_properties(nes_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Configurar mensagens
message(STATUS "Configurando teste NES...")
message(STATUS "ROM path: ${CMAKE_SOURCE_DIR}/resources/roms/nes/Super Mario Bros. (World).nes")
