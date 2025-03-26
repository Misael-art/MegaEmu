# Script para atualizar o CHANGELOG
param(
    [Parameter(Mandatory = $true)]
    [string]$Version,
    [Parameter(Mandatory = $true)]
    [string]$Type,
    [string]$Message,
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

# Função para obter data formatada
function Get-FormattedDate {
    return Get-Date -Format "yyyy-MM-dd"
}

# Função para validar tipo de mudança
function Test-ChangeType {
    param([string]$Type)

    $validTypes = @(
        "Added",
        "Changed",
        "Deprecated",
        "Removed",
        "Fixed",
        "Security"
    )

    return $validTypes -contains $Type
}

# Verificar formato da versão
if (-not (Test-SemVer $Version)) {
    Write-Host "Versão inválida. Use o formato semântico (ex: 1.0.0, 1.0.0-beta.1)"
    exit 1
}

# Verificar tipo de mudança
if (-not (Test-ChangeType $Type)) {
    Write-Host "Tipo de mudança inválido. Use um dos seguintes:"
    Write-Host "Added, Changed, Deprecated, Removed, Fixed, Security"
    exit 1
}

# Verificar se arquivo existe
$changelogPath = "CHANGELOG.md"
if (-not (Test-Path $changelogPath)) {
    # Criar arquivo com cabeçalho inicial
    @"
# Changelog

Todas as mudanças notáveis neste projeto serão documentadas neste arquivo.

O formato é baseado em [Keep a Changelog](https://keepachangelog.com/pt-BR/1.0.0/),
e este projeto adere ao [Versionamento Semântico](https://semver.org/lang/pt-BR/).

"@ | Set-Content $changelogPath
}

# Ler conteúdo atual
$content = Get-Content $changelogPath -Raw
if (-not $content) {
    $content = ""
}

# Verificar se versão já existe
$versionHeader = "## [$Version]"
$versionExists = $content -match [regex]::Escape($versionHeader)

# Se não houver mensagem, solicitar ao usuário
if (-not $Message -and -not $NoPrompt) {
    $Message = Read-Host "Digite a mensagem de mudança"
}

# Preparar nova entrada
$date = Get-FormattedDate
$newEntry = "- $Message"

if ($versionExists) {
    # Adicionar à seção existente
    $pattern = "(?ms)$versionHeader.*?\n(## |$)"
    $replacement = { param($m)
        $section = $m.Value
        $typeHeader = "### $Type"

        if ($section -match [regex]::Escape($typeHeader)) {
            # Adicionar à lista existente
            $section = $section -replace "($typeHeader\n.*?)(\n## |\n$|$)", "`$1$newEntry`n`$2"
        }
        else {
            # Criar nova seção de tipo
            $section = $section -replace "(\n## |\n$|$)", "`n$typeHeader`n$newEntry`n`$1"
        }

        return $section
    }

    $content = $content -replace $pattern, $replacement
}
else {
    # Criar nova seção de versão
    $newSection = @"

$versionHeader - $date
### $Type
$newEntry
"@

    # Inserir após o cabeçalho inicial
    $content = $content -replace "(# Changelog.*?\n\n)", "`$1$newSection"
}

# Salvar alterações
$content | Set-Content $changelogPath

Write-Host "CHANGELOG atualizado com sucesso!"
Write-Host "Versão: $Version"
Write-Host "Tipo: $Type"
Write-Host "Mensagem: $Message"
