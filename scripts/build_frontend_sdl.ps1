# Script de build para o frontend SDL
param(
    [string]$BuildType = "Release",
    [string]$BuildDir = "build/frontend/sdl",
    [switch]$Clean = $false,
    [switch]$Rebuild = $false
)

# Importar funções comuns
. "$PSScriptRoot/../common/build_utils.ps1"

# Configurar opções específicas do frontend SDL
$CMAKE_OPTIONS = @(
    "-DBUILD_NES=OFF",
    "-DBUILD_MEGADRIVE=OFF",
    "-DBUILD_MASTERSYSTEM=OFF",
    "-DBUILD_FRONTEND_SDL=ON",
    "-DBUILD_FRONTEND_QT=OFF",
    "-DBUILD_TOOLS=OFF",
    "-DBUILD_TESTS=OFF",
    "-DUSE_SDL2=ON"
)

# Executar build
Build-Component -ComponentName "Frontend_SDL" -BuildType $BuildType -BuildDir $BuildDir -Clean:$Clean -Rebuild:$Rebuild -CMakeOptions $CMAKE_OPTIONS
