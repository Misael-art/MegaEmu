# Script para verificar a estrutura do projeto
# check_structure.ps1
# Autor: Mega_Emu Team
# Verifica se todas as plataformas seguem a estrutura padrão definida

$projectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$platformsDir = Join-Path -Path $projectRoot -ChildPath "src\platforms"

# Lista de plataformas a verificar
$platforms = @("megadrive", "mastersystem", "nes", "snes")

# Lista de diretórios padrão que cada plataforma deve ter
$standardDirs = @("cpu", "video", "audio", "memory", "io")

Write-Host "Verificando estrutura do projeto Mega_Emu..."
Write-Host "Diretório raiz do projeto: $projectRoot"
Write-Host "--------------------------------------------"

foreach ($platform in $platforms) {
    $platformPath = Join-Path -Path $platformsDir -ChildPath $platform

    if (-not (Test-Path -Path $platformPath -PathType Container)) {
        Write-Host "ERRO: Plataforma $platform não encontrada em $platformsDir" -ForegroundColor Red
        continue
    }

    Write-Host "Verificando estrutura para: $platform" -ForegroundColor Cyan

    # Verificar arquivos principais
    $mainFiles = @("$platform.c", "$platform.h", "CMakeLists.txt")
    foreach ($file in $mainFiles) {
        $filePath = Join-Path -Path $platformPath -ChildPath $file
        if (-not (Test-Path -Path $filePath -PathType Leaf)) {
            Write-Host "  AVISO: Arquivo $file não encontrado em $platform" -ForegroundColor Yellow
        } else {
            Write-Host "  OK: Arquivo $file encontrado" -ForegroundColor Green
        }
    }

    # Verificar diretórios padrão
    foreach ($dir in $standardDirs) {
        $dirPath = Join-Path -Path $platformPath -ChildPath $dir
        if (-not (Test-Path -Path $dirPath -PathType Container)) {
            Write-Host "  AVISO: Diretório $dir não encontrado em $platform" -ForegroundColor Yellow
        } else {
            Write-Host "  OK: Diretório $dir encontrado" -ForegroundColor Green
        }
    }

    Write-Host ""
}

Write-Host "Verificação concluída!" -ForegroundColor Green
