# Script para verificar o ambiente de build
param(
    [switch]$InstallMissing,
    [switch]$Verbose
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

# Função para verificar versão do Visual Studio
function Test-VisualStudio {
    # Lista de possíveis locais do Visual Studio
    $vsLocations = @(
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\BuildTools",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Community",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Professional",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Enterprise",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Community",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Professional",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Enterprise"
    )

    # Primeiro tenta usar vswhere se disponível
    foreach ($vsWhere in $vsLocations | Where-Object { $_.EndsWith('vswhere.exe') }) {
        if (Test-Path $vsWhere) {
            $vsInstallation = & $vsWhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
            if ($vsInstallation) {
                $version = & $vsWhere -latest -property catalog_productDisplayVersion
                Write-Host "Visual Studio encontrado via vswhere: versão $version"
                return $true
            }
        }
    }

    # Se vswhere falhar, verifica diretamente os caminhos conhecidos
    foreach ($vsPath in $vsLocations | Where-Object { -not $_.EndsWith('vswhere.exe') }) {
        if (Test-Path $vsPath) {
            # Verifica se tem os componentes C++ necessários
            if (Test-Path "$vsPath\VC\Tools\MSVC") {
                $version = (Get-Item $vsPath).Name
                Write-Host "Visual Studio encontrado em caminho direto: $version"
                return $true
            }
        }
    }

    Write-Host "Visual Studio não encontrado ou sem componentes C++ instalados"
    return $false
}

# Função para verificar versão do CMake
function Test-CMake {
    if (Test-Command "cmake") {
        $version = (cmake --version).Split("`n")[0]
        Write-Host $version
        return $true
    }
    Write-Host "CMake não encontrado"
    return $false
}

# Função para verificar versão do Ninja
function Test-Ninja {
    if (Test-Command "ninja") {
        $version = (ninja --version)
        Write-Host "Ninja versão $version"
        return $true
    }
    Write-Host "Ninja não encontrado"
    return $false
}

# Função para verificar vcpkg
function Test-Vcpkg {
    # Lista de possíveis locais do vcpkg
    $vcpkgLocations = @(
        "vcpkg/vcpkg.exe",
        "C:\vcpkg\vcpkg.exe",
        "${env:ProgramFiles}\vcpkg\vcpkg.exe",
        "${env:LOCALAPPDATA}\vcpkg\vcpkg.exe",
        "${env:USERPROFILE}\vcpkg\vcpkg.exe"
    )

    foreach ($vcpkgPath in $vcpkgLocations) {
        if (Test-Path $vcpkgPath) {
            try {
                $version = & $vcpkgPath version 2>&1
                Write-Host "vcpkg encontrado em: $vcpkgPath"
                Write-Host $version
                # Adiciona o diretório do vcpkg ao PATH se ainda não estiver
                $vcpkgDir = Split-Path -Parent $vcpkgPath
                if ($env:PATH -notlike "*$vcpkgDir*") {
                    $env:PATH = "$vcpkgDir;$env:PATH"
                }
                return $true
            }
            catch {
                Write-Host "vcpkg encontrado em $vcpkgPath mas não pode ser executado"
            }
        }
    }

    Write-Host "vcpkg não encontrado"
    return $false
}

# Função para verificar Git
function Test-Git {
    if (Test-Command "git") {
        $version = (git --version)
        Write-Host $version
        return $true
    }
    Write-Host "Git não encontrado"
    return $false
}

# Função para instalar dependências via winget
function Install-Dependency {
    param(
        [string]$Name,
        [string]$WingetId
    )

    Write-Host "Instalando $Name..."
    winget install -e --id $WingetId --accept-source-agreements --accept-package-agreements
}

# Função para obter o caminho do Visual Studio
function Get-VisualStudioPath {
    $vsLocations = @(
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\BuildTools",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Community",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Professional",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Enterprise",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Community",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Professional",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Enterprise"
    )

    # Primeiro tenta usar vswhere
    foreach ($vsWhere in $vsLocations | Where-Object { $_.EndsWith('vswhere.exe') }) {
        if (Test-Path $vsWhere) {
            $vsInstallation = & $vsWhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
            if ($vsInstallation) {
                return $vsInstallation
            }
        }
    }

    # Se vswhere falhar, verifica diretamente os caminhos conhecidos
    foreach ($vsPath in $vsLocations | Where-Object { -not $_.EndsWith('vswhere.exe') }) {
        if (Test-Path $vsPath) {
            if (Test-Path "$vsPath\VC\Tools\MSVC") {
                return $vsPath
            }
        }
    }

    return $null
}

# Função para obter o caminho do vcpkg
function Get-VcpkgPath {
    $vcpkgLocations = @(
        "vcpkg/vcpkg.exe",
        "C:\vcpkg\vcpkg.exe",
        "${env:ProgramFiles}\vcpkg\vcpkg.exe",
        "${env:LOCALAPPDATA}\vcpkg\vcpkg.exe",
        "${env:USERPROFILE}\vcpkg\vcpkg.exe"
    )

    foreach ($vcpkgPath in $vcpkgLocations) {
        if (Test-Path $vcpkgPath) {
            try {
                $version = & $vcpkgPath version 2>&1
                return $vcpkgPath
            }
            catch {
                continue
            }
        }
    }

    return $null
}

# Array para armazenar status
$status = @{
    "Visual Studio" = $false
    "CMake"         = $false
    "Ninja"         = $false
    "vcpkg"         = $false
    "Git"           = $false
}

# Verificar cada ferramenta
Write-Host "`nVerificando ambiente de build...`n"

Write-Host "Verificando Visual Studio..."
$status["Visual Studio"] = Test-VisualStudio

Write-Host "`nVerificando CMake..."
$status["CMake"] = Test-CMake

Write-Host "`nVerificando Ninja..."
$status["Ninja"] = Test-Ninja

Write-Host "`nVerificando vcpkg..."
$status["vcpkg"] = Test-Vcpkg

Write-Host "`nVerificando Git..."
$status["Git"] = Test-Git

# Instalar dependências ausentes se solicitado
if ($InstallMissing) {
    if (-not $status["Visual Studio"]) {
        Install-Dependency "Visual Studio Build Tools" "Microsoft.VisualStudio.2022.BuildTools"
    }
    if (-not $status["CMake"]) {
        Install-Dependency "CMake" "Kitware.CMake"
    }
    if (-not $status["Ninja"]) {
        Install-Dependency "Ninja" "Ninja-build.Ninja"
    }
    if (-not $status["Git"]) {
        Install-Dependency "Git" "Git.Git"
    }
    if (-not $status["vcpkg"]) {
        if (Test-Command "git") {
            Write-Host "`nClonando vcpkg..."
            git clone https://github.com/Microsoft/vcpkg.git
            Push-Location vcpkg
            .\bootstrap-vcpkg.bat
            Pop-Location
        }
        else {
            Write-Host "Git é necessário para instalar vcpkg"
        }
    }
}

# Exibir resumo
Write-Host "`nResumo do ambiente:"
Write-Host "===================="
foreach ($item in $status.GetEnumerator()) {
    $statusText = if ($item.Value) { "OK" } else { "Não encontrado" }
    Write-Host "$($item.Name): $statusText"
}

# Verificar se todas as dependências estão presentes
$allPresent = $true
foreach ($value in $status.Values) {
    if (-not $value) {
        $allPresent = $false
        break
    }
}

# Exibir mensagem final
Write-Host "`nStatus final:"
if ($allPresent) {
    Write-Host "Ambiente configurado corretamente!"
}
else {
    Write-Host "Algumas dependências estão faltando."
    Write-Host "Execute o script com -InstallMissing para instalar as dependências ausentes."
}

# Exibir informações adicionais se verbose
if ($Verbose) {
    Write-Host "`nInformações do sistema:"
    Write-Host "====================="
    Write-Host "Sistema operacional: $([System.Environment]::OSVersion.VersionString)"
    Write-Host "Arquitetura: $([System.Environment]::GetEnvironmentVariable("PROCESSOR_ARCHITECTURE"))"
    Write-Host "Número de processadores: $([System.Environment]::ProcessorCount)"
    Write-Host "Memória total: $([math]::Round((Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory / 1GB, 2)) GB"

    # Exibir caminhos encontrados
    Write-Host "`nCaminhos das ferramentas:"
    Write-Host "======================="
    if ($status["Visual Studio"]) {
        Write-Host "Visual Studio: $(Get-VisualStudioPath)"
    }
    if ($status["vcpkg"]) {
        Write-Host "vcpkg: $(Get-VcpkgPath)"
    }
}
