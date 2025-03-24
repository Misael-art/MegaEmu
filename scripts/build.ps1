# Script de build para o Mega_Emu
param(
    [string]$BuildType = "Release",
    [string]$BuildDir = "build",
    [switch]$Clean = $false,
    [switch]$Rebuild = $false,
    [switch]$ForceVcpkgUpdate = $false
)

# Função para exibir mensagens coloridas
function Write-ColorMessage {
    param([string]$Message, [string]$Color = "White")
    Write-Host $Message -ForegroundColor $Color
}

# Função para verificar se um comando existe
function Test-Command {
    param([string]$Command)
    return [bool](Get-Command -Name $Command -ErrorAction SilentlyContinue)
}

# Função para configurar o ambiente MSVC
function Initialize-MSVCEnvironment {
    Write-ColorMessage "Configurando ambiente MSVC..." "Cyan"

    # Procurar o vcvarsall.bat
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vsWhere)) {
        Write-ColorMessage "Instalando Visual Studio Build Tools..." "Yellow"
        # Baixar o instalador
        $installerUrl = "https://aka.ms/vs/17/release/vs_buildtools.exe"
        $installerPath = "$env:TEMP\vs_buildtools.exe"

        try {
            Invoke-WebRequest -Uri $installerUrl -OutFile $installerPath
        }
        catch {
            Write-ColorMessage "Falha ao baixar o Visual Studio Build Tools" "Red"
            return $false
        }

        # Instalar Build Tools
        $process = Start-Process -FilePath $installerPath -ArgumentList `
            "--quiet", `
            "--wait", `
            "--norestart", `
            "--nocache", `
            "--add", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", `
            "--add", "Microsoft.VisualStudio.Component.Windows10SDK.19041" `
            -Wait -PassThru

        if ($process.ExitCode -ne 0) {
            Write-ColorMessage "Falha ao instalar o Visual Studio Build Tools" "Red"
            return $false
        }
    }

    # Procurar o vcvarsall.bat novamente
    $vsPath = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    $vcvarsallPath = Join-Path $vsPath "VC\Auxiliary\Build\vcvarsall.bat"

    if (-not (Test-Path $vcvarsallPath)) {
        Write-ColorMessage "vcvarsall.bat não encontrado" "Red"
        return $false
    }

    # Criar um arquivo temporário para armazenar as variáveis de ambiente
    $tempFile = [System.IO.Path]::GetTempFileName()

    # Executar vcvarsall.bat e capturar as variáveis de ambiente
    $null = cmd /c "`"$vcvarsallPath`" x64 && set > `"$tempFile`""

    # Ler e aplicar as variáveis de ambiente
    Get-Content $tempFile | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') {
            $varName = $matches[1]
            $varValue = $matches[2]
            [System.Environment]::SetEnvironmentVariable($varName, $varValue, [System.EnvironmentVariableTarget]::Process)
        }
    }

    # Limpar o arquivo temporário
    Remove-Item $tempFile

    # Verificar se o ambiente foi configurado corretamente
    if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
        Write-ColorMessage "Falha ao configurar o ambiente MSVC" "Red"
        return $false
    }

    Write-ColorMessage "Ambiente MSVC configurado com sucesso" "Green"
    return $true
}

# Verificar e instalar CMake se necessário
if (-not (Test-Command "cmake")) {
    Write-ColorMessage "CMake não encontrado. Instalando via winget..." "Yellow"
    winget install Kitware.CMake
    if ($LASTEXITCODE -ne 0) {
        Write-ColorMessage "Falha ao instalar CMake" "Red"
        exit 1
    }
    # Recarregar PATH
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
    Write-ColorMessage "CMake instalado com sucesso" "Green"
}

# Verificar e instalar Ninja se necessário
if (-not (Test-Command "ninja")) {
    Write-ColorMessage "Ninja não encontrado. Instalando via winget..." "Yellow"
    winget install Ninja-build.Ninja
    if ($LASTEXITCODE -ne 0) {
        Write-ColorMessage "Falha ao instalar Ninja" "Red"
        exit 1
    }
    # Recarregar PATH
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
    Write-ColorMessage "Ninja instalado com sucesso" "Green"
}

