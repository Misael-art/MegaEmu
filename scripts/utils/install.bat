@echo off
setlocal enabledelayedexpansion

:: Script de instalacao para Mega_Emu
:: Verifica privilegios de administrador e eleva se necessario

echo [DEBUG] Iniciando script de instalacao...
echo [DEBUG] Diretorio atual: %CD%

:: Verificar se o script PowerShell existe
set "SCRIPT_PATH=%~dp0install.ps1"
echo [DEBUG] Procurando script em: %SCRIPT_PATH%

if not exist "%SCRIPT_PATH%" (
    echo ERRO: Script PowerShell nao encontrado em: %SCRIPT_PATH%
    pause
    exit /b 1
)

:: Verificar privilegios de administrador
echo [DEBUG] Verificando privilegios de administrador...
net session >nul 2>&1
if %errorLevel% == 0 (
    echo [DEBUG] Ja executando como administrador
    goto :runScript
) else (
    echo [DEBUG] Elevando privilegios...
    powershell -Command "Start-Process -FilePath '%~dpnx0' -Verb RunAs -Wait"
    exit /b
)

:runScript
echo [DEBUG] Executando script PowerShell...
echo [DEBUG] Comando: powershell -NoProfile -ExecutionPolicy Bypass -Command "& '%SCRIPT_PATH%' %*"

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "Write-Host '[DEBUG] PowerShell iniciado'; ^
     Write-Host '[DEBUG] Executando script:' '%SCRIPT_PATH%'; ^
     & '%SCRIPT_PATH%' %*"

if %errorLevel% neq 0 (
    echo [DEBUG] Script PowerShell falhou com codigo: %errorLevel%
    pause
    exit /b %errorLevel%
)

echo [DEBUG] Instalacao concluida com sucesso
pause
