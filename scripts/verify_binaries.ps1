# Script para verificar a integridade dos binários gerados
param(
    [string]$BuildDir = "build",
    [switch]$Verbose
)

# Função para calcular hash SHA256
function Get-FileHash256 {
    param([string]$FilePath)

    try {
        $hash = Get-FileHash -Path $FilePath -Algorithm SHA256
        return $hash.Hash
    }
    catch {
        return $null
    }
}

# Função para verificar dependências de DLL
function Get-Dependencies {
    param([string]$FilePath)

    try {
        $output = & "drmemory" -no_follow_children -- "$FilePath"
        if ($LASTEXITCODE -eq 0) {
            return $output | Where-Object { $_ -match "\.dll" }
        }
    }
    catch {
        Write-Host "Erro ao verificar dependências. Dr. Memory está instalado?"
        return $null
    }
}

# Função para verificar tamanho do arquivo
function Get-FileSize {
    param([string]$FilePath)

    try {
        $file = Get-Item $FilePath
        return $file.Length
    }
    catch {
        return 0
    }
}

# Função para verificar se é um binário válido
function Test-ValidBinary {
    param([string]$FilePath)

    try {
        $bytes = [System.IO.File]::ReadAllBytes($FilePath)
        # Verificar assinatura PE para executáveis Windows
        if ($bytes[0] -eq 0x4D -and $bytes[1] -eq 0x5A) {
            return $true
        }
    }
    catch {
        return $false
    }
    return $false
}

# Função para verificar binário
function Test-Binary {
    param(
        [string]$FilePath,
        [string]$ExpectedHash = $null
    )

    if (-not (Test-Path $FilePath)) {
        Write-Host "Arquivo não encontrado: $FilePath"
        return $false
    }

    $isValid = Test-ValidBinary $FilePath
    if (-not $isValid) {
        Write-Host "Arquivo não é um binário válido: $FilePath"
        return $false
    }

    $hash = Get-FileHash256 $FilePath
    $size = Get-FileSize $FilePath
    $deps = Get-Dependencies $FilePath

    Write-Host "`nVerificando: $FilePath"
    Write-Host "Tamanho: $([math]::Round($size / 1KB, 2)) KB"
    Write-Host "Hash SHA256: $hash"

    if ($ExpectedHash -and $hash -ne $ExpectedHash) {
        Write-Host "AVISO: Hash não corresponde ao esperado!"
        Write-Host "Esperado: $ExpectedHash"
        Write-Host "Atual: $hash"
        return $false
    }

    if ($Verbose -and $deps) {
        Write-Host "`nDependências:"
        $deps | ForEach-Object {
            Write-Host "  $_"
        }
    }

    return $true
}

# Binários esperados
$binaries = @(
    @{
        Path = "$BuildDir/emulators/nes/nes_emu.exe"
        Hash = $null  # Será preenchido na primeira verificação
    }
    @{
        Path = "$BuildDir/emulators/megadrive/megadrive_emu.exe"
        Hash = $null
    }
    @{
        Path = "$BuildDir/emulators/mastersystem/mastersystem_emu.exe"
        Hash = $null
    }
    @{
        Path = "$BuildDir/frontend/sdl/sdl_frontend.exe"
        Hash = $null
    }
    @{
        Path = "$BuildDir/frontend/qt/qt_frontend.exe"
        Hash = $null
    }
    @{
        Path = "$BuildDir/Mega_tools/mega_tools.exe"
        Hash = $null
    }
)

# Verificar cada binário
$results = @{}
$allValid = $true

foreach ($binary in $binaries) {
    $path = $binary.Path
    $valid = Test-Binary -FilePath $path -ExpectedHash $binary.Hash
    $results[$path] = $valid
    if (-not $valid) {
        $allValid = $false
    }
}

# Exibir resumo
Write-Host "`nResumo da verificação:"
Write-Host "===================="
foreach ($result in $results.GetEnumerator()) {
    $status = if ($result.Value) { "OK" } else { "Falha" }
    Write-Host "$($result.Key): $status"
}

# Exibir status final
Write-Host "`nStatus final:"
if ($allValid) {
    Write-Host "Todos os binários verificados com sucesso!"
}
else {
    Write-Host "Alguns binários falharam na verificação."
    exit 1
}

# Salvar hashes para verificação futura
$hashFile = "binary_hashes.json"
if ($allValid) {
    $hashes = @{}
    foreach ($binary in $binaries) {
        $hash = Get-FileHash256 $binary.Path
        $hashes[$binary.Path] = $hash
    }

    $hashes | ConvertTo-Json | Set-Content $hashFile
    Write-Host "`nHashes salvos em: $hashFile"
}
