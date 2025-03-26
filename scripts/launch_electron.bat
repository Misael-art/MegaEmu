@echo off
setlocal enabledelayedexpansion

:: Script para lançar o frontend do Mega Emu no modo desktop (Electron)

:: Definir cores para CMD
set "CYAN=[36m"
set "GREEN=[32m"
set "YELLOW=[33m"
set "RED=[31m"
set "RESET=[0m"

:: Definir caminhos
set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR=%SCRIPT_DIR%..\..\"
set "FRONTEND_DIR=%ROOT_DIR%frontend\"

echo %CYAN%=====================================%RESET%
echo %CYAN%    Mega Emu - Modo Desktop (Electron)%RESET%
echo %CYAN%=====================================%RESET%
echo.

:: Verificar argumentos
set DEV=0
set BUILD=0

if "%1"=="-dev" (
    set DEV=1
) else if "%1"=="-build" (
    set BUILD=1
) else (
    echo %YELLOW%Uso: launch_electron.bat [-dev] [-build]%RESET%
    echo.
    echo %YELLOW%Opções:%RESET%
    echo %GREEN%  -dev    : Iniciar o Electron em modo de desenvolvimento (hot reload)%RESET%
    echo %CYAN%  -build  : Construir uma versão portátil do aplicativo%RESET%
    echo.
    echo %YELLOW%Usando o modo de desenvolvimento por padrão...%RESET%
    set DEV=1
)

:: Verificar se a pasta do frontend existe
if not exist "%FRONTEND_DIR%" (
    echo %RED%Erro: Diretório do frontend não encontrado em %FRONTEND_DIR%%RESET%
    exit /b 1
)

:: Navegar para o diretório do frontend
echo %YELLOW%Navegando para o diretório do frontend...%RESET%
cd /d "%FRONTEND_DIR%"

:: Verificar dependências
echo %YELLOW%Verificando o Node.js e npm...%RESET%
node --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo %RED%Erro: Node.js não encontrado. Por favor, instale o Node.js.%RESET%
    exit /b 1
)
echo %GREEN%Node.js instalado!%RESET%

:: Verificar se o Electron está instalado
echo %YELLOW%Verificando as dependências do Electron...%RESET%
if not exist "%FRONTEND_DIR%node_modules\electron" (
    echo %YELLOW%Instalando dependências (isso pode levar alguns minutos)...%RESET%
    call npm install
    if %ERRORLEVEL% neq 0 (
        echo %RED%Erro ao instalar dependências.%RESET%
        exit /b 1
    )
)

:: Executar o Electron em modo de desenvolvimento ou construir uma versão distribuível
if %DEV%==1 (
    echo %GREEN%Iniciando o Electron em modo de desenvolvimento...%RESET%
    echo %YELLOW%Aguarde enquanto o ambiente está sendo preparado...%RESET%

    call npm run electron:dev
    if %ERRORLEVEL% neq 0 (
        echo %RED%Erro ao iniciar o Electron em modo de desenvolvimento.%RESET%
        exit /b 1
    )
) else if %BUILD%==1 (
    echo %CYAN%Construindo o aplicativo Electron para distribuição...%RESET%
    echo %YELLOW%Isso pode levar alguns minutos, por favor aguarde...%RESET%

    call npm run electron:build
    if %ERRORLEVEL% neq 0 (
        echo %RED%Erro ao construir o aplicativo Electron.%RESET%
        exit /b 1
    )

    :: Verificar se a build foi bem-sucedida
    if exist "%FRONTEND_DIR%dist\" (
        echo %GREEN%Build concluída com sucesso!%RESET%
        echo %GREEN%Verifique o diretório dist para o executável gerado.%RESET%

        :: Perguntar se deseja executar o aplicativo
        set /p RUN_APP=Deseja executar o aplicativo agora? (S/N)
        if /i "!RUN_APP!"=="S" (
            for /f "delims=" %%i in ('dir /s /b "%FRONTEND_DIR%dist\*.exe"') do (
                start "" "%%i"
                goto :done_run
            )
            echo %YELLOW%Aviso: Não foi encontrado o executável.%RESET%
            :done_run
        )
    ) else (
        echo %YELLOW%Aviso: Build concluída, mas o diretório de distribuição não foi encontrado.%RESET%
    )
)

echo %CYAN%Script de lançamento concluído.%RESET%
endlocal
