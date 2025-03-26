# Testes unitários

# CPU
add_executable(cpu_tests
    cpu/cpu_tests.cpp
    cpu/memory_tests.cpp
    cpu/interrupts_tests.cpp
)
target_link_libraries(cpu_tests ${TEST_COMMON_LIBS})
add_test(NAME cpu_tests COMMAND cpu_tests)

# PPU
add_executable(ppu_tests
    video/ppu_tests.cpp
    video/sprites_tests.cpp
    video/rendering_tests.cpp
)
target_link_libraries(ppu_tests ${TEST_COMMON_LIBS})
add_test(NAME ppu_tests COMMAND ppu_tests)

# Plataformas
add_executable(platform_tests
    platforms/megadrive_tests.cpp
    platforms/nes_tests.cpp
    platforms/sms_tests.cpp
    platforms/gg_tests.cpp
)
target_link_libraries(platform_tests ${TEST_COMMON_LIBS})
add_test(NAME platform_tests COMMAND platform_tests)

# Frontend
add_executable(frontend_tests
    frontend/sdl_tests.cpp
    frontend/qt_tests.cpp
    frontend/react_tests.cpp
)
target_link_libraries(frontend_tests ${TEST_COMMON_LIBS})
add_test(NAME frontend_tests COMMAND frontend_tests)
