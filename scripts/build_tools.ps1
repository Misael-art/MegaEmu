# Script de build para as ferramentas Mega_tools
param(
    [string]$BuildType = "Release",
    [string]$BuildDir = "build/Mega_tools",
    [switch]$Clean = $false,
    [switch]$Rebuild = $false
)

# Importar funções comuns
. "$PSScriptRoot/../common/build_utils.ps1"

# Configurar opções específicas das ferramentas
$CMAKE_OPTIONS = @(
    "-DBUILD_NES=OFF",
    "-DBUILD_MEGADRIVE=OFF",
    "-DBUILD_MASTERSYSTEM=OFF",
    "-DBUILD_FRONTEND_SDL=OFF",
    "-DBUILD_FRONTEND_QT=OFF",
    "-DBUILD_TOOLS=ON",
    "-DBUILD_TESTS=OFF"
)

# Executar build
Build-Component -ComponentName "Mega_tools" -BuildType $BuildType -BuildDir $BuildDir -Clean:$Clean -Rebuild:$Rebuild -CMakeOptions $CMAKE_OPTIONS
