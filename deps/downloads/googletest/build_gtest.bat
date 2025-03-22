@echo on
setlocal enabledelayedexpansion

REM Definir diretórios
set "GTEST_SRC=%~dp0googletest-1.14.0"
set "GTEST_BUILD=%~dp0build"
set "GTEST_DEST=%~dp0..\..\lib"
set "GTEST_INCLUDE=%~dp0..\..\include"

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

REM Compilar GoogleTest
if "%COMPILER%"=="msvc" (
    echo Compilando GoogleTest com MSVC...
    
    REM Compilar gtest
    cl /nologo /EHsc /MD /I"%GTEST_SRC%\googletest\include" /I"%GTEST_SRC%\googletest" /c "%GTEST_SRC%\googletest\src\gtest-all.cc" /Fo"%GTEST_BUILD%\gtest-all.obj"
    
    REM Compilar gtest_main
    cl /nologo /EHsc /MD /I"%GTEST_SRC%\googletest\include" /I"%GTEST_SRC%\googletest" /c "%GTEST_SRC%\googletest\src\gtest_main.cc" /Fo"%GTEST_BUILD%\gtest_main.obj"
    
    REM Criar bibliotecas estáticas
    lib /nologo /out:"%GTEST_BUILD%\gtest.lib" "%GTEST_BUILD%\gtest-all.obj"
    lib /nologo /out:"%GTEST_BUILD%\gtest_main.lib" "%GTEST_BUILD%\gtest_main.obj"
    
) else (
    echo Compilando GoogleTest com GCC...
    
    REM Compilar gtest
    g++ -I"%GTEST_SRC%\googletest\include" -I"%GTEST_SRC%\googletest" -c "%GTEST_SRC%\googletest\src\gtest-all.cc" -o "%GTEST_BUILD%\gtest-all.o"
    
    REM Compilar gtest_main
    g++ -I"%GTEST_SRC%\googletest\include" -I"%GTEST_SRC%\googletest" -c "%GTEST_SRC%\googletest\src\gtest_main.cc" -o "%GTEST_BUILD%\gtest_main.o"
    
    REM Criar bibliotecas estáticas
    ar rcs "%GTEST_BUILD%\libgtest.a" "%GTEST_BUILD%\gtest-all.o"
    ar rcs "%GTEST_BUILD%\libgtest_main.a" "%GTEST_BUILD%\gtest_main.o"
)

REM Copiar bibliotecas para o diretório de destino
if "%COMPILER%"=="msvc" (
    copy /Y "%GTEST_BUILD%\gtest.lib" "%GTEST_DEST%\"
    copy /Y "%GTEST_BUILD%\gtest_main.lib" "%GTEST_DEST%\"
) else (
    copy /Y "%GTEST_BUILD%\libgtest.a" "%GTEST_DEST%\gtest.lib"
    copy /Y "%GTEST_BUILD%\libgtest_main.a" "%GTEST_DEST%\gtest_main.lib"
)

echo.
echo GoogleTest compilado e instalado com sucesso!
echo.

endlocal
