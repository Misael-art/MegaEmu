# Script para corrigir o perfil do PowerShell
$ErrorActionPreference = "Stop"

function Write-Status {
    param($Message, $Type = "Info")

    $colors = @{
        "Info"    = "Cyan"
        "Success" = "Green"
        "Warning" = "Yellow"
        "Error"   = "Red"
    }

    Write-Host "[Profile Fix] $Message" -ForegroundColor $colors[$Type]
}

function Test-AdminPrivileges {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Initialize-PowerShellProfile {
    # Verificar privilegios de administrador
    if (-not (Test-AdminPrivileges)) {
        Write-Status "Este script requer privilegios de administrador!" "Error"
        exit 1
    }

    Write-Status "Iniciando correcao do perfil do PowerShell..."

    # Caminhos importantes
    $allProfilePaths = @(
        $PROFILE.AllUsersAllHosts, # Todos os usuarios, todos os hosts
        $PROFILE.AllUsersCurrentHost, # Todos os usuarios, host atual
        $PROFILE.CurrentUserAllHosts, # Usuario atual, todos os hosts
        $PROFILE.CurrentUserCurrentHost # Usuario atual, host atual
    )

    # Backup de todos os perfis existentes
    foreach ($profilePath in $allProfilePaths) {
        $profileDir = Split-Path -Parent $profilePath

        if (-not (Test-Path $profileDir)) {
            Write-Status "Criando diretorio: $profileDir"
            New-Item -ItemType Directory -Path $profileDir -Force | Out-Null
        }

        if (Test-Path $profilePath) {
            $backupPath = "$profilePath.bak_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
            Write-Status "Fazendo backup do perfil: $profilePath -> $backupPath"
            Copy-Item -Path $profilePath -Destination $backupPath -Force
            Remove-Item -Path $profilePath -Force
        }
    }

    # Verificar instalacao do Chocolatey
    $chocoInstalled = $false
    if (Test-Path "$env:ChocolateyInstall") {
        $chocoInstalled = $true
        Write-Status "Chocolatey encontrado em: $env:ChocolateyInstall"
    }

    # Criar novo perfil com configuracao robusta
    $newProfile = @'
# Perfil do PowerShell
# Gerado automaticamente por Mega_Emu installer

# Configurar codificacao para UTF-8 sem BOM
$OutputEncoding = [System.Text.UTF8Encoding]::new($false)
[System.Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)
[System.Console]::InputEncoding = [System.Text.UTF8Encoding]::new($false)

# Configuracoes basicas
$ErrorActionPreference = "Stop"
$ProgressPreference = 'SilentlyContinue'
$FormatEnumerationLimit = 100

# Funcao para importar modulos com seguranca
function Import-ModuleSafely {
    param(
        [string]$ModulePath,
        [string]$ModuleName
    )

    try {
        if (Test-Path $ModulePath) {
            Import-Module $ModulePath -ErrorAction Stop
            Write-Verbose "Modulo '$ModuleName' carregado com sucesso"
            return $true
        }
    }
    catch {
        Write-Warning "Erro ao carregar modulo '$ModuleName': $($_.Exception.Message)"
    }
    return $false
}

# Configuracao do Chocolatey
if ($env:ChocolateyInstall) {
    $chocoModule = Join-Path $env:ChocolateyInstall "helpers\chocolateyProfile.psm1"
    Import-ModuleSafely -ModulePath $chocoModule -ModuleName "Chocolatey"
}

# Aliases uteis
Set-Alias -Name which -Value Get-Command
Set-Alias -Name touch -Value New-Item

# Configurar ambiente para desenvolvimento
$env:POWERSHELL_TELEMETRY_OPTOUT = 1
$env:DOTNET_CLI_TELEMETRY_OPTOUT = 1

# Funcoes uteis
function Get-PathEnvironment { $env:Path -split ';' | Sort-Object }
function Test-AdminRole { ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator) }
'@

    # Aplicar o novo perfil para todos os usuarios
    $mainProfile = $PROFILE.CurrentUserCurrentHost
    Write-Status "Criando novo perfil em: $mainProfile"

    # Garantir que o arquivo seja salvo em UTF-8 sem BOM
    $utf8NoBom = New-Object System.Text.UTF8Encoding $false
    [System.IO.File]::WriteAllLines($mainProfile, $newProfile.Split("`n"), $utf8NoBom)

    # Criar links simbolicos para os outros perfis
    foreach ($profilePath in $allProfilePaths) {
        if ($profilePath -ne $mainProfile -and -not (Test-Path $profilePath)) {
            Write-Status "Criando link para: $profilePath"
            New-Item -ItemType SymbolicLink -Path $profilePath -Target $mainProfile -Force | Out-Null
        }
    }

    # Atualizar politica de execucao
    try {
        Set-ExecutionPolicy RemoteSigned -Scope CurrentUser -Force
        Write-Status "Politica de execucao atualizada para RemoteSigned" "Success"
    }
    catch {
        Write-Status "Nao foi possivel atualizar a politica de execucao: $_" "Warning"
    }

    # Limpar cache de modulos
    $modulesPath = "$env:USERPROFILE\Documents\WindowsPowerShell\Modules"
    if (Test-Path $modulesPath) {
        Write-Status "Limpando cache de modulos..."
        Remove-Item -Path $modulesPath\* -Recurse -Force -ErrorAction SilentlyContinue
    }

    Write-Status "Perfil do PowerShell corrigido com sucesso!" "Success"
    Write-Status "As alteracoes terao efeito na proxima vez que o PowerShell for iniciado." "Info"
}

# Executar a funcao principal
Initialize-PowerShellProfile
