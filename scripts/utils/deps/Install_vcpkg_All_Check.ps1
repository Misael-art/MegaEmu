# Diretório de instalação do vcpkg (ajuste se necessário)
$vcpkgInstallPath = "C:\vcpkg\installed"

# Lista de pacotes para verificar
$packages = @(
    "sdl2",
    "sdl2-ttf",
    "zlib",
    "libpng",
    "opengl",
    "gtest",
    "doxygen"
)

# Função para verificar a instalação de um pacote e exibir informações
function Check-PackageInstallation {
    param (
        [string]$packageName
    )

    Write-Host "Verificando instalação de $($packageName)..."

    # Verifica se a pasta do pacote existe em ambas as arquiteturas
    $x86Path = Join-Path -Path $vcpkgInstallPath -ChildPath "x86-windows\lib\$packageName.lib"
    $x64Path = Join-Path -Path $vcpkgInstallPath -ChildPath "x64-windows\lib\$packageName.lib"
    $doxygenExe = "C:\vcpkg\installed\x64-windows\tools\doxygen\doxygen.exe"
    if (Test-Path $x86Path -and Test-Path $x64Path) {
        Write-Host "  $($packageName) instalado com sucesso:" -ForegroundColor Green
        Write-Host "    x86: $(Split-Path -Path $x86Path)"
        Write-Host "    x64: $(Split-Path -Path $x64Path)"
    }
    elseif ($packageName -eq "doxygen" -and Test-Path $doxygenExe) {
            Write-Host "  $($packageName) instalado com sucesso:" -ForegroundColor Green
            Write-Host "    caminho: $(Split-Path -Path $doxygenExe)"

    }
    else {
        Write-Host "  $($packageName) não encontrado." -ForegroundColor Red
    }

    Write-Host "-----------------------------"
}

# Loop para verificar cada pacote na lista
foreach ($package in $packages) {
    Check-PackageInstallation -packageName $package
}

Write-Host "Verificação concluída."
Pause