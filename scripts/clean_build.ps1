# Script para limpar os diretórios de build
param(
    [string]$RootDir = "build",
    [switch]$KeepReleased,
    [switch]$Force
)

# Função para remover diretório mantendo .gitkeep
function Remove-DirectoryContents {
    param(
        [string]$Path,
        [switch]$KeepGitkeep
    )

    if (Test-Path $Path) {
        Get-ChildItem -Path $Path -Exclude ".gitkeep" | ForEach-Object {
            if ($_.PSIsContainer) {
                Remove-Item $_.FullName -Recurse -Force
                Write-Host "Removido diretório: $($_.FullName)"
            }
            else {
                Remove-Item $_.FullName -Force
                Write-Host "Removido arquivo: $($_.FullName)"
            }
        }

        if (-not $KeepGitkeep) {
            Remove-Item $Path -Recurse -Force
            Write-Host "Removido diretório: $Path"
        }
    }
}

# Confirmar limpeza se não forçado
if (-not $Force) {
    $confirmation = Read-Host "Isso irá limpar todos os diretórios de build. Continuar? (S/N)"
    if ($confirmation -ne "S") {
        Write-Host "Operação cancelada."
        exit
    }
}

# Diretórios para limpar
$directories = @(
    "$RootDir/test",
    "$RootDir/temp",
    "$RootDir/emulators/nes",
    "$RootDir/emulators/megadrive",
    "$RootDir/emulators/mastersystem",
    "$RootDir/frontend/sdl",
    "$RootDir/frontend/qt",
    "$RootDir/Mega_tools"
)

# Adicionar released se não for mantido
if (-not $KeepReleased) {
    $directories += "$RootDir/released"
}

# Limpar cada diretório
foreach ($dir in $directories) {
    Remove-DirectoryContents -Path $dir -KeepGitkeep
}

# Limpar arquivos CMake no diretório raiz
Get-ChildItem -Path $RootDir -File | Where-Object {
    $_.Name -match "CMakeCache\.txt|cmake_install\.cmake$|Makefile$|\.cmake$" -and
    $_.Name -ne "CMakeLists.txt"
} | ForEach-Object {
    Remove-Item $_.FullName -Force
    Write-Host "Removido arquivo: $($_.FullName)"
}

Write-Host "`nLimpeza concluída!"
if ($KeepReleased) {
    Write-Host "Diretório 'released' foi mantido."
}

# Remover diretório vcpkg_installed se existir
$vcpkgInstalled = "vcpkg_installed"
if (Test-Path $vcpkgInstalled) {
    Remove-Item $vcpkgInstalled -Recurse -Force
    Write-Host "Removido diretório vcpkg_installed"
}

Write-Host "`nPara recriar a estrutura de diretórios, execute:"
Write-Host ".\scripts\build\create_build_structure.ps1"
