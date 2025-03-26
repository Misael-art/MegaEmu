@echo on
setlocal enabledelayedexpansion

REM ============================================================================
REM Script de Teste Unitário para o Mega_Emu
REM Autor: Equipe Mega_Emu
REM Data: 21/03/2025
REM ============================================================================

echo.
echo ============================================================================
echo                   MEGA_EMU - EXECUÇÃO DE TESTES UNITÁRIOS
echo ============================================================================
echo.

REM Definir diretórios
set "ROOT_DIR=%~dp0..\..\..\"
set "SRC_DIR=%ROOT_DIR%src"
set "TESTS_DIR=%ROOT_DIR%tests"
set "BUILD_DIR=%ROOT_DIR%build\tests"
set "TOOLS_DIR=%ROOT_DIR%tools"

REM Verificar se o diretório de build existe, caso contrário, criar
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Verificar qual compilador está disponível
where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set "COMPILER=msvc"
    echo Usando compilador MSVC
) else (
    where gcc >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        set "COMPILER=gcc"
        echo Usando compilador GCC
    ) else (
        echo ERRO: Nenhum compilador (MSVC ou GCC) encontrado no PATH.
        echo Por favor, execute este script a partir de um prompt de comando com ambiente de compilação configurado.
        echo Para MSVC: Execute a partir do "Developer Command Prompt for VS"
        echo Para GCC: Certifique-se de que MinGW/MSYS2 está instalado e no PATH
        exit /b 1
    )
)

echo.
echo Compilando e executando testes unitários...
echo.

REM ============================================================================
REM Teste do Sistema de Configuração
REM ============================================================================

echo Teste: Sistema de Configuração
echo ----------------------------------------------------------------------

set "TEST_NAME=test_config_system"
set "TEST_SRC=%TESTS_DIR%\core\test_config_system.cpp"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "CONFIG_SRC=%SRC_DIR%\core\config_system.cpp"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /Fe:"%TEST_EXE%" "%TEST_SRC%" %CONFIG_SRC% %COMMON_SRC% /link /LIBPATH:"%ROOT_DIR%\deps\lib" gtest.lib gtest_main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" %CONFIG_SRC% %COMMON_SRC% -I"%SRC_DIR%" -L"%ROOT_DIR%\deps\lib" -lgtest -lgtest_main -lpthread -std=c++11
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do Sistema de Memória
REM ============================================================================

echo Teste: Sistema de Memória
echo ----------------------------------------------------------------------

set "TEST_NAME=test_memory_system"
set "TEST_SRC=%TESTS_DIR%\core\test_memory_system.cpp"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "MEMORY_SRC=%SRC_DIR%\core\memory\memory_system.c"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /Fe:"%TEST_EXE%" "%TEST_SRC%" %MEMORY_SRC% %COMMON_SRC% /link /LIBPATH:"%ROOT_DIR%\deps\lib" gtest.lib gtest_main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" %MEMORY_SRC% %COMMON_SRC% -I"%SRC_DIR%" -L"%ROOT_DIR%\deps\lib" -lgtest -lgtest_main -lpthread -std=c++11
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do Sistema de Eventos
REM ============================================================================

echo Teste: Sistema de Eventos
echo ----------------------------------------------------------------------

set "TEST_NAME=test_event_system"
set "TEST_SRC=%TESTS_DIR%\core\test_event_system.cpp"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "EVENT_SRC=%SRC_DIR%\core\events\event_system.c"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /Fe:"%TEST_EXE%" "%TEST_SRC%" %EVENT_SRC% %COMMON_SRC% /link /LIBPATH:"%ROOT_DIR%\deps\lib" gtest.lib gtest_main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" %EVENT_SRC% %COMMON_SRC% -I"%SRC_DIR%" -L"%ROOT_DIR%\deps\lib" -lgtest -lgtest_main -lpthread -std=c++11
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do Sistema de Áudio do Mega Drive
REM ============================================================================

echo Teste: Sistema de Áudio do Mega Drive
echo ----------------------------------------------------------------------

