# Script para executar todos os testes
param(
    [string]$BuildDir = "build/test",
    [switch]$Verbose,
    [switch]$FailFast,
    [string]$Filter = "*"
)

# Função para encontrar executáveis de teste
function Get-TestExecutables {
    param([string]$Directory)

    Get-ChildItem -Path $Directory -Recurse -Filter "test_*.exe"
}

# Função para executar um teste
function Invoke-Test {
    param(
        [string]$TestPath,
        [switch]$Verbose
    )

    $testName = Split-Path -Leaf $TestPath
    Write-Host "`nExecutando teste: $testName"

    if ($Verbose) {
        & $TestPath
    }
    else {
        & $TestPath | Where-Object { $_ -match "^(FAIL|ERROR|PASS):" }
    }

    return $LASTEXITCODE -eq 0
}

# Verificar se diretório de build existe
if (-not (Test-Path $BuildDir)) {
    Write-Host "Diretório de build não encontrado: $BuildDir"
    Write-Host "Execute 'build_all.ps1 -BuildType Debug -BuildTests' primeiro"
    exit 1
}

# Encontrar todos os executáveis de teste
$testFiles = Get-TestExecutables -Directory $BuildDir | Where-Object { $_.Name -like $Filter }

if ($testFiles.Count -eq 0) {
    Write-Host "Nenhum teste encontrado em: $BuildDir"
    if ($Filter -ne "*") {
        Write-Host "Filtro atual: $Filter"
    }
    exit 1
}

Write-Host "Encontrados $($testFiles.Count) testes"

# Variáveis para estatísticas
$stats = @{
    Total = $testFiles.Count
    Passed = 0
    Failed = 0
    Skipped = 0
}

# Executar cada teste
foreach ($test in $testFiles) {
    $result = Invoke-Test -TestPath $test.FullName -Verbose:$Verbose

    if ($result) {
        $stats.Passed++
        if ($Verbose) {
            Write-Host "PASSOU: $($test.Name)" -ForegroundColor Green
        }
    }
    else {
        $stats.Failed++
        Write-Host "FALHOU: $($test.Name)" -ForegroundColor Red

        if ($FailFast) {
            Write-Host "`nParando execução devido a falha (FailFast)"
            break
        }
    }
}

# Exibir resumo
Write-Host "`nResumo dos Testes"
Write-Host "================"
Write-Host "Total: $($stats.Total)"
Write-Host "Passou: $($stats.Passed)" -ForegroundColor Green
if ($stats.Failed -gt 0) {
    Write-Host "Falhou: $($stats.Failed)" -ForegroundColor Red
}
if ($stats.Skipped -gt 0) {
    Write-Host "Pulou: $($stats.Skipped)" -ForegroundColor Yellow
}

# Calcular cobertura
$coverage = [math]::Round(($stats.Passed / $stats.Total) * 100, 2)
Write-Host "Cobertura: $coverage%"

# Definir código de saída
if ($stats.Failed -gt 0) {
    exit 1
}
else {
    exit 0
}
