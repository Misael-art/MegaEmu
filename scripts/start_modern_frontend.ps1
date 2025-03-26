# Script para iniciar o frontend moderno do Mega Emu
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "    Iniciando o Frontend do Mega Emu" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# Definir caminho do frontend
$frontendPath = "D:\Steamapps\DevProjetos\PC Engines Projects\Mega_Emu\frontend"

# Verificar se o diretório existe
if (-not (Test-Path $frontendPath)) {
    Write-Host "Erro: Diretório do frontend não encontrado em $frontendPath" -ForegroundColor Red
    Write-Host "Verifique se o caminho está correto." -ForegroundColor Red
    exit 1
}

Write-Host "Navegando para o diretório do frontend..." -ForegroundColor Yellow
Set-Location -Path $frontendPath

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

Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "    Iniciando servidor de desenvolvimento" -ForegroundColor Cyan
Write-Host "    Pressione Ctrl+C para encerrar" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# Iniciar o frontend
try {
    npm start
}
catch {
    Write-Host "Erro ao iniciar o servidor de desenvolvimento." -ForegroundColor Red
    exit 1
}
finally {
    Write-Host ""
    Write-Host "Frontend encerrado." -ForegroundColor Yellow
}