set "TEST_NAME=test_audio_system"
set "TEST_SRC=%TESTS_DIR%\platforms\megadrive\audio\test_audio_system.cpp"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "AUDIO_SRC=%SRC_DIR%\platforms\megadrive\audio\audio_system.c %SRC_DIR%\platforms\megadrive\audio\ym2612.c %SRC_DIR%\platforms\megadrive\audio\sn76489.c"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /Fe:"%TEST_EXE%" "%TEST_SRC%" %AUDIO_SRC% %COMMON_SRC% /link /LIBPATH:"%ROOT_DIR%\deps\lib" gtest.lib gtest_main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" %AUDIO_SRC% %COMMON_SRC% -I"%SRC_DIR%" -L"%ROOT_DIR%\deps\lib" -lgtest -lgtest_main -lpthread -std=c++11
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do Chip SN76489
REM ============================================================================

echo Teste: Chip SN76489
echo ----------------------------------------------------------------------

set "TEST_NAME=test_sn76489"
set "TEST_SRC=%TESTS_DIR%\platforms\megadrive\audio\test_sn76489.cpp"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "AUDIO_SRC=%SRC_DIR%\platforms\megadrive\audio\sn76489.c"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /Fe:"%TEST_EXE%" "%TEST_SRC%" %AUDIO_SRC% %COMMON_SRC% /link /LIBPATH:"%ROOT_DIR%\deps\lib" gtest.lib gtest_main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" %AUDIO_SRC% %COMMON_SRC% -I"%SRC_DIR%" -L"%ROOT_DIR%\deps\lib" -lgtest -lgtest_main -lpthread -std=c++11
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do Chip YM2612
REM ============================================================================

echo Teste: Chip YM2612
echo ----------------------------------------------------------------------

set "TEST_NAME=test_ym2612"
set "TEST_SRC=%TESTS_DIR%\platforms\megadrive\audio\test_ym2612.cpp"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "AUDIO_SRC=%SRC_DIR%\platforms\megadrive\audio\ym2612.c"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /Fe:"%TEST_EXE%" "%TEST_SRC%" %AUDIO_SRC% %COMMON_SRC% /link /LIBPATH:"%ROOT_DIR%\deps\lib" gtest.lib gtest_main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" %AUDIO_SRC% %COMMON_SRC% -I"%SRC_DIR%" -L"%ROOT_DIR%\deps\lib" -lgtest -lgtest_main -lpthread -std=c++11
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste da Ferramenta de Áudio
REM ============================================================================

echo Teste: Ferramenta de Áudio
echo ----------------------------------------------------------------------

set "TEST_NAME=audio_test"
set "TEST_SRC=%TOOLS_DIR%\audio_test.c"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "AUDIO_SRC=%SRC_DIR%\platforms\megadrive\audio\audio_system.c %SRC_DIR%\platforms\megadrive\audio\ym2612.c %SRC_DIR%\platforms\megadrive\audio\sn76489.c"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /Fe:"%TEST_EXE%" "%TEST_SRC%" %AUDIO_SRC% %COMMON_SRC% /link /LIBPATH:"%ROOT_DIR%\deps\lib" SDL2.lib SDL2main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" %AUDIO_SRC% %COMMON_SRC% -I"%SRC_DIR%" -L"%ROOT_DIR%\deps\lib" -lSDL2 -lSDL2main -lm
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação da ferramenta %TEST_NAME% falhou.
) else (
    echo Compilação da ferramenta %TEST_NAME% bem-sucedida.
    echo Para executar a ferramenta de teste de áudio, use: "%TEST_EXE%"
)

echo.

REM ============================================================================
REM Teste do NES Mapper 0 (NROM)
REM ============================================================================

echo Teste: NES Mapper 0 (NROM)
echo ----------------------------------------------------------------------

