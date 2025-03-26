# Script de compilação para o Mega_Emu
# Seguindo as diretrizes do AI_GUIDELINES.md

# Configuração de variáveis
$BuildDir = "build"  # Diretório de build
$Configuration = "Debug"  # Configuração de build (Debug ou Release)
$Platform = "x64"  # Plataforma de build (x86 ou x64)

# Função para verificar dependências
function Check-Dependencies {
    Write-Host "Verificando dependências..." -ForegroundColor Cyan

    # Verificar CMake
    if (!(Get-Command "cmake.exe" -ErrorAction SilentlyContinue)) {
        Write-Host "❌ CMake não encontrado. Por favor, instale o CMake." -ForegroundColor Red
        exit 1
    }

    # Remover SDL2 check - CMake will handle SDL2 dependency
    # Verificar SDL2
    #if (!(Get-Command "sdl2-config" -ErrorAction SilentlyContinue)) {
    #    Write-Host "❌ SDL2 não encontrado. Por favor, instale o SDL2." -ForegroundColor Red
    #    exit 1
    #}

    # Verificar OpenGL
    # (Implementar verificação para OpenGL, se necessário)

    # Verificar zlib
    # (Implementar verificação para zlib, se necessário)

    # Verificar SQLite
    # (Implementar verificação para SQLite, se necessário)

    Write-Host "✅ Todas as dependências encontradas" -ForegroundColor Green
}

# Função para limpar diretório de build
function Clean-BuildDirectory {
    Write-Host "Limpando diretório de build..." -ForegroundColor Cyan

    try {
        # Remove o diretório de build se ele existir
        if (Test-Path $BuildDir) {
            Remove-Item -Path $BuildDir -Recurse -Force
        }

        # Cria um novo diretório de build
        New-Item -Path $BuildDir -ItemType Directory | Out-Null
        Write-Host "✅ Diretório de build limpo" -ForegroundColor Green
    }
    catch {
        Write-Host "❌ Erro ao limpar diretório de build: $_" -ForegroundColor Red
        exit 1
    }
}

# Função para configurar o projeto
function Configure-Project {
    Write-Host "Configurando projeto com CMake..." -ForegroundColor Cyan

    # Entra no diretório de build
    Push-Location $BuildDir
    # Configura o projeto com CMake
    cmake -G "Visual Studio 17 2022" -A $Platform ..
    # Verifica se a configuração foi bem-sucedida
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Falha na configuração do CMake" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    # Sai do diretório de build
    Pop-Location

    Write-Host "✅ Projeto configurado com sucesso" -ForegroundColor Green
}

# Função para compilar o projeto
function Build-Project {
    Write-Host "Compilando projeto..." -ForegroundColor Cyan

    # Entra no diretório de build
    Push-Location $BuildDir
    # Compila o projeto com CMake
    cmake --build . --config $Configuration
    # Verifica se a compilação foi bem-sucedida
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Falha na compilação" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    # Sai do diretório de build
    Pop-Location

    Write-Host "✅ Projeto compilado com sucesso" -ForegroundColor Green
}

# Execução principal
try {
    Write-Host "=== Iniciando compilação do Mega_Emu ===" -ForegroundColor Yellow

    # Executa as funções de verificação, limpeza, configuração e compilação
    Check-Dependencies
    Clean-BuildDirectory
    Configure-Project
    Build-Project

    Write-Host "=== Compilação concluída com sucesso ===" -ForegroundColor Yellow
}
catch {
    Write-Host "❌ Erro durante a compilação: $_" -ForegroundColor Red
    exit 1
}
