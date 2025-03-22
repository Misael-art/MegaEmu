# Script para testar diferentes configurações de compilação
# test_build_configs.ps1
# Autor: Mega_Emu Team
# Testa diferentes configurações de compilação para garantir compatibilidade

$projectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$testBuildDir = Join-Path -Path $projectRoot -ChildPath "build_test"

# Configurações de compilação a serem testadas
$configurations = @(
    "-DBUILD_MEGADRIVE=ON -DBUILD_MASTERSYSTEM=ON -DBUILD_NES=ON -DBUILD_SNES=OFF",
    "-DBUILD_MEGADRIVE=ON -DBUILD_MASTERSYSTEM=OFF -DBUILD_NES=OFF -DBUILD_SNES=OFF",
    "-DBUILD_MEGADRIVE=OFF -DBUILD_MASTERSYSTEM=ON -DBUILD_NES=OFF -DBUILD_SNES=OFF",
    "-DBUILD_MEGADRIVE=OFF -DBUILD_MASTERSYSTEM=OFF -DBUILD_NES=ON -DBUILD_SNES=OFF",
    "-DBUILD_MEGADRIVE=OFF -DBUILD_MASTERSYSTEM=OFF -DBUILD_NES=OFF -DBUILD_SNES=ON"
)

Write-Host "Testando configurações de compilação do projeto Mega_Emu..."
Write-Host "Diretório raiz do projeto: $projectRoot"
Write-Host "----------------------------------------------------"
Write-Host ""

# Limpar e criar diretório de build
if (Test-Path -Path $testBuildDir) {
    Remove-Item -Path $testBuildDir -Recurse -Force
}
New-Item -Path $testBuildDir -ItemType Directory | Out-Null

# Função para executar um comando e verificar o resultado
function Invoke-CommandWithCheck {
    param(
        [string]$Command,
        [string]$WorkingDir
    )

    $output = $null
    $exitCode = 0

    try {
        Push-Location $WorkingDir
        $output = Invoke-Expression -Command $Command -ErrorAction Stop
        $exitCode = $LASTEXITCODE
    }
    catch {
        $exitCode = 1
        Write-Host "ERRO: $($_.Exception.Message)" -ForegroundColor Red
    }
    finally {
        Pop-Location
    }

    return @{
        Output = $output
        ExitCode = $exitCode
    }
}

# Testar cada configuração
$testIndex = 1
$totalTests = $configurations.Count
$successCount = 0

foreach ($config in $configurations) {
    $testDir = Join-Path -Path $testBuildDir -ChildPath "test_$testIndex"
    New-Item -Path $testDir -ItemType Directory | Out-Null

    Write-Host "Teste $testIndex/$totalTests: Configuração: $config" -ForegroundColor Cyan

    # Executar CMake com a configuração atual
    $cmakeCommand = "cmake $projectRoot $config"
    Write-Host "  Executando: $cmakeCommand"
    $result = Invoke-CommandWithCheck -Command $cmakeCommand -WorkingDir $testDir

    if ($result.ExitCode -ne 0) {
        Write-Host "  FALHA: Erro na configuração do CMake" -ForegroundColor Red
        continue
    }

    # Compilar o projeto
    $buildCommand = "cmake --build ."
    Write-Host "  Executando: $buildCommand"
    $result = Invoke-CommandWithCheck -Command $buildCommand -WorkingDir $testDir

    if ($result.ExitCode -ne 0) {
        Write-Host "  FALHA: Erro na compilação" -ForegroundColor Red
    }
    else {
        Write-Host "  SUCESSO: Compilação concluída" -ForegroundColor Green
        $successCount++
    }

    Write-Host ""
    $testIndex++
}

# Resumo final
Write-Host "Resumo dos testes de compilação:"
Write-Host "  Total de configurações testadas: $totalTests"
Write-Host "  Configurações bem-sucedidas: $successCount"
Write-Host "  Configurações com falha: $($totalTests - $successCount)"

if ($successCount -eq $totalTests) {
    Write-Host "TODOS OS TESTES PASSARAM!" -ForegroundColor Green
}
else {
    $failRate = [math]::Round(($totalTests - $successCount) * 100 / $totalTests, 1)
    Write-Host "ATENÇÃO: $failRate% DOS TESTES FALHARAM!" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Testes de configuração concluídos."
