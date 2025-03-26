# Testes de integração

# Sistema completo
add_executable(system_integration_tests
    system/full_system_tests.cpp
    system/platform_integration_tests.cpp
    system/frontend_integration_tests.cpp
)
target_link_libraries(system_integration_tests ${TEST_COMMON_LIBS})
add_test(NAME system_integration_tests COMMAND system_integration_tests)

# Emulação
add_executable(emulation_integration_tests
    emulation/rom_loading_tests.cpp
    emulation/save_state_tests.cpp
    emulation/audio_video_sync_tests.cpp
)
target_link_libraries(emulation_integration_tests ${TEST_COMMON_LIBS})
add_test(NAME emulation_integration_tests COMMAND emulation_integration_tests)

# Frontend
add_executable(frontend_integration_tests
    frontend/input_handling_tests.cpp
    frontend/rendering_pipeline_tests.cpp
    frontend/audio_pipeline_tests.cpp
)
target_link_libraries(frontend_integration_tests ${TEST_COMMON_LIBS})
add_test(NAME frontend_integration_tests COMMAND frontend_integration_tests)

# Plugins
add_executable(plugin_integration_tests
    plugins/plugin_loading_tests.cpp
    plugins/plugin_communication_tests.cpp
    plugins/plugin_lifecycle_tests.cpp
)
target_link_libraries(plugin_integration_tests ${TEST_COMMON_LIBS})
add_test(NAME plugin_integration_tests COMMAND plugin_integration_tests)
