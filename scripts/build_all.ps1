# Script para compilar todos os componentes
param(
    [string]$BuildType = "Release",
    [switch]$Clean = $false,
    [switch]$Rebuild = $false
)

# Importar funções comuns
. "$PSScriptRoot/../common/build_utils.ps1"

Write-ColorMessage "=== Iniciando build completo do Mega_Emu ===" "Cyan"

# Array com todos os scripts de build
$buildScripts = @(
    "build_nes.ps1",
    "build_megadrive.ps1",
    "build_frontend_sdl.ps1",
    "build_tools.ps1"
)

$successCount = 0
$failureCount = 0

foreach ($script in $buildScripts) {
    Write-ColorMessage "`nExecutando $script..." "Yellow"

    & "$PSScriptRoot/$script" -BuildType $BuildType -Clean:$Clean -Rebuild:$Rebuild

    if ($LASTEXITCODE -eq 0) {
        $successCount++
        Write-ColorMessage "✓ $script concluído com sucesso" "Green"
    }
    else {
        $failureCount++
        Write-ColorMessage "✗ $script falhou" "Red"
    }
}

# Build do frontend React
Write-Host "🔨 Building frontend React..."
& "$PSScriptRoot\build_frontend_react.ps1" -BuildType $BuildType -Clean:$Clean -Rebuild:$Rebuild
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Frontend React build failed!" -ForegroundColor Red
    $failureCount++
}
else {
    Write-Host "✅ Frontend React build succeeded!" -ForegroundColor Green
    $successCount++
}

Write-ColorMessage "`n=== Resumo do build ===" "Cyan"
Write-ColorMessage "Builds com sucesso: $successCount" "Green"
Write-ColorMessage "Builds com falha: $failureCount" "Red"

if ($failureCount -gt 0) {
    Write-ColorMessage "`nBuild completo falhou!" "Red"
    exit 1
}
else {
    Write-ColorMessage "`nBuild completo concluído com sucesso!" "Green"
    exit 0
}
