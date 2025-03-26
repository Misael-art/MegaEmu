cmake_minimum_required(VERSION 3.15)

add_library(mega_tools STATIC
    src/tools.cpp
)

target_include_directories(mega_tools PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(mega_tools PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(MSVC)
    target_compile_options(mega_tools PRIVATE /W4)
else()
    target_compile_options(mega_tools PRIVATE -Wall -Wextra -Wpedantic)
endif()
