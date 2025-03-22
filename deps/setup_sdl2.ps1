# Script para configurar o SDL2
param (
    [string]$Version = "2.32.2"
)

$ErrorActionPreference = "Stop"

# Diretórios
$depsDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$sdl2Dir = Join-Path $depsDir "sdl2\$Version"
$tempDir = Join-Path $depsDir "temp"

# Criar diretórios temporários
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null

# Download do SDL2
$sdl2Url = "https://github.com/libsdl-org/SDL/releases/download/release-$Version/SDL2-devel-$Version-VC.zip"
$sdl2Zip = Join-Path $tempDir "SDL2.zip"
Write-Host "Baixando SDL2 $Version..."
Invoke-WebRequest -Uri $sdl2Url -OutFile $sdl2Zip

# Extrair SDL2
Write-Host "Extraindo SDL2..."
Expand-Archive -Path $sdl2Zip -DestinationPath $tempDir -Force
$extractedDir = Join-Path $tempDir "SDL2-$Version"

# Copiar arquivos
Write-Host "Copiando arquivos..."
Copy-Item "$extractedDir\include\*" -Destination "$sdl2Dir\include" -Recurse -Force
Copy-Item "$extractedDir\lib\x64\*" -Destination "$sdl2Dir\lib" -Force
Copy-Item "$extractedDir\lib\x64\*.dll" -Destination "$sdl2Dir\bin" -Force

# Limpar
Write-Host "Limpando arquivos temporários..."
Remove-Item -Path $tempDir -Recurse -Force

Write-Host "SDL2 $Version configurado com sucesso!"
