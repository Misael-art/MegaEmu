# Script para criar pacote de distribuição
param(
    [Parameter(Mandatory = $true)]
    [string]$Version,
    [string]$BuildDir = "build",
    [string]$OutputDir = "build/released"
)

# Função para verificar versão semântica
function Test-SemVer {
    param([string]$Version)

    if ($Version -match '^\d+\.\d+\.\d+(-[0-9A-Za-z-]+(\.[0-9A-Za-z-]+)*)?(\+[0-9A-Za-z-]+(\.[0-9A-Za-z-]+)*)?$') {
        return $true
    }
    return $false
}

# Função para criar diretório se não existir
function New-DirectoryIfNotExists {
    param([string]$Path)

    if (-not (Test-Path $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
        Write-Host "Criado diretório: $Path"
    }
}

# Verificar formato da versão
if (-not (Test-SemVer $Version)) {
    Write-Host "Versão inválida. Use o formato semântico (ex: 1.0.0, 1.0.0-beta.1)"
    exit 1
}

# Criar diretório de release
$releaseDir = Join-Path $OutputDir "release-$Version"
New-DirectoryIfNotExists $releaseDir

# Diretórios para cada componente
$dirs = @{
    "nes"          = Join-Path $releaseDir "emulators/nes"
    "megadrive"    = Join-Path $releaseDir "emulators/megadrive"
    "mastersystem" = Join-Path $releaseDir "emulators/mastersystem"
    "sdl"          = Join-Path $releaseDir "frontend/sdl"
    "qt"           = Join-Path $releaseDir "frontend/qt"
    "tools"        = Join-Path $releaseDir "tools"
    "docs"         = Join-Path $releaseDir "docs"
}

# Criar estrutura de diretórios
foreach ($dir in $dirs.Values) {
    New-DirectoryIfNotExists $dir
}

# Função para copiar binários e dependências
function Copy-BinaryAndDeps {
    param(
        [string]$Source,
        [string]$Destination
    )

    if (-not (Test-Path $Source)) {
        Write-Host "AVISO: Binário não encontrado: $Source"
        return $false
    }

    # Copiar binário principal
    Copy-Item $Source $Destination
    Write-Host "Copiado: $Source -> $Destination"

    # Copiar DLLs dependentes
    $dllDir = Split-Path -Parent $Source
    Get-ChildItem -Path $dllDir -Filter "*.dll" | ForEach-Object {
        $destFile = Join-Path $Destination $_.Name
        Copy-Item $_.FullName $destFile
        Write-Host "Copiada dependência: $($_.Name)"
    }

    return $true
}

# Copiar binários
$binaries = @{
    "nes"          = "$BuildDir/emulators/nes/nes_emu.exe"
    "megadrive"    = "$BuildDir/emulators/megadrive/megadrive_emu.exe"
    "mastersystem" = "$BuildDir/emulators/mastersystem/mastersystem_emu.exe"
    "sdl"          = "$BuildDir/frontend/sdl/sdl_frontend.exe"
    "qt"           = "$BuildDir/frontend/qt/qt_frontend.exe"
    "tools"        = "$BuildDir/Mega_tools/mega_tools.exe"
}

$success = $true
foreach ($component in $binaries.Keys) {
    $source = $binaries[$component]
    $destination = $dirs[$component]
    if (-not (Copy-BinaryAndDeps -Source $source -Destination $destination)) {
        $success = $false
    }
}

# Copiar documentação
Copy-Item "docs/generated/html/*" $dirs["docs"] -Recurse
Write-Host "Copiada documentação para: $($dirs['docs'])"

# Copiar arquivos adicionais
$additionalFiles = @(
    @{
        Source      = "README.md"
        Destination = Join-Path $releaseDir "README.md"
    }
    @{
        Source      = "LICENSE"
        Destination = Join-Path $releaseDir "LICENSE"
    }
    @{
        Source      = "CHANGELOG.md"
        Destination = Join-Path $releaseDir "CHANGELOG.md"
    }
)

foreach ($file in $additionalFiles) {
    if (Test-Path $file.Source) {
        Copy-Item $file.Source $file.Destination
        Write-Host "Copiado: $($file.Source) -> $($file.Destination)"
    }
    else {
        Write-Host "AVISO: Arquivo não encontrado: $($file.Source)"
    }
}

# Criar arquivo de versão
$versionInfo = @{
    version    = $Version
    buildDate  = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    components = @{}
}

foreach ($component in $binaries.Keys) {
    $binary = $binaries[$component]
    if (Test-Path $binary) {
        $hash = (Get-FileHash -Path $binary -Algorithm SHA256).Hash
        $versionInfo.components[$component] = @{
            path = $binary
            hash = $hash
        }
    }
}

$versionFile = Join-Path $releaseDir "version.json"
$versionInfo | ConvertTo-Json -Depth 10 | Set-Content $versionFile
Write-Host "Criado arquivo de versão: $versionFile"

# Criar arquivo ZIP
$zipFile = Join-Path $OutputDir "mega-emu-$Version.zip"
Compress-Archive -Path $releaseDir/* -DestinationPath $zipFile -Force
Write-Host "Criado arquivo ZIP: $zipFile"

# Exibir resultado
if ($success) {
    Write-Host "`nPacote de distribuição criado com sucesso!"
    Write-Host "Versão: $Version"
    Write-Host "Localização: $zipFile"
}
else {
    Write-Host "`nAVISO: Alguns componentes não foram encontrados."
    Write-Host "Verifique se todos os componentes foram compilados corretamente."
    exit 1
}