set "TEST_NAME=test_mapper0"
set "TEST_SRC=%TESTS_DIR%\platforms\nes\cartridge\mappers\test_mapper0.c"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "MAPPER_SRC=%SRC_DIR%\platforms\nes\cartridge\mappers\mapper0.cpp"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"
set "UNITY_SRC=%ROOT_DIR%\deps\unity\unity.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /I"%ROOT_DIR%\deps\unity" /Fe:"%TEST_EXE%" "%TEST_SRC%" "%MAPPER_SRC%" "%UNITY_SRC%" %COMMON_SRC%
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" "%MAPPER_SRC%" "%UNITY_SRC%" %COMMON_SRC% -I"%SRC_DIR%" -I"%ROOT_DIR%\deps\unity" -std=c99
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do NES Mapper 1 (MMC1)
REM ============================================================================

echo Teste: NES Mapper 1 (MMC1)
echo ----------------------------------------------------------------------

set "TEST_NAME=test_mapper1"
set "TEST_SRC=%TESTS_DIR%\platforms\nes\cartridge\mappers\test_mapper1.c"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "MAPPER_SRC=%SRC_DIR%\platforms\nes\cartridge\mappers\mapper1.cpp"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"
set "UNITY_SRC=%ROOT_DIR%\deps\unity\unity.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /I"%ROOT_DIR%\deps\unity" /Fe:"%TEST_EXE%" "%TEST_SRC%" "%MAPPER_SRC%" "%UNITY_SRC%" %COMMON_SRC%
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" "%MAPPER_SRC%" "%UNITY_SRC%" %COMMON_SRC% -I"%SRC_DIR%" -I"%ROOT_DIR%\deps\unity" -std=c99
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do NES Mapper 2 (UxROM)
REM ============================================================================

echo Teste: NES Mapper 2 (UxROM)
echo ----------------------------------------------------------------------

set "TEST_NAME=test_mapper2"
set "TEST_SRC=%TESTS_DIR%\platforms\nes\cartridge\mappers\test_mapper2.c"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "MAPPER_SRC=%SRC_DIR%\platforms\nes\cartridge\mappers\mapper2.cpp"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"
set "UNITY_SRC=%ROOT_DIR%\deps\unity\unity.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /I"%ROOT_DIR%\deps\unity" /Fe:"%TEST_EXE%" "%TEST_SRC%" "%MAPPER_SRC%" "%UNITY_SRC%" %COMMON_SRC%
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" "%MAPPER_SRC%" "%UNITY_SRC%" %COMMON_SRC% -I"%SRC_DIR%" -I"%ROOT_DIR%\deps\unity" -std=c99
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do NES Mapper 3 (CNROM)
REM ============================================================================

echo Teste: NES Mapper 3 (CNROM)
echo ----------------------------------------------------------------------

set "TEST_NAME=test_mapper3"
set "TEST_SRC=%TESTS_DIR%\platforms\nes\cartridge\mappers\test_mapper3.c"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "MAPPER_SRC=%SRC_DIR%\platforms\nes\cartridge\mappers\mapper3.cpp"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"
set "UNITY_SRC=%ROOT_DIR%\deps\unity\unity.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /I"%ROOT_DIR%\deps\unity" /Fe:"%TEST_EXE%" "%TEST_SRC%" "%MAPPER_SRC%" "%UNITY_SRC%" %COMMON_SRC%
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" "%MAPPER_SRC%" "%UNITY_SRC%" %COMMON_SRC% -I"%SRC_DIR%" -I"%ROOT_DIR%\deps\unity" -std=c99
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do NES APU (Audio Processing Unit)
REM ============================================================================

echo Teste: NES APU
echo ----------------------------------------------------------------------

set "TEST_NAME=test_nes_apu"
set "TEST_SRC=%TESTS_DIR%\platforms\nes\apu\test_nes_apu.c"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "APU_SRC=%SRC_DIR%\platforms\nes\apu\nes_apu.cpp"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"
set "UNITY_SRC=%ROOT_DIR%\deps\unity\unity.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /I"%ROOT_DIR%\deps\unity" /Fe:"%TEST_EXE%" "%TEST_SRC%" "%APU_SRC%" "%UNITY_SRC%" %COMMON_SRC%
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" "%APU_SRC%" "%UNITY_SRC%" %COMMON_SRC% -I"%SRC_DIR%" -I"%ROOT_DIR%\deps\unity" -std=c99
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do NES CPU (Central Processing Unit)
REM ============================================================================

