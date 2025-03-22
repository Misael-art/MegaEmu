# Script para verificar a conformidade das interfaces de plataforma
# check_platform_interfaces.ps1
# Autor: Mega_Emu Team
# Verifica se todas as plataformas implementam as interfaces padrão requeridas

$projectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$platformsDir = Join-Path -Path $projectRoot -ChildPath "src\platforms"

# Lista de plataformas para verificar
$platforms = @("megadrive", "mastersystem", "nes", "snes")

# Lista de funções de interface que cada plataforma deve implementar
$requiredFunctions = @(
    "_init",
    "_reset",
    "_shutdown",
    "_load_rom",
    "_run_frame",
    "_get_display_buffer",
    "_get_audio_buffer",
    "_get_input_state",
    "_set_config"
)

Write-Host "Verificando interfaces de plataforma do projeto Mega_Emu..."
Write-Host "Diretório raiz do projeto: $projectRoot"
Write-Host "----------------------------------------------------"
Write-Host ""

$overallSuccess = $true

foreach ($platform in $platforms) {
    Write-Host "Verificando plataforma: $platform" -ForegroundColor Cyan

    $platformFile = Join-Path -Path $platformsDir -ChildPath "$platform\$platform.c"

    if (-not (Test-Path -Path $platformFile)) {
        Write-Host "  ERRO: Arquivo principal da plataforma não encontrado: $platformFile" -ForegroundColor Red
        $overallSuccess = $false
        continue
    }

    $content = Get-Content -Path $platformFile -Raw
    $platformMissingFunctions = @()

    foreach ($func in $requiredFunctions) {
        $functionName = "${platform}${func}"
        if ($content -match [regex]::Escape($functionName)) {
            Write-Host "  ✓ Função $functionName encontrada" -ForegroundColor Green
        } else {
            Write-Host "  ✗ Função $functionName NÃO encontrada" -ForegroundColor Red
            $platformMissingFunctions += $functionName
            $overallSuccess = $false
        }
    }

    # Verificar também o arquivo de cabeçalho
    $headerFile = Join-Path -Path $platformsDir -ChildPath "$platform\$platform.h"

    if (-not (Test-Path -Path $headerFile)) {
        Write-Host "  ERRO: Arquivo de cabeçalho da plataforma não encontrado: $headerFile" -ForegroundColor Red
        $overallSuccess = $false
    } else {
        $headerContent = Get-Content -Path $headerFile -Raw

        foreach ($func in $requiredFunctions) {
            $functionName = "${platform}${func}"
            if ($headerContent -match [regex]::Escape($functionName) -and $headerContent -match [regex]::Escape("$functionName\(")) {
                Write-Host "  ✓ Protótipo da função $functionName declarado no cabeçalho" -ForegroundColor Green
            } else {
                if ($platformMissingFunctions -notcontains $functionName) {
                    Write-Host "  ✗ Protótipo da função $functionName NÃO declarado no cabeçalho" -ForegroundColor Red
                    $overallSuccess = $false
                }
            }
        }
    }

    Write-Host ""
}

# Resumo final
if ($overallSuccess) {
    Write-Host "VERIFICAÇÃO CONCLUÍDA: Todas as interfaces de plataforma estão conformes." -ForegroundColor Green
} else {
    Write-Host "VERIFICAÇÃO CONCLUÍDA: ALGUMAS INTERFACES DE PLATAFORMA ESTÃO INCOMPLETAS!" -ForegroundColor Red
    Write-Host "Por favor, corrija as funções ausentes nas plataformas indicadas acima."
}

Write-Host ""
Write-Host "Verificação de interfaces concluída."
