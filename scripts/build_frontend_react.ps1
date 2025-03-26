# Script de build para o frontend React
param(
    [ValidateSet('Release', 'Debug')]
    [string]$BuildType = 'Release',
    [switch]$Clean,
    [switch]$Rebuild
)

# Importar funções utilitárias comuns
. "$PSScriptRoot\build_utils.ps1"

# Configurar diretórios
$frontendDir = Join-Path $rootDir "src\frontend\react"
$buildDir = Join-Path $rootDir "build\frontend\react"
$distDir = Join-Path $rootDir "dist\frontend\react"

# Função para limpar diretórios
function Clean-Frontend {
    Write-Host "🧹 Limpando diretórios do frontend React..."
    if (Test-Path $buildDir) {
        Remove-Item -Recurse -Force $buildDir
    }
    if (Test-Path $distDir) {
        Remove-Item -Recurse -Force $distDir
    }
    if (Test-Path (Join-Path $frontendDir "node_modules")) {
        Remove-Item -Recurse -Force (Join-Path $frontendDir "node_modules")
    }
    if (Test-Path (Join-Path $frontendDir "build")) {
        Remove-Item -Recurse -Force (Join-Path $frontendDir "build")
    }
}

# Função para verificar dependências
function Check-Dependencies {
    Write-Host "🔍 Verificando dependências do frontend React..."

    # Verificar Node.js
    if (-not (Get-Command "node" -ErrorAction SilentlyContinue)) {
        throw "Node.js não encontrado. Por favor, instale Node.js para compilar o frontend React."
    }

    # Verificar npm
    if (-not (Get-Command "npm" -ErrorAction SilentlyContinue)) {
        throw "npm não encontrado. Por favor, instale npm para compilar o frontend React."
    }
}

# Função principal de build
function Build-Frontend {
    Write-Host "🏗️ Iniciando build do frontend React..."

    # Criar diretórios
    New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
    New-Item -ItemType Directory -Force -Path $distDir | Out-Null

    # Instalar dependências
    Write-Host "📦 Instalando dependências..."
    Push-Location $frontendDir
    if ($BuildType -eq 'Release') {
        $env:NODE_ENV = 'production'
        npm ci
    }
    else {
        npm install
    }

    # Build
    Write-Host "🔨 Compilando frontend React..."
    if ($BuildType -eq 'Release') {
        npm run build
    }
    else {
        npm run build:dev
    }

    # Copiar build para diretório de distribuição
    Write-Host "📋 Copiando build para diretório de distribuição..."
    Copy-Item -Path "build\*" -Destination $distDir -Recurse -Force

    Pop-Location
}

try {
    # Verificar se é rebuild
    if ($Rebuild) {
        $Clean = $true
    }

    # Limpar se necessário
    if ($Clean) {
        Clean-Frontend
    }

    # Verificar dependências
    Check-Dependencies

    # Executar build
    Build-Frontend

    Write-Host "✅ Build do frontend React concluído com sucesso!" -ForegroundColor Green
}
catch {
    Write-Host "❌ Erro durante o build do frontend React: $_" -ForegroundColor Red
    exit 1
}
