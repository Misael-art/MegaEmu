# Script para atualizar as dependências
param(
    [switch]$Force,
    [switch]$NoPrompt,
    [switch]$UpdateVcpkg
)

# Função para verificar se um comando existe
function Test-Command {
    param([string]$Command)

    try {
        Get-Command $Command -ErrorAction Stop | Out-Null
        return $true
    }
    catch {
        return $false
    }
}

# Função para atualizar vcpkg
function Update-Vcpkg {
    if (-not (Test-Path "vcpkg")) {
        Write-Host "vcpkg não encontrado. Execute setup_vcpkg.ps1 primeiro."
        return $false
    }

    Push-Location vcpkg
    Write-Host "Atualizando vcpkg..."
    git pull
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Erro ao atualizar vcpkg"
        Pop-Location
        return $false
    }

    Write-Host "Executando bootstrap..."
    .\bootstrap-vcpkg.bat
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Erro durante o bootstrap do vcpkg"
        Pop-Location
        return $false
    }

    Pop-Location
    return $true
}

# Função para atualizar dependências do vcpkg
function Update-VcpkgDependencies {
    if (-not (Test-Path "vcpkg.json")) {
        Write-Host "vcpkg.json não encontrado."
        return $false
    }

    Write-Host "Atualizando dependências do vcpkg..."
    .\vcpkg\vcpkg.exe upgrade --no-dry-run
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Erro ao atualizar dependências do vcpkg"
        return $false
    }

    return $true
}

# Função para atualizar ferramentas de build
function Update-BuildTools {
    $tools = @(
        @{
            Name = "Visual Studio Build Tools"
            Id = "Microsoft.VisualStudio.2022.BuildTools"
        }
        @{
            Name = "CMake"
            Id = "Kitware.CMake"
        }
        @{
            Name = "Ninja"
            Id = "Ninja-build.Ninja"
        }
    )

    $success = $true
    foreach ($tool in $tools) {
        Write-Host "Verificando atualização para $($tool.Name)..."
        winget upgrade -e --id $tool.Id --accept-source-agreements
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Erro ao atualizar $($tool.Name)"
            $success = $false
        }
    }

    return $success
}

# Função para verificar atualizações do Git
function Update-Git {
    if (-not (Test-Command "git")) {
        Write-Host "Git não encontrado."
        return $false
    }

    Write-Host "Verificando atualização para Git..."
    winget upgrade -e --id Git.Git --accept-source-agreements
    return $LASTEXITCODE -eq 0
}

# Confirmar atualização
if (-not $NoPrompt) {
    $confirmation = Read-Host "Isso atualizará todas as dependências do projeto. Continuar? (S/N)"
    if ($confirmation -ne "S") {
        Write-Host "Operação cancelada."
        exit
    }
}

# Verificar se winget está disponível
if (-not (Test-Command "winget")) {
    Write-Host "winget não encontrado. Por favor, instale o App Installer do Microsoft Store."
    exit 1
}

# Status das atualizações
$status = @{
    "Git" = $false
    "Build Tools" = $false
    "vcpkg" = $false
    "Dependências" = $false
}

# Atualizar Git
Write-Host "`nAtualizando Git..."
$status["Git"] = Update-Git

# Atualizar ferramentas de build
Write-Host "`nAtualizando ferramentas de build..."
$status["Build Tools"] = Update-BuildTools

# Atualizar vcpkg se solicitado
if ($UpdateVcpkg) {
    Write-Host "`nAtualizando vcpkg..."
    $status["vcpkg"] = Update-Vcpkg
}

# Atualizar dependências
Write-Host "`nAtualizando dependências..."
$status["Dependências"] = Update-VcpkgDependencies

# Exibir resumo
Write-Host "`nResumo das atualizações:"
Write-Host "======================"
foreach ($item in $status.GetEnumerator()) {
    $statusText = if ($item.Value) { "OK" } else { "Falha" }
    Write-Host "$($item.Name): $statusText"
}

# Verificar se todas as atualizações foram bem-sucedidas
$allSuccess = $true
foreach ($value in $status.Values) {
    if (-not $value) {
        $allSuccess = $false
        break
    }
}

# Exibir próximos passos
Write-Host "`nPróximos passos:"
if ($allSuccess) {
    Write-Host "1. Execute 'clean_build.ps1' para limpar os arquivos de build"
    Write-Host "2. Recompile o projeto com 'build_all.ps1'"
    Write-Host "3. Execute os testes com 'run_tests.ps1'"
}
else {
    Write-Host "Algumas atualizações falharam. Verifique os erros acima."
    exit 1
}
