# Script para configurar o vcpkg
param(
    [switch]$Force,
    [switch]$Update
)

# Verificar se Git está instalado
if (-not (Get-Command "git" -ErrorAction SilentlyContinue)) {
    Write-Host "Git não encontrado. Por favor, instale o Git primeiro."
    exit 1
}

# Função para clonar ou atualizar vcpkg
function Initialize-Vcpkg {
    if (-not (Test-Path "vcpkg")) {
        Write-Host "Clonando vcpkg..."
        git clone https://github.com/Microsoft/vcpkg.git
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Erro ao clonar vcpkg"
            exit 1
        }
    }
    elseif ($Update) {
        Write-Host "Atualizando vcpkg..."
        Push-Location vcpkg
        git pull
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Erro ao atualizar vcpkg"
            Pop-Location
            exit 1
        }
        Pop-Location
    }
}

# Função para executar bootstrap do vcpkg
function Start-VcpkgBootstrap {
    Push-Location vcpkg
    Write-Host "Executando bootstrap do vcpkg..."
    .\bootstrap-vcpkg.bat
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Erro durante o bootstrap do vcpkg"
        Pop-Location
        exit 1
    }
    Pop-Location
}

# Função para criar manifest do vcpkg
function New-VcpkgManifest {
    $manifestPath = "vcpkg.json"

    if ((Test-Path $manifestPath) -and -not $Force) {
        Write-Host "Arquivo vcpkg.json já existe. Use -Force para sobrescrever."
        return
    }

    $manifest = @{
        name         = "mega-emu"
        version      = "0.1.0"
        description  = "Multi-platform emulator"
        dependencies = @(
            @{
                name    = "sdl2"
                version = ">= 2.28.3"
            }
            @{
                name    = "qt5-base"
                version = ">= 5.15.10"
            }
            @{
                name    = "opengl"
                version = ">= 2022.12.01"
            }
            @{
                name    = "glew"
                version = ">= 2.2.0"
            }
            @{
                name    = "zlib"
                version = ">= 1.3"
            }
            @{
                name    = "libpng"
                version = ">= 1.6.40"
            }
            @{
                name    = "libjpeg-turbo"
                version = ">= 3.0.0"
            }
            @{
                name    = "freetype"
                version = ">= 2.12.1"
            }
        )
    }

    Write-Host "Criando vcpkg.json..."
    $manifest | ConvertTo-Json -Depth 10 | Set-Content $manifestPath
}

# Função para configurar variáveis de ambiente
function Set-VcpkgEnvironment {
    $vcpkgRoot = (Resolve-Path "vcpkg").Path

    Write-Host "Configurando variável de ambiente VCPKG_ROOT..."
    [System.Environment]::SetEnvironmentVariable("VCPKG_ROOT", $vcpkgRoot, "User")

    Write-Host "Configurando variável de ambiente VCPKG_DEFAULT_TRIPLET..."
    [System.Environment]::SetEnvironmentVariable("VCPKG_DEFAULT_TRIPLET", "x64-windows", "User")

    # Adicionar vcpkg ao PATH se não estiver
    $userPath = [System.Environment]::GetEnvironmentVariable("PATH", "User")
    if (-not $userPath.Contains($vcpkgRoot)) {
        Write-Host "Adicionando vcpkg ao PATH..."
        [System.Environment]::SetEnvironmentVariable("PATH", "$userPath;$vcpkgRoot", "User")
    }
}

# Função para instalar dependências básicas
function Install-VcpkgDependencies {
    Push-Location vcpkg
    Write-Host "Instalando dependências básicas..."

    $dependencies = @(
        "sdl2:x64-windows",
        "qt5-base:x64-windows",
        "opengl:x64-windows",
        "glew:x64-windows",
        "zlib:x64-windows",
        "libpng:x64-windows",
        "libjpeg-turbo:x64-windows",
        "freetype:x64-windows"
    )

    foreach ($dep in $dependencies) {
        Write-Host "Instalando $dep..."
        .\vcpkg.exe install $dep
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Erro ao instalar $dep"
            Pop-Location
            exit 1
        }
    }

    Pop-Location
}

# Executar configuração
Write-Host "Iniciando configuração do vcpkg..."

Initialize-Vcpkg
Start-VcpkgBootstrap
New-VcpkgManifest
Set-VcpkgEnvironment
Install-VcpkgDependencies

Write-Host "`nConfiguração do vcpkg concluída!"
Write-Host "Para usar em um novo projeto CMake, adicione:"
Write-Host 'set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")'
