# Script de build para o emulador Mega Drive
param(
    [string]$BuildType = "Release",
    [string]$BuildDir = "build/emulators/megadrive",
    [switch]$Clean = $false,
    [switch]$Rebuild = $false
)

# Importar funções comuns
. "$PSScriptRoot/../common/build_utils.ps1"

# Configurar opções específicas do Mega Drive
$CMAKE_OPTIONS = @(
    "-DBUILD_NES=OFF",
    "-DBUILD_MEGADRIVE=ON",
    "-DBUILD_MASTERSYSTEM=OFF",
    "-DBUILD_FRONTEND_SDL=ON",
    "-DBUILD_FRONTEND_QT=OFF",
    "-DBUILD_TOOLS=OFF",
    "-DBUILD_TESTS=OFF"
)

# Executar build
Build-Component -ComponentName "MegaDrive" -BuildType $BuildType -BuildDir $BuildDir -Clean:$Clean -Rebuild:$Rebuild -CMakeOptions $CMAKE_OPTIONS
