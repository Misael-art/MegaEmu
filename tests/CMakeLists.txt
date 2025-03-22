cmake_minimum_required(VERSION 3.10)

# Adiciona o framework de testes Unity
add_library(unity STATIC
    unity/unity.c
    unity/unity.h
    unity/unity_internals.h
)

# Configura os testes do M68K
add_executable(m68k_test
    cpu/m68k_test.c
)

target_link_libraries(m68k_test
    PRIVATE
        unity
        m68k
        memory
)

# Configura os testes do Z80
add_executable(z80_test
    cpu/z80_test.c
)

target_link_libraries(z80_test
    PRIVATE
        unity
        z80
        memory
)

# Configura os testes do Mega Drive
add_executable(mega_drive_test
    mega_drive/mega_drive_test.c
)

target_link_libraries(mega_drive_test
    PRIVATE
        unity
        mega_drive
        m68k
        z80
        memory
)

# Configura os testes do Master System
add_executable(master_system_test
    master_system/master_system_test.c
)

target_link_libraries(master_system_test
    PRIVATE
        unity
        master_system
        z80
        memory
)

# Adiciona os testes como testes do CTest
add_test(NAME m68k_test COMMAND m68k_test)
add_test(NAME z80_test COMMAND z80_test)
add_test(NAME mega_drive_test COMMAND mega_drive_test)
add_test(NAME master_system_test COMMAND master_system_test)

# Adiciona testes de CPU
add_subdirectory(cpu)

# Adiciona testes de memória
add_subdirectory(memory)

# Adiciona testes de vídeo
add_subdirectory(video)

# Adiciona testes de áudio
add_subdirectory(audio)

# Adiciona testes de save state
add_subdirectory(save_state)
