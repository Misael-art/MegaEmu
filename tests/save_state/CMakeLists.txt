###############################################################################
# Testes de Save State
###############################################################################

enable_testing()

# Adicionar executável para o teste minimalista
add_executable(test_save_state_minimal
    test_save_state_minimal.c
)

# Adicionar definição para suprimir warnings de funções inseguras
target_compile_definitions(test_save_state_minimal PRIVATE _CRT_SECURE_NO_WARNINGS)

# Incluir diretórios necessários
target_include_directories(test_save_state_minimal PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Adicionar o teste ao CTest
add_test(NAME test_save_state_minimal COMMAND test_save_state_minimal)

# Adicionar executável para o teste standalone
add_executable(test_save_state_standalone
    test_save_state_standalone.c
)

# Adicionar definição para suprimir warnings de funções inseguras
target_compile_definitions(test_save_state_standalone PRIVATE _CRT_SECURE_NO_WARNINGS)

# Incluir diretórios necessários
target_include_directories(test_save_state_standalone PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Adicionar o teste ao CTest
add_test(NAME test_save_state_standalone COMMAND test_save_state_standalone)
