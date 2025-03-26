@echo off
chcp 65001 > nul
setlocal enabledelayedexpansion

:: Verificar se o arquivo está em UTF-8
findstr /B /C:"﻿" "%~f0" >nul 2>&1
if not errorlevel 1 (
    echo [ERRO] Este arquivo contém BOM UTF-8. Por favor, salve-o como UTF-8 sem BOM.
    exit /b 1
)

:: Habilitar processamento de códigos ANSI
reg add HKCU\Console /v VirtualTerminalLevel /t REG_DWORD /d 1 /f > nul

:: Definir cores (usando códigos ANSI simples)
set "GREEN=[92m"
set "YELLOW=[93m"
set "RED=[91m"
set "CYAN=[96m"
set "RESET=[0m"

:: Definir variáveis de ambiente globais para todo o processo
set "MEGA_EMU_ROOT=%~dp0.."
set "MEGA_EMU_BUILD_DIR=%MEGA_EMU_ROOT%\build"
set "MEGA_EMU_SCRIPTS_DIR=%MEGA_EMU_ROOT%\scripts"
set "MEGA_EMU_ROMS_DIR=%MEGA_EMU_ROOT%\resources\roms"
set "MEGA_EMU_DEPS_CHECKED=0"
set "VCPKG_ROOT=%MEGA_EMU_ROOT%\vcpkg"
set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools"

:: Exibir cabeçalho
echo %CYAN%===============================================================
echo                   COMPILACAO TOTAL DO MEGA_EMU
echo ===============================================================%RESET%

:: Definir diretório base
cd /d "%MEGA_EMU_ROOT%"

:: Verificar se estamos no diretório correto
if not exist "scripts\_copilacao_total.bat" (
    echo %RED%[ERRO] Não foi possível encontrar o diretório raiz do projeto%RESET%
    exit /b 1
)

:: Verificar ambiente
echo %CYAN%Verificando ambiente...%RESET%

where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo %RED%[ERRO] CMake não encontrado%RESET%
    exit /b 1
)

where ninja >nul 2>&1
if %errorlevel% neq 0 (
    echo %RED%[ERRO] Ninja não encontrado%RESET%
    exit /b 1
)

where git >nul 2>&1
if %errorlevel% neq 0 (
    echo %RED%[ERRO] Git não encontrado%RESET%
    exit /b 1
)

:: Verificar Visual Studio
if not exist "%VS_PATH%\VC\Tools\MSVC" (
    echo %RED%[ERRO] Visual Studio Build Tools não encontrado%RESET%
    exit /b 1
)

:: Configurar vcpkg
if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo %YELLOW%Instalando vcpkg...%RESET%
    git clone https://github.com/Microsoft/vcpkg.git "%VCPKG_ROOT%"
    call "%VCPKG_ROOT%\bootstrap-vcpkg.bat"
)

:: Criar estrutura de diretórios
echo %CYAN%Criando estrutura de diretórios...%RESET%
set "dirs=test temp emulators\nes emulators\megadrive emulators\mastersystem frontend\sdl frontend\qt Mega_tools released"
for %%d in (%dirs%) do (
    if not exist "%MEGA_EMU_BUILD_DIR%\%%d" (
        mkdir "%MEGA_EMU_BUILD_DIR%\%%d"
        echo %GREEN%Criado: %%d%RESET%
    )
)

:: Executar scripts de build
echo %CYAN%Iniciando processo de build...%RESET%

set "error_count=0"

:: Build Tools
echo %CYAN%Compilando Mega Tools...%RESET%
pushd "%MEGA_EMU_BUILD_DIR%\Mega_tools"
cmake "%MEGA_EMU_ROOT%\src\tools" -G "Ninja" || set /a "error_count+=1"
ninja || set /a "error_count+=1"
popd

:: Build NES
echo %CYAN%Compilando Emulador NES...%RESET%
pushd "%MEGA_EMU_BUILD_DIR%\emulators\nes"
cmake "%MEGA_EMU_ROOT%\src\emulators\nes" -G "Ninja" || set /a "error_count+=1"
ninja || set /a "error_count+=1"
popd

:: Build MegaDrive
echo %CYAN%Compilando Emulador MegaDrive...%RESET%
pushd "%MEGA_EMU_BUILD_DIR%\emulators\megadrive"
cmake "%MEGA_EMU_ROOT%\src\emulators\megadrive" -G "Ninja" || set /a "error_count+=1"
ninja || set /a "error_count+=1"
popd

:: Build Frontend SDL
echo %CYAN%Compilando Frontend SDL...%RESET%
pushd "%MEGA_EMU_BUILD_DIR%\frontend\sdl"
cmake "%MEGA_EMU_ROOT%\src\frontend\sdl" -G "Ninja" || set /a "error_count+=1"
ninja || set /a "error_count+=1"
popd

:: Resumo
echo %CYAN%===============================================================
echo                        RESUMO DA COMPILAÇÃO
echo ===============================================================%RESET%

if %error_count% equ 0 (
    echo %GREEN%Compilação concluída com sucesso!%RESET%
) else (
    echo %RED%Compilação concluída com %error_count% erro(s)%RESET%
)

:: Log
echo Compilação concluída em: %date% %time% > "%MEGA_EMU_ROOT%\_ultima_compilacao.log"
echo Total de erros: %error_count% >> "%MEGA_EMU_ROOT%\_ultima_compilacao.log"

if %error_count% neq 0 (
    pause
    exit /b 1
)

exit /b 0
