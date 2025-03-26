# Examples
cmake_minimum_required(VERSION 3.10)

# NES Test
add_executable(nes_test
    nes_test.c
)

target_link_libraries(nes_test
    cpu_6502
    ppu_2c02
)

target_include_directories(nes_test PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
