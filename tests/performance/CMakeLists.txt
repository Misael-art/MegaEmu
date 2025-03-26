# Testes de performance

# CPU
add_executable(cpu_performance_tests
    cpu/cpu_benchmark_tests.cpp
    cpu/memory_access_tests.cpp
    cpu/instruction_timing_tests.cpp
)
target_link_libraries(cpu_performance_tests ${TEST_COMMON_LIBS})
add_test(NAME cpu_performance_tests COMMAND cpu_performance_tests)

# Renderização
add_executable(rendering_performance_tests
    rendering/frame_timing_tests.cpp
    rendering/sprite_rendering_tests.cpp
    rendering/shader_performance_tests.cpp
)
target_link_libraries(rendering_performance_tests ${TEST_COMMON_LIBS})
add_test(NAME rendering_performance_tests COMMAND rendering_performance_tests)

# Áudio
add_executable(audio_performance_tests
    audio/audio_processing_tests.cpp
    audio/mixing_performance_tests.cpp
    audio/latency_tests.cpp
)
target_link_libraries(audio_performance_tests ${TEST_COMMON_LIBS})
add_test(NAME audio_performance_tests COMMAND audio_performance_tests)

# Sistema
add_executable(system_performance_tests
    system/full_load_tests.cpp
    system/memory_usage_tests.cpp
    system/threading_tests.cpp
)
target_link_libraries(system_performance_tests ${TEST_COMMON_LIBS})
add_test(NAME system_performance_tests COMMAND system_performance_tests)
