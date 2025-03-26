# Script para criar a estrutura de diretórios de build
param(
    [string]$RootDir = "build"
)

# Função para criar diretório se não existir
function New-DirectoryIfNotExists {
    param([string]$Path)

    if (-not (Test-Path $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
        Write-Host "Criado diretório: $Path"
    }
}

# Estrutura de diretórios
$directories = @(
    "$RootDir/test",
    "$RootDir/temp",
    "$RootDir/emulators/nes",
    "$RootDir/emulators/megadrive",
    "$RootDir/emulators/mastersystem",
    "$RootDir/frontend/sdl",
    "$RootDir/frontend/qt",
    "$RootDir/Mega_tools",
    "$RootDir/released"
)

# Criar cada diretório
foreach ($dir in $directories) {
    New-DirectoryIfNotExists $dir
}

# Criar arquivo .gitkeep em diretórios vazios
foreach ($dir in $directories) {
    $gitkeep = Join-Path $dir ".gitkeep"
    if (-not (Test-Path $gitkeep)) {
        New-Item -ItemType File -Path $gitkeep -Force | Out-Null
        Write-Host "Criado .gitkeep em: $dir"
    }
}

Write-Host "`nEstrutura de diretórios criada com sucesso em: $RootDir"
Write-Host "Use 'tree $RootDir' para visualizar a estrutura completa"