echo Teste: NES CPU
echo ----------------------------------------------------------------------

set "TEST_NAME=test_nes_cpu"
set "TEST_SRC=%TESTS_DIR%\platforms\nes\cpu\test_nes_cpu.c"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "CPU_SRC=%SRC_DIR%\platforms\nes\cpu\nes_cpu.cpp"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"
set "UNITY_SRC=%ROOT_DIR%\deps\unity\unity.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /I"%ROOT_DIR%\deps\unity" /Fe:"%TEST_EXE%" "%TEST_SRC%" "%CPU_SRC%" "%UNITY_SRC%" %COMMON_SRC%
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" "%CPU_SRC%" "%UNITY_SRC%" %COMMON_SRC% -I"%SRC_DIR%" -I"%ROOT_DIR%\deps\unity" -std=c99
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do NES PPU (Picture Processing Unit)
REM ============================================================================

echo Teste: NES PPU
echo ----------------------------------------------------------------------

set "TEST_NAME=test_nes_ppu"
set "TEST_SRC=%TESTS_DIR%\platforms\nes\ppu\test_nes_ppu.c"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "PPU_SRC=%SRC_DIR%\platforms\nes\ppu\nes_ppu.cpp"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"
set "UNITY_SRC=%ROOT_DIR%\deps\unity\unity.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /I"%ROOT_DIR%\deps\unity" /Fe:"%TEST_EXE%" "%TEST_SRC%" "%PPU_SRC%" "%UNITY_SRC%" %COMMON_SRC%
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" "%PPU_SRC%" "%UNITY_SRC%" %COMMON_SRC% -I"%SRC_DIR%" -I"%ROOT_DIR%\deps\unity" -std=c99
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do APU do NES
REM ============================================================================

echo Teste: APU do NES
echo ----------------------------------------------------------------------

set "TEST_NAME=test_nes_apu"
set "TEST_SRC=%TESTS_DIR%\platforms\nes\apu\test_nes_apu.cpp"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "APU_SRC=%SRC_DIR%\platforms\nes\apu\nes_apu.cpp"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /Fe:"%TEST_EXE%" "%TEST_SRC%" %APU_SRC% %COMMON_SRC% /link /LIBPATH:"%ROOT_DIR%\deps\lib" gtest.lib gtest_main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" %APU_SRC% %COMMON_SRC% -I"%SRC_DIR%" -L"%ROOT_DIR%\deps\lib" -lgtest -lgtest_main -lpthread -std=c++11
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do CPU do NES
REM ============================================================================

echo Teste: CPU do NES
echo ----------------------------------------------------------------------

set "TEST_NAME=test_nes_cpu"
set "TEST_SRC=%TESTS_DIR%\platforms\nes\cpu\test_nes_cpu.cpp"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "CPU_SRC=%SRC_DIR%\platforms\nes\cpu\nes_cpu.cpp"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /Fe:"%TEST_EXE%" "%TEST_SRC%" %CPU_SRC% %COMMON_SRC% /link /LIBPATH:"%ROOT_DIR%\deps\lib" gtest.lib gtest_main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" %CPU_SRC% %COMMON_SRC% -I"%SRC_DIR%" -L"%ROOT_DIR%\deps\lib" -lgtest -lgtest_main -lpthread -std=c++11
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do PPU do NES
REM ============================================================================

echo Teste: PPU do NES
echo ----------------------------------------------------------------------

set "TEST_NAME=test_nes_ppu"
set "TEST_SRC=%TESTS_DIR%\platforms\nes\ppu\test_nes_ppu.cpp"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "PPU_SRC=%SRC_DIR%\platforms\nes\ppu\src\nes_ppu.cpp"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /Fe:"%TEST_EXE%" "%TEST_SRC%" %PPU_SRC% %COMMON_SRC% /link /LIBPATH:"%ROOT_DIR%\deps\lib" gtest.lib gtest_main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" %PPU_SRC% %COMMON_SRC% -I"%SRC_DIR%" -L"%ROOT_DIR%\deps\lib" -lgtest -lgtest_main -lpthread -std=c++11
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do VDP do Mega Drive
REM ============================================================================

