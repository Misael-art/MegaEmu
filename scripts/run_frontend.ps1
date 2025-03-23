# Script para iniciar o frontend moderno do Mega Emu
param (
    [switch]$dev,
    [switch]$prod,
    [switch]$build
)

$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
$rootPath = Split-Path -Parent $scriptPath
$buildScriptsPath = Join-Path -Path $rootPath -ChildPath "scripts\build"

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "    Mega Emu Frontend Runner" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# Opções de uso
if (-not $dev -and -not $prod -and -not $build) {
    Write-Host "Uso: .\run_frontend.ps1 [-dev] [-prod] [-build]" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Opções:"
    Write-Host "  -dev    : Inicia o servidor de desenvolvimento (modo padrão)" -ForegroundColor Green
    Write-Host "  -prod   : Inicia o servidor de produção usando a build otimizada" -ForegroundColor Blue
    Write-Host "  -build  : Apenas compila o frontend para produção" -ForegroundColor Magenta
    Write-Host ""
    $dev = $true  # Usar modo dev como padrão
}

# Definir caminho do frontend
$frontendPath = Join-Path -Path $rootPath -ChildPath "frontend"

# Verificar se o diretório existe
if (-not (Test-Path $frontendPath)) {
    Write-Host "Erro: Diretório do frontend não encontrado em $frontendPath" -ForegroundColor Red
    Write-Host "Verifique se o caminho está correto." -ForegroundColor Red
    exit 1
}

# Navegar para o diretório do frontend
Write-Host "Navegando para o diretório do frontend..." -ForegroundColor Yellow
Set-Location -Path $frontendPath

# Verificar dependências
Write-Host ""
Write-Host "Verificando dependências..." -ForegroundColor Yellow
try {
    $npmVersion = npm --version
    Write-Host "npm versão $npmVersion encontrado." -ForegroundColor Green
}
catch {
    Write-Host "Erro: Node.js/npm não encontrado. Por favor, instale o Node.js." -ForegroundColor Red
    exit 1
}

# Verificar node_modules
Write-Host ""
Write-Host "Verificando node_modules..." -ForegroundColor Yellow
if (-not (Test-Path "$frontendPath\node_modules")) {
    Write-Host "Instalando dependências (isso pode levar alguns minutos)..." -ForegroundColor Yellow
    try {
        npm install
    }
    catch {
        Write-Host "Erro ao instalar dependências." -ForegroundColor Red
        exit 1
    }
}

# Executar a ação adequada com base nos parâmetros
if ($build -or $prod) {
    Write-Host ""
    Write-Host "Compilando o frontend para produção..." -ForegroundColor Cyan
    try {
        npm run build
        Write-Host "Compilação concluída com sucesso!" -ForegroundColor Green
    }
    catch {
        Write-Host "Erro ao compilar o frontend." -ForegroundColor Red
        exit 1
    }
}

if ($dev) {
    Write-Host ""
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host "    Iniciando servidor de desenvolvimento" -ForegroundColor Cyan
    Write-Host "    Pressione Ctrl+C para encerrar" -ForegroundColor Cyan
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host ""

    try {
        npm start
    }
    catch {
        Write-Host "Erro ao iniciar o servidor de desenvolvimento." -ForegroundColor Red
        exit 1
    }
}
elseif ($prod) {
    Write-Host ""
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host "    Iniciando servidor de produção" -ForegroundColor Cyan
    Write-Host "    Pressione Ctrl+C para encerrar" -ForegroundColor Cyan
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host ""

    # Verificar se o pacote serve está instalado
    $serveInstalled = npm list -g serve
    if ($serveInstalled -notmatch "serve@") {
        Write-Host "Instalando o pacote 'serve' globalmente..." -ForegroundColor Yellow
        npm install -g serve
    }

    try {
        # Iniciar o servidor usando a build de produção
        serve -s build
    }
    catch {
        Write-Host "Erro ao iniciar o servidor de produção." -ForegroundColor Red
        exit 1
    }
}

Write-Host ""
Write-Host "Frontend encerrado." -ForegroundColor Yellow
