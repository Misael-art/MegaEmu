# CMakeLists.txt para o componente de cartuchos do NES

# Definir arquivos fonte C
set(NES_CARTRIDGE_SOURCES
    nes_cartridge.c
    mappers/mapper0.c
    mappers/mapper1.c
    mappers/mapper2.c
    mappers/mapper3.c
    mappers/mapper4.c
    mappers/mapper5.c
    mappers/mapper6.c
    mappers/mapper7.c
    mappers/mapper8.c
    mappers/mapper9.c
    mappers/mapper71.c
    mappers/mapper90.c
    mappers/mapper230.c
    mappers/mapper255.c
)

# Definir arquivos de cabeçalho
set(NES_CARTRIDGE_HEADERS
    nes_cartridge.h
)

# Criar biblioteca do cartridge
add_library(nes_cartridge STATIC
    ${NES_CARTRIDGE_SOURCES}
    ${NES_CARTRIDGE_HEADERS}
)

# Definir includes
target_include_directories(nes_cartridge PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/platforms/nes
)

# Linkar com outras bibliotecas
target_link_libraries(nes_cartridge
    nes_ppu
    utils
)

# Exportar biblioteca
set(NES_CARTRIDGE_LIBRARY nes_cartridge PARENT_SCOPE)
set(NES_CARTRIDGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
