@echo off
setlocal enabledelayedexpansion

REM ============================================================================
REM Script de Compilação para a Demonstração de GUI do Mega_Emu
REM Autor: Equipe Mega_Emu
REM Data: 25/03/2025
REM ============================================================================

echo.
echo ============================================================================
echo                   MEGA_EMU - COMPILAÇÃO DA DEMO DE GUI
echo ============================================================================
echo.

REM Definir diretórios
set "ROOT_DIR=%~dp0..\..\\"
set "SRC_DIR=%ROOT_DIR%src"
set "BUILD_DIR=%ROOT_DIR%build\examples\gui_demo"
set "DEPS_DIR=%ROOT_DIR%deps"

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
echo Compilando a demonstração de GUI...
echo.

set "DEMO_SRC=%~dp0gui_demo.c"
set "DEMO_EXE=%BUILD_DIR%\gui_demo.exe"
set "GUI_SRC=%SRC_DIR%\frontend\gui\core\gui_manager.c %SRC_DIR%\frontend\gui\core\gui_element.c %SRC_DIR%\frontend\gui\core\gui_common.c"
set "WIDGETS_SRC=%SRC_DIR%\frontend\gui\widgets\gui_button.c %SRC_DIR%\frontend\gui\widgets\gui_label.c"
set "FRONTEND_SRC=%SRC_DIR%\frontend\common\frontend.c"
set "PLATFORM_SRC=%SRC_DIR%\frontend\platform\sdl\sdl_frontend_adapter.c"
set "COMMON_SRC=%SRC_DIR%\utils\enhanced_log.c"

if "%COMPILER%"=="msvc" (
    echo Compilando com MSVC...
    cl /nologo /EHsc /MD /I"%SRC_DIR%" /I"%DEPS_DIR%\include" /Fe:"%DEMO_EXE%" "%DEMO_SRC%" %GUI_SRC% %WIDGETS_SRC% %FRONTEND_SRC% %PLATFORM_SRC% %COMMON_SRC% /link /LIBPATH:"%DEPS_DIR%\lib" SDL2.lib SDL2main.lib
) else (
    echo Compilando com GCC...
    gcc -o "%DEMO_EXE%" "%DEMO_SRC%" %GUI_SRC% %WIDGETS_SRC% %FRONTEND_SRC% %PLATFORM_SRC% %COMMON_SRC% -I"%SRC_DIR%" -I"%DEPS_DIR%\include" -L"%DEPS_DIR%\lib" -lSDL2 -lSDL2main -std=c99
)

if %ERRORLEVEL% NEQ 0 (
    echo FALHA: Compilação da demonstração de GUI falhou.
    exit /b 1
) else (
    echo SUCESSO: Demonstração de GUI compilada com sucesso.
    echo.
    echo Para executar a demonstração, use:
    echo %DEMO_EXE%
    echo.
)

REM Copiar DLLs necessárias para o diretório de build
if exist "%DEPS_DIR%\bin\SDL2.dll" (
    copy "%DEPS_DIR%\bin\SDL2.dll" "%BUILD_DIR%\" > nul
    echo Copiado SDL2.dll para o diretório de build.
)

echo ============================================================================
echo                   FIM DA COMPILAÇÃO
echo ============================================================================
echo.

exit /b 0
