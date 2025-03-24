# Script Wrapper para Instalacao do Mega_Emu
# Este script tenta elevar privilegios automaticamente se necessario

# Desabilitar a execucao do perfil para evitar erros
$env:POWERSHELL_SKIP_PROFILE = $true

function Start-InstallerWithElevation {
    param (
        [string]$Arguments
    )

    try {
        # Verificar se ja esta rodando como administrador
        $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
        $principal = New-Object Security.Principal.WindowsPrincipal($identity)
        $isAdmin = $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

        if (-not $isAdmin) {
            Write-Host "Elevando privilegios..." -ForegroundColor Yellow

            # Obter o caminho completo do script atual
            $scriptPath = $MyInvocation.MyCommand.Path
            if (-not $scriptPath) {
                $scriptPath = $PSCommandPath
            }

            # Obter o diretório do script install.ps1
            $installScript = Join-Path (Split-Path -Parent $scriptPath) "install.ps1"

            # Preparar argumentos para o novo processo
            $processArgs = "-NoProfile -ExecutionPolicy Bypass -Command `"& '$installScript' $Arguments`""

            try {
                # Tentar iniciar um novo processo com privilegios elevados
                $process = Start-Process powershell.exe -Verb RunAs -ArgumentList $processArgs -Wait -PassThru

                if ($process.ExitCode -ne 0) {
                    throw "O processo de instalacao falhou com codigo de saida: $($process.ExitCode)"
                }
            }
            catch {
                Write-Host @"

ERRO: Nao foi possivel elevar os privilegios automaticamente.

Por favor, tente um dos seguintes metodos:

1) Execute o PowerShell como administrador:
   - Clique com o botao direito no PowerShell
   - Selecione 'Executar como administrador'
   - Navegue ate: D:\Steamapps\DevProjetos\PC Engines Projects\Mega_Emu
   - Execute: .\scripts\utils\install.ps1

2) Ou execute este comando em um prompt de comando (cmd) como administrador:
   powershell -NoProfile -ExecutionPolicy Bypass -File "$installScript"

Erro detalhado:
$($_.Exception.Message)

"@ -ForegroundColor Red
                exit 1
            }
        }
        else {
            # Se ja for admin, executar o script principal diretamente
            $installScript = Join-Path $PSScriptRoot "install.ps1"
            if (Test-Path $installScript) {
                & $installScript $Arguments
            }
            else {
                throw "Arquivo de instalacao nao encontrado: $installScript"
            }
        }
    }
    catch {
        Write-Host "ERRO: $($_.Exception.Message)" -ForegroundColor Red
        exit 1
    }
}

# Passar todos os argumentos recebidos para o script principal
$argString = $args -join " "
Start-InstallerWithElevation -Arguments $argString
