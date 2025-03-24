# Setup Environment Script para Mega_Emu
# Autor: Assistant AI
# Data: 2024
# Descrição: Script de configuração completa do ambiente para compilação do Mega_Emu

#Requires -RunAsAdministrator
[CmdletBinding()]
param(
    [switch]$SkipVcpkg,
    [switch]$SkipDependencies,
    [switch]$SkipBuild,
    [string]$BuildType = "Release",
    [string]$VcpkgRoot = "$env:USERPROFILE\vcpkg"
)

# Função para verificar se um comando existe
function Test-Command {
    param($Command)
    try { Get-Command $Command -ErrorAction Stop; return $true }
    catch { return $false }
}

# Função para exibir mensagens coloridas
function Write-ColorMessage {
    param(
        [Parameter(Mandatory=$true)]
        [string]$Message,
        [string]$Color = "White"
    )
    Write-Host "==> $Message" -ForegroundColor $Color
}

# Função para verificar e instalar o Chocolatey
function Install-ChocolateyIfNeeded {
    if (-not (Test-Command "choco")) {
        Write-ColorMessage "Instalando Chocolatey..." "Yellow"
        Set-ExecutionPolicy Bypass -Scope Process -Force
        [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
        Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
        refreshenv
    }
}

# Função para verificar e instalar dependências via Chocolatey
function Install-RequiredTools {
    Write-ColorMessage "Verificando e instalando ferramentas necessárias..." "Cyan"

    $tools = @(
        @{Name = "cmake"; Version = "3.15.0"},
        @{Name = "git"; Version = ""},
        @{Name = "visualstudio2022buildtools"; Version = ""},
        @{Name = "python3"; Version = ""},
        @{Name = "ninja"; Version = ""}
    )

    foreach ($tool in $tools) {
        if (-not (Test-Command $tool.Name)) {
            $installCmd = "choco install $($tool.Name)"
            if ($tool.Version) {
                $installCmd += " --version $($tool.Version)"
            }
            $installCmd += " -y"
            Write-ColorMessage "Instalando $($tool.Name)..." "Yellow"
            Invoke-Expression $installCmd
        }
        else {
            Write-ColorMessage "$($tool.Name) já está instalado." "Green"
        }
    }
    refreshenv
}

# Função para configurar vcpkg
function Setup-Vcpkg {
    if ($SkipVcpkg) {
        Write-ColorMessage "Pulando instalação do vcpkg..." "Yellow"
        return
    }

    if (-not (Test-Path $VcpkgRoot)) {
        Write-ColorMessage "Clonando vcpkg..." "Cyan"
        git clone https://github.com/Microsoft/vcpkg.git $VcpkgRoot
        Push-Location $VcpkgRoot
        .\bootstrap-vcpkg.bat
        .\vcpkg integrate install
        Pop-Location
    }

    Write-ColorMessage "Instalando dependências via vcpkg..." "Cyan"
    $dependencies = @(
        "sdl2:x64-windows",
        "sdl2-ttf:x64-windows",
        "zlib:x64-windows",
        "libpng:x64-windows",
        "opengl:x64-windows",
        "gtest:x64-windows",
        "doxygen:x64-windows"
    )

    foreach ($dep in $dependencies) {
        Write-ColorMessage "Instalando $dep..." "Yellow"
        & $VcpkgRoot\vcpkg install $dep
    }
}

# Função para configurar variáveis de ambiente
function Setup-Environment {
    Write-ColorMessage "Configurando variáveis de ambiente..." "Cyan"

    $envPath = [Environment]::GetEnvironmentVariable("Path", "Machine")
    $pathsToAdd = @(
        $VcpkgRoot,
        (Join-Path $VcpkgRoot "installed/x64-windows/bin")
    )

    foreach ($path in $pathsToAdd) {
        if ($envPath -notlike "*$path*") {
            $envPath += ";$path"
            [Environment]::SetEnvironmentVariable("Path", $envPath, "Machine")
        }
    }

    [Environment]::SetEnvironmentVariable("VCPKG_ROOT", $VcpkgRoot, "Machine")
}

# Função para compilar o projeto
function Build-Project {
    if ($SkipBuild) {
        Write-ColorMessage "Pulando compilação..." "Yellow"
        return
    }

    Write-ColorMessage "Configurando e compilando o projeto..." "Cyan"

    $buildDir = "build"
    if (Test-Path $buildDir) {
        Remove-Item $buildDir -Recurse -Force
    }

    $vcpkgToolchain = Join-Path $VcpkgRoot "scripts/buildsystems/vcpkg.cmake"

    # Configurar CMake
    cmake -B $buildDir -S . `
        -DCMAKE_BUILD_TYPE=$BuildType `
        -DCMAKE_TOOLCHAIN_FILE=$vcpkgToolchain `
        -DBUILD_MEGADRIVE=ON `
        -DBUILD_MASTERSYSTEM=ON `
        -DBUILD_NES=ON `
        -DBUILD_SNES=OFF `
        -G "Visual Studio 17 2022" -A x64

    # Compilar
    cmake --build $buildDir --config $BuildType --parallel
}

# Função principal
function Main {
    $ErrorActionPreference = "Stop"

    Write-ColorMessage "Iniciando configuração do ambiente Mega_Emu..." "Green"

    try {
        Install-ChocolateyIfNeeded
        Install-RequiredTools
        Setup-Vcpkg
        Setup-Environment
        Build-Project

        Write-ColorMessage "Configuração concluída com sucesso!" "Green"
    }
    catch {
        Write-ColorMessage "Erro durante a configuração: $_" "Red"
        Write-ColorMessage "Stack Trace: $($_.ScriptStackTrace)" "Red"
        exit 1
    }
}

# Executar script principal
Main
