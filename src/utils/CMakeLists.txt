# CMakeLists.txt para o diretório utils

# Utils library
set(UTILS_SOURCES
    enhanced_log.c
    error_handling.c
    file_utils.c
    log_utils.c
    md5.c
    save_state.c
)

add_library(mega_emu_utils STATIC ${UTILS_SOURCES})

target_include_directories(mega_emu_utils PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(mega_emu_utils
    ${SDL2_LIBRARIES}
)
