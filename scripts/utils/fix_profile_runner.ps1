# Script intermediário para executar o fix_profile.ps1
param(
    [switch]$Elevated
)

function Test-Admin {
    $currentUser = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    return $currentUser.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not $Elevated) {
    if (-not (Test-Admin)) {
        Start-Process powershell.exe -Verb RunAs -ArgumentList ("-NoProfile -ExecutionPolicy Bypass -File `"{0}`" -Elevated" -f ($MyInvocation.MyCommand.Path))
        exit
    }
}

# Configurar título da janela
$Host.UI.RawUI.WindowTitle = 'Correcao do Perfil PowerShell'

# Obter caminho do script principal
$scriptPath = Join-Path $PSScriptRoot "fix_profile.ps1"

if (-not (Test-Path $scriptPath)) {
    Write-Host "ERRO: Script principal nao encontrado em: $scriptPath" -ForegroundColor Red
    Read-Host "Pressione ENTER para sair"
    exit 1
}

try {
    # Executar o script principal
    Write-Host "Iniciando processo de correcao..." -ForegroundColor Cyan
    & $scriptPath -Verbose
}
catch {
    Write-Host "`nERRO: $($_.Exception.Message)" -ForegroundColor Red
    Read-Host "Pressione ENTER para sair"
    exit 1
}
