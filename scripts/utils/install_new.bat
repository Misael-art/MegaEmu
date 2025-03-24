@echo off
setlocal enabledelayedexpansion

echo Iniciando instalacao do Mega_Emu...
echo.

:: Obter o diretorio do script
set "SCRIPT_DIR=%~dp0"
set "VBS_SCRIPT=%SCRIPT_DIR%install.vbs"

:: Verificar se o script VBS existe
if not exist "%VBS_SCRIPT%" (
    echo ERRO: Script VBS nao encontrado em: %VBS_SCRIPT%
    pause
    exit /b 1
)

:: Executar o script VBS
echo Iniciando processo de instalacao...
cscript //nologo "%VBS_SCRIPT%"

echo.
echo Processo de instalacao iniciado.
echo Se uma janela de UAC aparecer, por favor aceite para continuar a instalacao.
echo.
pause
