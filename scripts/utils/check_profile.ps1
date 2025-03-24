# Script para verificar o perfil do PowerShell
$ErrorActionPreference = "Stop"

Write-Host "Verificando perfil do PowerShell..."
Write-Host "=================================="
Write-Host ""

Write-Host "Caminho do perfil: $($PROFILE.CurrentUserCurrentHost)"
if (Test-Path $PROFILE.CurrentUserCurrentHost) {
    Write-Host "Conteúdo do perfil:"
    Get-Content $PROFILE.CurrentUserCurrentHost
} else {
    Write-Host "Perfil não encontrado!"
}

Write-Host ""
Write-Host "Verificando variável do Chocolatey..."
if ($env:ChocolateyInstall) {
    Write-Host "ChocolateyInstall: $env:ChocolateyInstall"
    $chocoModule = "$env:ChocolateyInstall\helpers\chocolateyProfile.psm1"
    if (Test-Path $chocoModule) {
        Write-Host "Módulo do Chocolatey encontrado!"
    }
    else {
        Write-Host "Módulo do Chocolatey não encontrado em: $chocoModule"
    }
}
else {
    Write-Host "ChocolateyInstall não definido!"
}
