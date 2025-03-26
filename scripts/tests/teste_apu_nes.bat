@echo on
setlocal enabledelayedexpansion

REM Definir diretórios
set "ROOT_DIR=%~dp0..\..\..\"
set "SRC_DIR=%ROOT_DIR%src"
set "TESTS_DIR=%ROOT_DIR%tests"
set "BUILD_DIR=%ROOT_DIR%build\tests"

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
        exit /b 1
    )
)

echo.
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
pause
