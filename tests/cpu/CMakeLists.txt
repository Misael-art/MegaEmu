# Testes de CPU

# Teste do Z80
add_executable(z80_test z80_test.c)
target_link_libraries(z80_test
    z80_core_lib
    utils
)

# Teste do M68K
if(BUILD_MEGADRIVE OR BUILD_NEOGEO OR BUILD_ATARIJAGUAR)
    add_executable(m68k_test m68k_test.c)
    target_link_libraries(m68k_test
        mega_emu_cpu
        mega_emu_memory
        mega_emu_utils
        unity
    )

    # Adicionar ao CTest
    add_test(NAME m68k_test COMMAND m68k_test)
endif()

# Adicionar testes ao CTest
add_test(NAME z80_test COMMAND z80_test)
