# Script para lançar o frontend do Mega Emu no modo desktop (Electron)
param (
    [switch]$dev,
    [switch]$build
)

# Definir caminhos
$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
$rootPath = Split-Path -Parent (Split-Path -Parent $scriptPath)
$frontendPath = Join-Path -Path $rootPath -ChildPath "frontend"

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "    Mega Emu - Modo Desktop (Electron)" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# Verificar argumentos
if (-not $dev -and -not $build) {
    Write-Host "Uso: .\launch_electron.ps1 [-dev] [-build]" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Opções:"
    Write-Host "  -dev    : Iniciar o Electron em modo de desenvolvimento (hot reload)" -ForegroundColor Green
    Write-Host "  -build  : Construir uma versão portátil do aplicativo" -ForegroundColor Blue
    Write-Host ""
    $dev = $true  # Usar modo dev como padrão se nenhuma opção for especificada
}

# Verificar se a pasta do frontend existe
if (-not (Test-Path $frontendPath)) {
    Write-Host "Erro: Diretório do frontend não encontrado em $frontendPath" -ForegroundColor Red
    exit 1
}

# Navegar para o diretório do frontend
Write-Host "Navegando para o diretório do frontend..." -ForegroundColor Yellow
Set-Location -Path $frontendPath

# Verificar dependências
Write-Host "Verificando o Node.js e npm..." -ForegroundColor Yellow
try {
    $nodeVersion = node --version
    $npmVersion = npm --version
    Write-Host "Node.js $nodeVersion e npm $npmVersion encontrados." -ForegroundColor Green
}
catch {
    Write-Host "Erro: Node.js não encontrado. Por favor, instale o Node.js." -ForegroundColor Red
    exit 1
}

# Verificar se o Electron está instalado
Write-Host "Verificando as dependências do Electron..." -ForegroundColor Yellow
if (-not (Test-Path "$frontendPath\node_modules\electron")) {
    Write-Host "Instalando dependências (isso pode levar alguns minutos)..." -ForegroundColor Yellow
    try {
        npm install
    }
    catch {
        Write-Host "Erro ao instalar dependências." -ForegroundColor Red
        exit 1
    }
}

# Executar o Electron em modo de desenvolvimento ou construir uma versão distribuível
if ($dev) {
    Write-Host "Iniciando o Electron em modo de desenvolvimento..." -ForegroundColor Green
    Write-Host "Aguarde enquanto o ambiente está sendo preparado..." -ForegroundColor Yellow

    try {
        npm run electron:dev
    }
    catch {
        Write-Host "Erro ao iniciar o Electron em modo de desenvolvimento." -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        exit 1
    }
}
elseif ($build) {
    Write-Host "Construindo o aplicativo Electron para distribuição..." -ForegroundColor Blue
    Write-Host "Isso pode levar alguns minutos, por favor aguarde..." -ForegroundColor Yellow

    try {
        npm run electron:build

        # Verificar se a build foi bem-sucedida
        $distPath = Join-Path -Path $frontendPath -ChildPath "dist"
        if (Test-Path $distPath) {
            $exePath = Get-ChildItem -Path $distPath -Filter "*.exe" -Recurse | Select-Object -First 1 -ExpandProperty FullName

            if ($exePath) {
                Write-Host "Build concluída com sucesso!" -ForegroundColor Green
                Write-Host "Aplicativo gerado em: $exePath" -ForegroundColor Green

                # Perguntar se deseja executar o aplicativo
                $runApp = Read-Host "Deseja executar o aplicativo agora? (S/N)"
                if ($runApp -eq "S" -or $runApp -eq "s") {
                    Start-Process $exePath
                }
            }
            else {
                Write-Host "Aviso: Build concluída, mas não foi encontrado o executável." -ForegroundColor Yellow
            }
        }
        else {
            Write-Host "Aviso: Build concluída, mas o diretório de distribuição não foi encontrado." -ForegroundColor Yellow
        }
    }
    catch {
        Write-Host "Erro ao construir o aplicativo Electron." -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        exit 1
    }
}

Write-Host "Script de lançamento concluído." -ForegroundColor Cyan