echo Teste: VDP do Mega Drive
echo ----------------------------------------------------------------------

set "TEST_NAME=test_vdp_system"
set "TEST_SRC=%TESTS_DIR%\platforms\megadrive\video\test_vdp_system.cpp"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "VDP_SRC=%SRC_DIR%\platforms\megadrive\video\vdp.cpp"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /Fe:"%TEST_EXE%" "%TEST_SRC%" %VDP_SRC% %COMMON_SRC% /link /LIBPATH:"%ROOT_DIR%\deps\lib" gtest.lib gtest_main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" %VDP_SRC% %COMMON_SRC% -I"%SRC_DIR%" -L"%ROOT_DIR%\deps\lib" -lgtest -lgtest_main -lpthread -std=c++11
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do Sistema de GUI
REM ============================================================================

echo Teste: Sistema de GUI
echo ----------------------------------------------------------------------

set "TEST_NAME=test_gui_manager"
set "TEST_SRC=%TESTS_DIR%\frontend\gui\test_gui_manager.c"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "GUI_SRC=%SRC_DIR%\frontend\gui\core\gui_manager.c %SRC_DIR%\frontend\gui\core\gui_element.c %SRC_DIR%\frontend\gui\core\gui_common.c"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"
set "UNITY_SRC=%ROOT_DIR%\deps\unity\unity.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /I"%ROOT_DIR%\deps\unity" /Fe:"%TEST_EXE%" "%TEST_SRC%" %GUI_SRC% %COMMON_SRC% %UNITY_SRC% /link /LIBPATH:"%ROOT_DIR%\deps\lib" SDL2.lib SDL2main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" %GUI_SRC% %COMMON_SRC% %UNITY_SRC% -I"%SRC_DIR%" -I"%ROOT_DIR%\deps\unity" -L"%ROOT_DIR%\deps\lib" -lSDL2 -lSDL2main -std=c99
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

REM ============================================================================
REM Teste do Widget de Label
REM ============================================================================

echo Teste: Widget de Label
echo ----------------------------------------------------------------------

set "TEST_NAME=test_gui_label"
set "TEST_SRC=%TESTS_DIR%\frontend\gui\test_gui_label.c"
set "TEST_EXE=%BUILD_DIR%\%TEST_NAME%.exe"
set "GUI_SRC=%SRC_DIR%\frontend\gui\core\gui_manager.c %SRC_DIR%\frontend\gui\core\gui_element.c %SRC_DIR%\frontend\gui\core\gui_common.c"
set "WIDGETS_SRC=%SRC_DIR%\frontend\gui\widgets\gui_label.c"
set "COMMON_SRC=%SRC_DIR%\common\logging.c"
set "UNITY_SRC=%ROOT_DIR%\deps\unity\unity.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /I"%ROOT_DIR%\deps\unity" /Fe:"%TEST_EXE%" "%TEST_SRC%" %GUI_SRC% %WIDGETS_SRC% %COMMON_SRC% %UNITY_SRC% /link /LIBPATH:"%ROOT_DIR%\deps\lib" SDL2.lib SDL2main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%TEST_EXE%" "%TEST_SRC%" %GUI_SRC% %WIDGETS_SRC% %COMMON_SRC% %UNITY_SRC% -I"%SRC_DIR%" -I"%ROOT_DIR%\deps\unity" -L"%ROOT_DIR%\deps\lib" -lSDL2 -lSDL2main -std=c99
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação do teste %TEST_NAME% falhou.
) else (
    echo Executando %TEST_NAME%...
    "%TEST_EXE%"
    if %ERRORLEVEL% NEQ 0 (
        echo FALHA: Teste %TEST_NAME% falhou.
    ) else (
        echo SUCESSO: Teste %TEST_NAME% passou.
    )
)

echo.

echo ============================================================================
echo                   FIM DA EXECUÇÃO DOS TESTES UNITÁRIOS
echo ============================================================================
echo.

endlocal
