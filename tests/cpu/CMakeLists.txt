# Testes de CPU

# Teste do Z80
add_executable(cpu_z80_test z80_test.c)
target_link_libraries(cpu_z80_test
    PRIVATE
    z80_core_lib
    utils
    unity
)

# Teste do M68K
if(BUILD_MEGADRIVE OR BUILD_NEOGEO OR BUILD_ATARIJAGUAR)
    add_executable(cpu_m68k_test m68k_test.c)
    target_link_libraries(cpu_m68k_test
        PRIVATE
        mega_emu_cpu
        mega_emu_memory
        mega_emu_utils
        unity
    )

    # Adicionar ao CTest
    add_test(NAME cpu_m68k_test COMMAND cpu_m68k_test)
endif()

# Adicionar testes ao CTest
add_test(NAME cpu_z80_test COMMAND cpu_z80_test)
