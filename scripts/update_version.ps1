# Script para atualizar a versão do projeto
param(
    [Parameter(Mandatory=$true)]
    [string]$Version,
    [switch]$NoPrompt
)

# Função para verificar versão semântica
function Test-SemVer {
    param([string]$Version)

    if ($Version -match '^\d+\.\d+\.\d+(-[0-9A-Za-z-]+(\.[0-9A-Za-z-]+)*)?(\+[0-9A-Za-z-]+(\.[0-9A-Za-z-]+)*)?$') {
        return $true
    }
    return $false
}

# Função para atualizar versão em arquivo
function Update-VersionInFile {
    param(
        [string]$FilePath,
        [string]$Version,
        [string]$Pattern,
        [string]$Replacement
    )

    if (Test-Path $FilePath) {
        $content = Get-Content $FilePath -Raw
        $newContent = $content -replace $Pattern, $Replacement

        if ($content -ne $newContent) {
            $newContent | Set-Content $FilePath
            Write-Host "Atualizado: $FilePath"
            return $true
        }
    }
    return $false
}

# Verificar formato da versão
if (-not (Test-SemVer $Version)) {
    Write-Host "Versão inválida. Use o formato semântico (ex: 1.0.0, 1.0.0-beta.1)"
    exit 1
}

# Confirmar atualização
if (-not $NoPrompt) {
    $confirmation = Read-Host "Atualizar para versão $Version? (S/N)"
    if ($confirmation -ne "S") {
        Write-Host "Operação cancelada."
        exit
    }
}

# Lista de arquivos e padrões para atualizar
$updates = @(
    @{
        Path = "CMakeLists.txt"
        Pattern = 'project\(Mega_Emu VERSION \d+\.\d+\.\d+.*?\)'
        Replacement = "project(Mega_Emu VERSION $Version)"
    }
    @{
        Path = "src/version.h"
        Pattern = '#define MEGA_EMU_VERSION ".*?"'
        Replacement = "#define MEGA_EMU_VERSION `"$Version`""
    }
    @{
        Path = "docs/AI_GUIDELINE.md"
        Pattern = 'PROJECT_NUMBER\s*=\s*\d+\.\d+\.\d+'
        Replacement = "PROJECT_NUMBER         = $Version"
    }
    @{
        Path = "vcpkg.json"
        Pattern = '"version":\s*".*?"'
        Replacement = "`"version`": `"$Version`""
    }
)

# Contador de arquivos atualizados
$updatedFiles = 0

# Atualizar cada arquivo
foreach ($update in $updates) {
    if (Update-VersionInFile `
        -FilePath $update.Path `
        -Version $Version `
        -Pattern $update.Pattern `
        -Replacement $update.Replacement) {
        $updatedFiles++
    }
    else {
        Write-Host "AVISO: Não foi possível atualizar $($update.Path)"
    }
}

# Criar tag Git
if ($updatedFiles -gt 0) {
    Write-Host "`nCriando tag Git..."
    git add -A
    git commit -m "build: atualizar versão para $Version"
    git tag -a "v$Version" -m "Versão $Version"

    Write-Host "`nPara enviar as alterações:"
    Write-Host "git push origin main"
    Write-Host "git push origin v$Version"
}

# Exibir resumo
Write-Host "`nResumo da atualização:"
Write-Host "Nova versão: $Version"
Write-Host "Arquivos atualizados: $updatedFiles"

if ($updatedFiles -eq 0) {
    Write-Host "Nenhum arquivo foi atualizado."
    exit 1
}

# Sugerir próximos passos
Write-Host "`nPróximos passos:"
Write-Host "1. Verifique as alterações com 'git diff'"
Write-Host "2. Atualize o CHANGELOG.md com './scripts/build/update_changelog.ps1 -Version $Version'"
Write-Host "3. Execute os testes com './scripts/build/run_tests.ps1'"
Write-Host "4. Crie um novo release com './scripts/build/create_release.ps1 -Version $Version'"
