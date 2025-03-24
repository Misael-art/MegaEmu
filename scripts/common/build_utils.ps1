# Funções comuns para scripts de build

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

    # Procurar o vcvarsall.bat
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

# Função para verificar e instalar dependências
function Install-BuildDependencies {
    # Verificar e instalar CMake
    if (-not (Test-Command "cmake")) {
        Write-ColorMessage "CMake não encontrado. Instalando via winget..." "Yellow"
        winget install Kitware.CMake
        if ($LASTEXITCODE -ne 0) {
            Write-ColorMessage "Falha ao instalar CMake" "Red"
            return $false
        }
        # Recarregar PATH
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
    }

    # Verificar e instalar Ninja
    if (-not (Test-Command "ninja")) {
        Write-ColorMessage "Ninja não encontrado. Instalando via winget..." "Yellow"
        winget install Ninja-build.Ninja
        if ($LASTEXITCODE -ne 0) {
            Write-ColorMessage "Falha ao instalar Ninja" "Red"
            return $false
        }
        # Recarregar PATH
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
    }

    return $true
}

# Função para configurar vcpkg
function Initialize-Vcpkg {
    param([switch]$ForceUpdate)

    if (-not (Test-Path Env:VCPKG_ROOT) -or $ForceUpdate) {
        Write-ColorMessage "Configurando vcpkg..." "Yellow"

        # Verificar se Git está instalado
        if (-not (Test-Command "git")) {
            Write-ColorMessage "Git não encontrado. Instalando via winget..." "Yellow"
            winget install --id Git.Git -e --source winget
            if ($LASTEXITCODE -ne 0) {
                Write-ColorMessage "Falha ao instalar Git" "Red"
                return $false
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
                return $false
            }
        }

        # Atualizar vcpkg
        Push-Location $vcpkgPath
        git pull
        if ($LASTEXITCODE -ne 0) {
            Write-ColorMessage "Falha ao atualizar vcpkg" "Red"
            Pop-Location
            return $false
        }

        # Bootstrap vcpkg
        Write-ColorMessage "Executando bootstrap do vcpkg..." "Yellow"
        .\bootstrap-vcpkg.bat
        if ($LASTEXITCODE -ne 0) {
            Write-ColorMessage "Falha no bootstrap do vcpkg" "Red"
            Pop-Location
            return $false
        }

        # Integrar com Visual Studio
        .\vcpkg integrate install
        if ($LASTEXITCODE -ne 0) {
            Write-ColorMessage "Falha ao integrar vcpkg com Visual Studio" "Red"
            Pop-Location
            return $false
        }

        Pop-Location

        # Configurar variável de ambiente VCPKG_ROOT
        [System.Environment]::SetEnvironmentVariable("VCPKG_ROOT", $vcpkgPath, [System.EnvironmentVariableTarget]::User)
        $env:VCPKG_ROOT = $vcpkgPath
    }

    return $true
}

# Função principal para build de componentes
function Build-Component {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ComponentName,
        [string]$BuildType = "Release",
        [string]$BuildDir = "build",
        [switch]$Clean = $false,
        [switch]$Rebuild = $false,
        [string[]]$CMakeOptions = @()
    )

    Write-ColorMessage "Iniciando build do componente: $ComponentName" "Cyan"

    # Verificar dependências
    if (-not (Install-BuildDependencies)) {
        Write-ColorMessage "Falha ao instalar dependências" "Red"
        return $false
    }

    # Configurar MSVC
    if (-not (Initialize-MSVCEnvironment)) {
        Write-ColorMessage "Falha ao configurar ambiente MSVC" "Red"
        return $false
    }

    # Configurar vcpkg
    if (-not (Initialize-Vcpkg)) {
        Write-ColorMessage "Falha ao configurar vcpkg" "Red"
        return $false
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

    # Configurar o CMake com o compilador MSVC
    $env:CC = "cl.exe"
    $env:CXX = "cl.exe"

    $cmakeArgs = @(
        "-G", "Visual Studio 16 2019",
        "-DCMAKE_TOOLCHAIN_FILE=`"$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake`"",
        "-DCMAKE_BUILD_TYPE=$BuildType"
    )
    $cmakeArgs += $CMakeOptions
    $cmakeArgs += ".."

    cmake $cmakeArgs

    if ($LASTEXITCODE -ne 0) {
        Write-ColorMessage "Falha na configuração do CMake" "Red"
        Pop-Location
        return $false
    }

    # Compilar
    Write-ColorMessage "Compilando projeto..." "Cyan"
    cmake --build . --config $BuildType

    if ($LASTEXITCODE -ne 0) {
        Write-ColorMessage "Falha na compilação" "Red"
        Pop-Location
        return $false
    }

    Pop-Location

    Write-ColorMessage "Build do componente $ComponentName concluído com sucesso!" "Green"
    return $true
}