# Verificar se vcpkg está instalado e configurado
if (-not (Test-Path Env:VCPKG_ROOT) -or $ForceVcpkgUpdate) {
    Write-ColorMessage "Configurando vcpkg..." "Yellow"

    # Verificar se Git está instalado
    if (-not (Test-Command "git")) {
        Write-ColorMessage "Git não encontrado. Instalando via winget..." "Yellow"
        winget install --id Git.Git -e --source winget
        if ($LASTEXITCODE -ne 0) {
            Write-ColorMessage "Falha ao instalar Git" "Red"
            exit 1
        }
        # Recarregar PATH
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
    }

    # Configurar vcpkg
    $vcpkgPath = "C:\vcpkg"
    if (-not (Test-Path $vcpkgPath)) {
        Write-ColorMessage "Clonando vcpkg..." "Yellow"
        git clone https://github.com/Microsoft/vcpkg.git $vcpkgPath
        if ($LASTEXITCODE -ne 0) {
            Write-ColorMessage "Falha ao clonar vcpkg" "Red"
            exit 1
        }
    }

    # Atualizar vcpkg
    Push-Location $vcpkgPath
    git pull
    if ($LASTEXITCODE -ne 0) {
        Write-ColorMessage "Falha ao atualizar vcpkg" "Red"
        Pop-Location
        exit 1
    }

    # Bootstrap vcpkg
    Write-ColorMessage "Executando bootstrap do vcpkg..." "Yellow"
    .\bootstrap-vcpkg.bat
    if ($LASTEXITCODE -ne 0) {
        Write-ColorMessage "Falha no bootstrap do vcpkg" "Red"
        Pop-Location
        exit 1
    }

    # Integrar com Visual Studio
    .\vcpkg integrate install
    if ($LASTEXITCODE -ne 0) {
        Write-ColorMessage "Falha ao integrar vcpkg com Visual Studio" "Red"
        Pop-Location
        exit 1
    }

    Pop-Location

    # Configurar variável de ambiente VCPKG_ROOT
    [System.Environment]::SetEnvironmentVariable("VCPKG_ROOT", $vcpkgPath, [System.EnvironmentVariableTarget]::User)
    $env:VCPKG_ROOT = $vcpkgPath
}

# Verificar dependências do SDL2
Write-ColorMessage "Verificando dependências SDL2..." "Cyan"
& "$env:VCPKG_ROOT\vcpkg" install sdl2:x64-windows sdl2-ttf:x64-windows
if ($LASTEXITCODE -ne 0) {
    Write-ColorMessage "Falha ao instalar dependências SDL2" "Red"
    exit 1
}

# Limpar diretório de build se solicitado
if ($Clean -or $Rebuild) {
    Write-ColorMessage "Limpando diretório de build..." "Yellow"
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
    }
}

# Criar diretório de build
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Configurar CMake
Write-ColorMessage "Configurando CMake..." "Cyan"
Push-Location $BuildDir

# Configurar o ambiente MSVC
if (-not (Initialize-MSVCEnvironment)) {
    Write-ColorMessage "Falha ao configurar o ambiente MSVC" "Red"
    Pop-Location
    exit 1
}

# Verificar versão do CMake
$cmakeVersion = (cmake --version | Select-Object -First 1) -replace 'cmake version '
if ([version]$cmakeVersion -lt [version]"3.10.0") {
    Write-ColorMessage "Versão do CMake ($cmakeVersion) é muito antiga. Atualizando..." "Yellow"
    winget install Kitware.CMake
    if ($LASTEXITCODE -ne 0) {
        Write-ColorMessage "Falha ao atualizar CMake" "Red"
        Pop-Location
        exit 1
    }
    # Recarregar PATH
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
}

# Configurar o CMake com o compilador MSVC
$env:CC = "cl.exe"
$env:CXX = "cl.exe"

cmake -G "Visual Studio 16 2019" `
    -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
    -DCMAKE_BUILD_TYPE=$BuildType `
    -DUSE_SDL2=ON `
    -DBUILD_TESTS=ON `
    -DBUILD_NES=ON `
    -DBUILD_MEGADRIVE=ON `
    -DBUILD_MASTERSYSTEM=ON `
    ..

if ($LASTEXITCODE -ne 0) {
    Write-ColorMessage "Falha na configuração do CMake" "Red"
    Pop-Location
    exit 1
}

# Compilar
Write-ColorMessage "Compilando projeto..." "Cyan"
cmake --build . --config $BuildType

if ($LASTEXITCODE -ne 0) {
    Write-ColorMessage "Falha na compilação" "Red"
    Pop-Location
    exit 1
}

Pop-Location

Write-ColorMessage "Build concluído com sucesso!" "Green"
