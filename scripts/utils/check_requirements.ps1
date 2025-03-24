# Script de Verificação de Requisitos para Mega_Emu
# Autor: Assistant AI
# Data: 2024
# Descrição: Verifica se o sistema atende aos requisitos mínimos para compilação

[CmdletBinding()]
param()

# Função para verificar versão do Windows
function Test-WindowsVersion {
    $osInfo = Get-WmiObject -Class Win32_OperatingSystem
    $version = [Version]$osInfo.Version

    if ($version -lt [Version]"10.0") {
        throw "Este script requer Windows 10 ou superior. Versão atual: $($osInfo.Caption)"
    }

    Write-Host "✓ Versão do Windows OK: $($osInfo.Caption)" -ForegroundColor Green
}

# Função para verificar espaço em disco
function Test-DiskSpace {
    $requiredSpace = 10GB
    $systemDrive = (Get-Item $env:SystemDrive)
    $freeSpace = (Get-PSDrive $systemDrive.Name).Free

    if ($freeSpace -lt $requiredSpace) {
        throw "Espaço em disco insuficiente. Necessário: $($requiredSpace/1GB)GB, Disponível: $([math]::Round($freeSpace/1GB, 2))GB"
    }

    Write-Host "✓ Espaço em disco OK: $([math]::Round($freeSpace/1GB, 2))GB disponível" -ForegroundColor Green
}

# Função para verificar memória RAM
function Test-Memory {
    $requiredMemory = 4GB
    $totalMemory = (Get-WmiObject -Class Win32_ComputerSystem).TotalPhysicalMemory

    if ($totalMemory -lt $requiredMemory) {
        throw "Memória RAM insuficiente. Necessário: $($requiredMemory/1GB)GB, Disponível: $([math]::Round($totalMemory/1GB, 2))GB"
    }

    Write-Host "✓ Memória RAM OK: $([math]::Round($totalMemory/1GB, 2))GB" -ForegroundColor Green
}

# Função para verificar privilégios de administrador
function Test-AdminPrivileges {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)

    if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw "Este script requer privilégios de administrador"
    }

    Write-Host "✓ Privilégios de administrador OK" -ForegroundColor Green
}

# Função para verificar conexão com a internet
function Test-InternetConnection {
    $testUrls = @(
        "https://github.com",
        "https://chocolatey.org",
        "https://cmake.org"
    )

    foreach ($url in $testUrls) {
        try {
            $response = Invoke-WebRequest -Uri $url -UseBasicParsing -TimeoutSec 5
            if ($response.StatusCode -ne 200) {
                throw "Falha ao acessar $url"
            }
        }
        catch {
            throw "Sem conexão com a internet ou falha ao acessar $url"
        }
    }

    Write-Host "✓ Conexão com a internet OK" -ForegroundColor Green
}

# Função para verificar Visual Studio
function Test-VisualStudio {
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

    if (-not (Test-Path $vsWhere)) {
        Write-Host "! Visual Studio não encontrado - será instalado durante a configuração" -ForegroundColor Yellow
        return
    }

    $vsInstallation = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vsInstallation) {
        Write-Host "! Visual Studio C++ Workload não encontrado - será instalado durante a configuração" -ForegroundColor Yellow
        return
    }

    Write-Host "✓ Visual Studio OK" -ForegroundColor Green
}

# Função principal de verificação
function Main {
    Write-Host "Verificando requisitos do sistema..." -ForegroundColor Cyan
    Write-Host "======================================" -ForegroundColor Cyan

    try {
        Test-AdminPrivileges
        Test-WindowsVersion
        Test-DiskSpace
        Test-Memory
        Test-InternetConnection
        Test-VisualStudio

        Write-Host "======================================" -ForegroundColor Cyan
        Write-Host "✓ Todos os requisitos básicos atendidos!" -ForegroundColor Green
        return $true
    }
    catch {
        Write-Host "======================================" -ForegroundColor Cyan
        Write-Host "✗ Falha na verificação de requisitos:" -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        return $false
    }
}

# Executar verificação
$result = Main
if (-not $result) {
    exit 1
}
