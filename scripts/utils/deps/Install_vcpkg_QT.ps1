# Diretório de instalação do vcpkg (ajuste se necessário)
$vcpkgInstallPath = "C:\vcpkg"

# Função para exibir erros e encerrar o script
function Show-Error {
    param ([string]$message)
    Write-Host "[ERRO] $message" -ForegroundColor Red
    Pause
    exit 1
}

# Função para instalar o Qt com tratamento de erros
function Install-Qt {
    Write-Host "Iniciando a instalação do Qt..."

    try {

        # Instala Qt x86
        Write-Host "Instalando Qt x86..."
        Start-Process -FilePath "cmd.exe" -ArgumentList "/c $($vcpkgInstallPath)\vcpkg.exe install qt5:x86-windows" -Wait -NoNewWindow -ErrorAction Stop
        if ($LASTEXITCODE -ne 0) {
            throw "Falha na instalação do Qt x86."
        }
        Write-Host "Instalação do Qt x86 concluída com sucesso."

        # Instala Qt x64
        Write-Host "Instalando Qt x64..."
        Start-Process -FilePath "cmd.exe" -ArgumentList "/c $($vcpkgInstallPath)\vcpkg.exe install qt5:x64-windows" -Wait -NoNewWindow -ErrorAction Stop
        if ($LASTEXITCODE -ne 0) {
            throw "Falha na instalação do Qt x64."
        }
        Write-Host "Instalação do Qt x64 concluída com sucesso."
    }
    catch {
        Show-Error "Ocorreu um erro durante a instalação do Qt: $($_.Exception.Message)"
    }
    Write-Host "Instalação do Qt concluída."
}

# Chamada da função de instalação do Qt
Install-Qt

Pause