# Banco de Dados de ROMs

set(ROM_DB_SOURCES
    rom_db.c
    rom_db_hash.c
    rom_db_io.c
    rom_db_entry.c
    rom_db_utils.c
)

set(ROM_DB_HEADERS
    rom_db.h
    rom_db_types.h
)

# Adicionar as fontes ao projeto principal
target_sources(mega_emu PRIVATE ${ROM_DB_SOURCES} ${ROM_DB_HEADERS})

# Configuração de compilação
target_include_directories(mega_emu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Dependências externas
find_package(SQLite3 REQUIRED)
target_include_directories(mega_emu PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(mega_emu PRIVATE ${SQLite3_LIBRARIES})

# Configuração específica para testes
if(MEGA_EMU_BUILD_TESTS)
    add_executable(test_rom_db tests/test_rom_db.c ${ROM_DB_SOURCES})
    target_include_directories(test_rom_db PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${SQLite3_INCLUDE_DIRS}
    )
    target_link_libraries(test_rom_db PRIVATE ${SQLite3_LIBRARIES})
    add_test(NAME test_rom_db COMMAND test_rom_db)
endif()
