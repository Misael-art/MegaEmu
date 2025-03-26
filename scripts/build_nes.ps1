# Script de build para o emulador NES
param(
    [string]$BuildType = "Release",
    [string]$BuildDir = "build/emulators/nes",
    [switch]$Clean = $false,
    [switch]$Rebuild = $false
)

# Importar funções comuns
. "$PSScriptRoot/../common/build_utils.ps1"

# Configurar opções específicas do NES
$CMAKE_OPTIONS = @(
    "-DBUILD_NES=ON",
    "-DBUILD_MEGADRIVE=OFF",
    "-DBUILD_MASTERSYSTEM=OFF",
    "-DBUILD_FRONTEND_SDL=ON",
    "-DBUILD_FRONTEND_QT=OFF",
    "-DBUILD_TOOLS=OFF",
    "-DBUILD_TESTS=OFF"
)

# Executar build
Build-Component -ComponentName "NES" -BuildType $BuildType -BuildDir $BuildDir -Clean:$Clean -Rebuild:$Rebuild -CMakeOptions $CMAKE_OPTIONS
