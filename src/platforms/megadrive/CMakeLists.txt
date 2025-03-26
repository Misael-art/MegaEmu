cmake_minimum_required(VERSION 3.15)

add_library(mega_emu_megadrive_platform
    megadrive.c
)

target_include_directories(mega_emu_megadrive_platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mega_emu_megadrive_platform PRIVATE
    mega_emu_cpu
    mega_emu_ppu
)

set_target_properties(mega_emu_megadrive_platform PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON)

if(MSVC)
    target_compile_options(mega_emu_megadrive_platform PRIVATE /W4)
else()
    target_compile_options(mega_emu_megadrive_platform PRIVATE -Wall -Wextra -Wpedantic)
endif()
