# Source directory CMakeLists.txt

# Verificar dependências SDL2
if(USE_SDL2)
    if(NOT SDL2_FOUND)
        message(FATAL_ERROR "SDL2 não encontrado. Verifique se vcpkg está configurado corretamente.")
    else()
        message(STATUS "SDL2 encontrado e configurado corretamente")
    endif()

    # Configurar opções de SDL2_ttf
    if(SDL2_ttf_FOUND)
        add_definitions(-DUSE_SDL2_TTF)
        message(STATUS "SDL2_ttf encontrado - recursos de texto avançados habilitados")
    else()
        message(STATUS "SDL2_ttf não encontrado - recursos de texto avançados desabilitados")
    endif()
endif()

# Configuração de compilação para os componentes do emulador
message(STATUS "Configurando componentes do Mega_Emu...")

# Adicionar subdiretórios
add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(frontend)
add_subdirectory(platforms)

# Definir fontes do projeto principal
set(SOURCES
    main.cpp)

# Criar executável principal
add_executable(mega_emu ${SOURCES})

# Incluir diretórios
target_include_directories(mega_emu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms
    ${CMAKE_CURRENT_SOURCE_DIR}/utils)

# Configurar C++17
set_target_properties(mega_emu PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

# Configurar bibliotecas base
set(MEGA_EMU_LIBS
    mega_emu_core
    mega_emu_utils
    mega_emu_platforms
    mega_emu_frontend)

# Adicionar SDL2 se disponível
if(USE_SDL2 AND SDL2_FOUND)
    list(APPEND MEGA_EMU_LIBS SDL2::SDL2)
    if(SDL2_ttf_FOUND)
        list(APPEND MEGA_EMU_LIBS SDL2_ttf::SDL2_ttf)
    endif()
endif()

# Linkar bibliotecas
target_link_libraries(mega_emu PRIVATE ${MEGA_EMU_LIBS})

# Definir flags de compilação
target_compile_definitions(mega_emu PRIVATE
    $<$<BOOL:${USE_SDL2}>:USE_SDL2>
    $<$<BOOL:${SDL2_ttf_FOUND}>:USE_SDL2_TTF>
    SDL_MAIN_HANDLED)

# Copiar DLLs do SDL2 para Windows
if(WIN32 AND USE_SDL2 AND SDL2_FOUND)
    add_custom_command(TARGET mega_emu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE_DIR:mega_emu>)

    if(SDL2_ttf_FOUND)
        add_custom_command(TARGET mega_emu POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2_ttf::SDL2_ttf>
            $<TARGET_FILE_DIR:mega_emu>)
    endif()
endif()

# Configurar diretório de saída
set_target_properties(mega_emu PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Lista de bibliotecas do projeto
set(MEGA_EMU_LIBRARIES
    ${MEGA_EMU_LIBS}
    PARENT_SCOPE)
