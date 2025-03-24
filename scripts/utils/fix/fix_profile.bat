@echo off
setlocal enabledelayedexpansion

echo Correcao do Perfil do PowerShell
echo ===============================
echo.

:: Obter o diretorio do script
set "SCRIPT_DIR=%~dp0"
set "PS_RUNNER=%SCRIPT_DIR%fix_profile_runner.ps1"

:: Verificar se o script existe
if not exist "%PS_RUNNER%" (
    echo ERRO: Script PowerShell nao encontrado em: %PS_RUNNER%
    pause
    exit /b 1
)

:: Executar o script PowerShell
powershell -NoProfile -ExecutionPolicy Bypass -File "%PS_RUNNER%"

if %errorLevel% neq 0 (
    echo.
    echo ERRO: A correcao falhou com codigo %errorLevel%
    pause
    exit /b %errorLevel%
)

echo.
echo Correcao concluida com sucesso!
echo Para aplicar as alteracoes, feche e abra novamente o PowerShell.
echo.
pause
