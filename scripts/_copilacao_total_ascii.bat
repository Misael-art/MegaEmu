@echo off
setlocal enabledelayedexpansion

REM Definir cores
set "CYAN=[96m"
set "RED=[91m"
set "NC=[0m"

echo %CYAN%===============================================================
echo                   COMPILACAO TOTAL DO MEGA_EMU
echo ===============================================================%NC%

REM Verificar ambiente
echo %CYAN%Verificando ambiente...%NC%

REM Configurar vcpkg
set "VCPKG_ROOT=%~dp0..\deps\vcpkg"
set "PATH=%VCPKG_ROOT%;%PATH%"

REM Instalar dependências via vcpkg
echo %CYAN%Instalando dependências do vcpkg...%NC%
"%VCPKG_ROOT%\vcpkg.exe" install sdl2:x64-windows glew:x64-windows zlib:x64-windows libpng:x64-windows libjpeg-turbo:x64-windows freetype:x64-windows qt5-base:x64-windows

REM Configurar Visual Studio
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

REM Limpar diretórios antigos
echo %CYAN%Limpando diretórios antigos...%NC%
if exist build rmdir /s /q build
mkdir build
cd build

REM Configurar CMake
echo %CYAN%Configurando CMake...%NC%
cmake -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
      -DVCPKG_TARGET_TRIPLET=x64-windows ^
      -DCMAKE_PREFIX_PATH="%VCPKG_ROOT%\installed\x64-windows" ^
      -DCMAKE_INCLUDE_PATH="%VCPKG_ROOT%\installed\x64-windows\include" ^
      -DCMAKE_LIBRARY_PATH="%VCPKG_ROOT%\installed\x64-windows\lib" ^
      -DCMAKE_BUILD_TYPE=Release ^
      ..

if errorlevel 1 (
    echo %RED%[ERRO] Falha na configuração do CMake%NC%
    exit /b 1
)

REM Compilar
echo %CYAN%Compilando...%NC%
cmake --build . --config Release --parallel 4

if errorlevel 1 (
    echo %RED%[ERRO] Falha na compilação%NC%
    exit /b 1
)

echo %CYAN%Compilação concluída com sucesso%NC%

cd ..
exit /b 0
