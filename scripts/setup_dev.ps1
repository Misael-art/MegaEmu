# Script para configurar o ambiente de desenvolvimento
param(
    [switch]$Force,
    [switch]$NoPrompt
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

# Função para instalar ferramenta via winget
function Install-Tool {
    param(
        [string]$Name,
        [string]$Id
    )

    Write-Host "Instalando $Name..."
    winget install -e --id $Id --accept-source-agreements --accept-package-agreements
    return $LASTEXITCODE -eq 0
}

# Função para criar diretório se não existir
function New-DirectoryIfNotExists {
    param([string]$Path)

    if (-not (Test-Path $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
        Write-Host "Criado diretório: $Path"
        return $true
    }
    return $false
}

# Confirmar configuração
if (-not $NoPrompt) {
    $confirmation = Read-Host "Isso configurará o ambiente de desenvolvimento. Continuar? (S/N)"
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

# Lista de ferramentas necessárias
$tools = @(
    @{
        Name     = "Visual Studio Build Tools"
        Id       = "Microsoft.VisualStudio.2022.BuildTools"
        Required = $true
    }
    @{
        Name     = "CMake"
        Id       = "Kitware.CMake"
        Required = $true
    }
    @{
        Name     = "Ninja"
        Id       = "Ninja-build.Ninja"
        Required = $true
    }
    @{
        Name     = "Git"
        Id       = "Git.Git"
        Required = $true
    }
    @{
        Name     = "Visual Studio Code"
        Id       = "Microsoft.VisualStudioCode"
        Required = $false
    }
    @{
        Name     = "Windows Terminal"
        Id       = "Microsoft.WindowsTerminal"
        Required = $false
    }
    @{
        Name     = "PowerShell 7"
        Id       = "Microsoft.PowerShell"
        Required = $false
    }
)

# Status da instalação
$status = @{}

# Instalar cada ferramenta
foreach ($tool in $tools) {
    if (-not (Test-Command $tool.Name.Replace(" ", "").ToLower())) {
        $status[$tool.Name] = Install-Tool -Name $tool.Name -Id $tool.Id
        if (-not $status[$tool.Name] -and $tool.Required) {
            Write-Host "Erro ao instalar $($tool.Name) (requerido)"
            exit 1
        }
    }
    else {
        $status[$tool.Name] = $true
        Write-Host "$($tool.Name) já está instalado"
    }
}

# Criar estrutura de diretórios
Write-Host "`nCriando estrutura de diretórios..."
.\create_build_structure.ps1

# Configurar vcpkg
Write-Host "`nConfigurando vcpkg..."
.\setup_vcpkg.ps1

# Configurar VS Code (se instalado)
if ($status["Visual Studio Code"]) {
    Write-Host "`nConfigurando Visual Studio Code..."

    # Extensões recomendadas
    $extensions = @(
        "ms-vscode.cpptools"
        "ms-vscode.cmake-tools"
        "twxs.cmake"
        "ms-vscode.powershell"
        "eamodio.gitlens"
        "streetsidesoftware.code-spell-checker"
        "ms-vscode.hexeditor"
        "ms-vscode.cpptools-themes"
    )

    foreach ($ext in $extensions) {
        Write-Host "Instalando extensão: $ext"
        code --install-extension $ext
    }

    # Criar diretório .vscode se não existir
    New-DirectoryIfNotExists ".vscode"

    # Configurar settings.json
    $settings = @{
        "cmake.configureOnOpen"               = $true
        "cmake.buildDirectory"                = "`${workspaceFolder}/build"
        "cmake.installPrefix"                 = "`${workspaceFolder}/install"
        "cmake.generator"                     = "Ninja"
        "cmake.configureSettings"             = @{
            "CMAKE_TOOLCHAIN_FILE" = "`${env:VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        }
        "C_Cpp.default.configurationProvider" = "ms-vscode.cmake-tools"
        "files.associations"                  = @{
            "*.h"   = "c"
            "*.c"   = "c"
            "*.hpp" = "cpp"
            "*.cpp" = "cpp"
        }
        "editor.formatOnSave"                 = $true
        "editor.rulers"                       = @(80, 120)
        "files.trimTrailingWhitespace"        = $true
        "files.insertFinalNewline"            = $true
    }

    $settingsPath = ".vscode/settings.json"
    $settings | ConvertTo-Json -Depth 10 | Set-Content $settingsPath
    Write-Host "Criado arquivo de configuração: $settingsPath"
}

# Exibir resumo
Write-Host "`nResumo da configuração:"
Write-Host "====================="
foreach ($item in $status.GetEnumerator()) {
    $statusText = if ($item.Value) { "OK" } else { "Falha" }
    Write-Host "$($item.Name): $statusText"
}

# Verificar se todas as instalações necessárias foram bem-sucedidas
$requiredSuccess = $true
foreach ($tool in $tools) {
    if ($tool.Required -and -not $status[$tool.Name]) {
        $requiredSuccess = $false
        break
    }
}

# Exibir próximos passos
Write-Host "`nPróximos passos:"
if ($requiredSuccess) {
    Write-Host "1. Reinicie seu terminal para aplicar as alterações de ambiente"
    Write-Host "2. Execute 'build_all.ps1' para compilar o projeto"
    Write-Host "3. Execute 'run_tests.ps1' para verificar se tudo está funcionando"
}
else {
    Write-Host "Algumas instalações requeridas falharam. Verifique os erros acima."
    exit 1
}
