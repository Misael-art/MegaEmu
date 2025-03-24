# Script de Instalacao Principal para Mega_Emu
# Autor: Assistant AI
# Data: 2024
# Descricao: Script principal que coordena a instalacao e configuracao do ambiente

#Requires -RunAsAdministrator

[CmdletBinding()]
param(
    [Parameter()]
    [switch]$Help,

    [Parameter()]
    [switch]$SkipRequirements,

    [Parameter()]
    [switch]$SkipVcpkg,

    [Parameter()]
    [switch]$SkipDependencies,

    [Parameter()]
    [switch]$SkipBuild,

    [Parameter()]
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$BuildType = "Release",

    [Parameter()]
    [string]$VcpkgRoot = "$env:USERPROFILE\vcpkg"
)

$ErrorActionPreference = "Stop"
$ScriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path

function Test-Administrator {
    $user = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($user)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Show-AdminError {
    $currentFile = $MyInvocation.MyCommand.Path
    $message = @"

ERRO: Este script requer privilegios de administrador!

Para executar o script:
1) Abra o PowerShell como Administrador:
   - Clique com o botao direito no PowerShell
   - Selecione 'Executar como administrador'

2) Navegue ate a pasta do projeto:
   cd 'D:\Steamapps\DevProjetos\PC Engines Projects\Mega_Emu'

3) Execute o script novamente:
   .\scripts\utils\install.ps1

"@
    Write-Host $message -ForegroundColor Red
    exit 1
}

function Show-Banner {
    $banner = @'
+===========================================+
|             MEGA EMU INSTALLER           |
|                                         |
|        Instalador e Configurador        |
|      Ambiente de Desenvolvimento        |
+===========================================+
'@
    Write-Host $banner -ForegroundColor Cyan
}

function Show-Help {
    $help = @'
Uso: .\install.ps1 [opcoes]

IMPORTANTE: Este script deve ser executado como Administrador!

Opcoes:
    -Help               Exibe esta ajuda
    -SkipRequirements   Pula a verificacao de requisitos do sistema
    -SkipVcpkg         Pula a instalacao/atualizacao do vcpkg
    -SkipDependencies  Pula a instalacao de dependencias
    -SkipBuild         Pula a compilacao do projeto
    -BuildType         Tipo de build (Debug/Release/RelWithDebInfo/MinSizeRel)
    -VcpkgRoot         Caminho para instalacao do vcpkg

Exemplo:
    .\install.ps1 -BuildType Debug -VcpkgRoot "D:\vcpkg"
'@
    Write-Host $help
    exit 0
}

# Verificar privilegios de administrador
if (-not (Test-Administrator)) {
    Show-AdminError
}

# Verificar se o parametro Help foi fornecido
if ($Help) {
    Show-Help
}

function Main {
    Show-Banner

    Write-Host "`nIniciando processo de instalacao..." -ForegroundColor Cyan
    Write-Host "Configuracoes:"
    Write-Host "  Build Type: $BuildType"
    Write-Host "  Vcpkg Path: $VcpkgRoot"
    Write-Host ""

    try {
        if (-not $SkipRequirements) {
            Write-Host "Verificando requisitos do sistema..." -ForegroundColor Cyan
            $requirementsScript = Join-Path $ScriptPath "check_requirements.ps1"
            & $requirementsScript
            if ($LASTEXITCODE -ne 0) {
                throw "Falha na verificacao de requisitos"
            }
        }

        Write-Host "`nConfigurando ambiente de desenvolvimento..." -ForegroundColor Cyan
        $setupScript = Join-Path $ScriptPath "setup_environment.ps1"
        & $setupScript `
            -SkipVcpkg:$SkipVcpkg `
            -SkipDependencies:$SkipDependencies `
            -SkipBuild:$SkipBuild `
            -BuildType $BuildType `
            -VcpkgRoot $VcpkgRoot

        if ($LASTEXITCODE -ne 0) {
            throw "Falha na configuracao do ambiente"
        }

        Write-Host "`nInstalacao concluida com sucesso!" -ForegroundColor Green

        $instructions = @'

Proximos passos:
1) Abrir o Visual Studio Code ou IDE preferida
2) Abrir a pasta do projeto
3) Selecionar o kit de ferramentas do Visual Studio no CMake
4) Compilar o projeto usando o tipo de build '{0}'

Boa codificacao!
'@ -f $BuildType

        Write-Host $instructions -ForegroundColor Green
    }
    catch {
        Write-Host "`nErro durante a instalacao:" -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        Write-Host "`nPara mais detalhes, execute o script com -Verbose" -ForegroundColor Yellow
        exit 1
    }
}

# Executar script principal
if (-not $Help) {
    Main
}
