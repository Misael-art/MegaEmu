# Mega_Emu Component Validation Script
# Este script verifica a existência e integridade dos componentes marcados como concluídos no ROADMAP.md

# Definir o caminho raiz do projeto
$projectRoot = "D:\Steamapps\DevProjetos\PC Engines Projects\Mega_Emu"
$resultsFile = "$projectRoot\validation_results.md"

# Função para registrar resultados
function Log-Result {
    param (
        [string]$component,
        [string]$status,
        [string]$details = ""
    )
    
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logEntry = "| $component | $status | $details | $timestamp |"
    
    Add-Content -Path $resultsFile -Value $logEntry
    Write-Host $logEntry
}

# Iniciar o arquivo de resultados
$header = @"
# Mega_Emu Component Validation Report
Gerado em: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Resultados da Validação

| Componente | Status | Detalhes | Timestamp |
|------------|--------|---------|-----------|
"@

Set-Content -Path $resultsFile -Value $header

# --------------------------
# Funções de validação de componentes
# --------------------------

function Test-CoreComponents {
    # Verificar componentes da Fase 1 (Infraestrutura)
    $coreComponents = @{
        "Sistema de Log" = @{
            Path = "$projectRoot\src\core\logging"
            Files = @("enhanced_logger.h")
        }
        "Sistema de Configuração" = @{
            Path = "$projectRoot\src\core\config"
            Files = @("config_interface.h", "config.c")
        }
        "Gerenciamento de Memória" = @{
            Path = "$projectRoot\src\core\memory"
            Files = @("memory_interface.c", "memory_interface.h", "memory.c")
        }
        "Sistema de Eventos" = @{
            Path = "$projectRoot\src\core\events"
            Files = @("event_dispatcher.c", "event_logger.c", "events_interface.h")
        }
        "Interface Gráfica Básica" = @{
            Path = "$projectRoot\src\frontend"
            Files = @("gui_manager.c", "gui_manager.h")
        }
        "Abstração de Entrada" = @{
            Path = "$projectRoot\src\core\input"
            Files = @("input_interface.h", "input.c", "input.h")
        }
        "Ferramentas de Depuração" = @{
            Path = "$projectRoot\src\core\debug"
            Files = @()  # Verificar se o diretório existe
        }
        "Framework de Testes" = {
            Path = "$projectRoot\src\tests"
            Files = @()  # Verificar se o diretório existe
        }
    }

    foreach ($component in $coreComponents.Keys) {
        $entry = $coreComponents[$component]
        $path = $entry.Path
        $files = $entry.Files
        
        $componentExists = Test-Path -Path $path
        $filesExist = $true
        $missingFiles = @()
        
        # Verificar arquivos específicos se existirem
        if ($componentExists -and $files.Count -gt 0) {
            foreach ($file in $files) {
                $filePath = Join-Path -Path $path -ChildPath $file
                if (-not (Test-Path -Path $filePath)) {
                    $filesExist = $false
                    $missingFiles += $file
                }
            }
        }
        
        if ($componentExists -and $filesExist) {
            Log-Result -component $component -status "✅ VALIDADO" -details "Diretório e arquivos principais encontrados"
        } elseif ($componentExists) {
            Log-Result -component $component -status "⚠️ PARCIAL" -details "Diretório existe mas faltam arquivos: $($missingFiles -join ', ')"
        } else {
            Log-Result -component $component -status "❌ FALHOU" -details "Diretório não encontrado"
        }
    }
}

function Test-NESComponents {
    # Verificar componentes do NES (Fase 2)
    $nesComponents = @{
        "CPU (6502/2A03)" = @{
            Path = "$projectRoot\src\platforms\nes\cpu"
            Files = @()
        }
        "PPU (Picture Processing Unit)" = @{
            Path = "$projectRoot\src\platforms\nes\ppu"
            Files = @()
        }
        "APU (Audio Processing Unit)" = @{
            Path = "$projectRoot\src\platforms\nes\apu"
            Files = @()
        }
        "Controladores NES" = @{
            Path = "$projectRoot\src\platforms\nes\input"
            Files = @()
        }
        "Mapper 0 (NROM)" = @{
            Path = "$projectRoot\src\platforms\nes\cartridge\mappers"
            Files = @("mapper0.cpp", "mapper0.hpp")
        }
        "Mapper 1 (MMC1)" = @{
            Path = "$projectRoot\src\platforms\nes\cartridge\mappers"
            Files = @("mapper1.cpp", "mapper1.hpp")
        }
        "Mapper 2 (UxROM)" = @{
            Path = "$projectRoot\src\platforms\nes\cartridge\mappers"
            Files = @()  # Verificar menção no código
        }
        "Mapper 3 (CNROM)" = @{
            Path = "$projectRoot\src\platforms\nes\cartridge\mappers"
            Files = @()  # Verificar menção no código
        }
        "Sistema de Salvamento NES" = @{
            Path = "$projectRoot\src\platforms\nes"
            Files = @()  # Verificar funcionalidade no código
        }
    }

    foreach ($component in $nesComponents.Keys) {
        $entry = $nesComponents[$component]
        $path = $entry.Path
        $files = $entry.Files
        
        $componentExists = Test-Path -Path $path
        $filesExist = $true
        $missingFiles = @()
        
        # Verificar arquivos específicos se existirem
        if ($componentExists -and $files.Count -gt 0) {
            foreach ($file in $files) {
                $filePath = Join-Path -Path $path -ChildPath $file
                if (-not (Test-Path -Path $filePath)) {
                    $filesExist = $false
                    $missingFiles += $file
                }
            }
        }
        
        if ($componentExists -and $filesExist) {
            Log-Result -component $component -status "✅ VALIDADO" -details "Diretório e arquivos principais encontrados"
        } elseif ($componentExists) {
            # Verificação especial para mappers mencionados no código
            if ($component -like "Mapper*" -and $files.Count -eq 0) {
                # Verifica se há menção ao mapper no código nes_cartridge.c/h
                $nesCartridgeFiles = @(
                    "$projectRoot\src\platforms\nes\cartridge\nes_cartridge.c",
                    "$projectRoot\src\platforms\nes\cartridge\nes_cartridge.h",
                    "$projectRoot\src\platforms\nes\cartridge\nes_cartridge.cpp",
                    "$projectRoot\src\platforms\nes\cartridge\nes_cartridge.hpp"
                )
                
                $mapperSupported = $false
                foreach ($file in $nesCartridgeFiles) {
                    if (Test-Path $file) {
                        $content = Get-Content $file -Raw
                        # Extrair o número do mapper do nome do componente
                        if ($component -match "Mapper (\d+)") {
                            $mapperNumber = $matches[1]
                            if ($content -match "mapper.*$mapperNumber" -or $content -match "MAPPER.*$mapperNumber") {
                                $mapperSupported = $true
                                break
                            }
                        }
                    }
                }
                
                if ($mapperSupported) {
                    Log-Result -component $component -status "⚠️ PARCIAL" -details "Mencionado no código, mas sem arquivos dedicados"
                } else {
                    Log-Result -component $component -status "❌ FALHOU" -details "Diretório existe mas não há menção ao mapper no código"
                }
            } else {
                Log-Result -component $component -status "⚠️ PARCIAL" -details "Diretório existe mas faltam arquivos: $($missingFiles -join ', ')"
            }
        } else {
            Log-Result -component $component -status "❌ FALHOU" -details "Diretório não encontrado"
        }
    }
}

function Test-MegaDriveComponents {
    # Verificar componentes do Mega Drive (Fase 4)
    $megaDriveComponents = @{
        "CPU principal (68000)" = @{
            Path = "$projectRoot\src\platforms\megadrive\cpu"
            Files = @()
        }
        "CPU secundária (Z80)" = @{
            Path = "$projectRoot\src\platforms\megadrive\z80"
            Files = @()
        }
        "VDP (Video Display Processor)" = @{
            Path = "$projectRoot\src\platforms\megadrive\vdp"
            Files = @()
        }
        "Controladores Mega Drive" = @{
            Path = "$projectRoot\src\platforms\megadrive\input"
            Files = @()
        }
        "Sistema de Memória Mega Drive" = @{
            Path = "$projectRoot\src\platforms\megadrive\memory"
            Files = @()
        }
    }

    foreach ($component in $megaDriveComponents.Keys) {
        $entry = $megaDriveComponents[$component]
        $path = $entry.Path
        $files = $entry.Files
        
        $componentExists = Test-Path -Path $path
        $filesExist = $true
        $missingFiles = @()
        
        # Verificar arquivos específicos se existirem
        if ($componentExists -and $files.Count -gt 0) {
            foreach ($file in $files) {
                $filePath = Join-Path -Path $path -ChildPath $file
                if (-not (Test-Path -Path $filePath)) {
                    $filesExist = $false
                    $missingFiles += $file
                }
            }
        }
        
        if ($componentExists -and $filesExist) {
            Log-Result -component $component -status "✅ VALIDADO" -details "Diretório e arquivos principais encontrados"
        } elseif ($componentExists) {
            Log-Result -component $component -status "⚠️ PARCIAL" -details "Diretório existe mas faltam arquivos: $($missingFiles -join ', ')"
        } else {
            Log-Result -component $component -status "❌ FALHOU" -details "Diretório não encontrado"
        }
    }
}

function Test-MasterSystemComponents {
    # Verificar componentes do Master System (Fase 3)
    $masterSystemComponents = @{
        "CPU (Z80) Master System" = @{
            Path = "$projectRoot\src\platforms\sms\cpu"
            Files = @()
        }
    }

    foreach ($component in $masterSystemComponents.Keys) {
        $entry = $masterSystemComponents[$component]
        $path = $entry.Path
        $files = $entry.Files
        
        $componentExists = Test-Path -Path $path
        $filesExist = $true
        $missingFiles = @()
        
        # Verificar arquivos específicos se existirem
        if ($componentExists -and $files.Count -gt 0) {
            foreach ($file in $files) {
                $filePath = Join-Path -Path $path -ChildPath $file
                if (-not (Test-Path -Path $filePath)) {
                    $filesExist = $false
                    $missingFiles += $file
                }
            }
        }
        
        if ($componentExists -and $filesExist) {
            Log-Result -component $component -status "✅ VALIDADO" -details "Diretório e arquivos principais encontrados"
        } elseif ($componentExists) {
            Log-Result -component $component -status "⚠️ PARCIAL" -details "Diretório existe mas faltam arquivos: $($missingFiles -join ', ')"
        } else {
            Log-Result -component $component -status "❌ FALHOU" -details "Diretório não encontrado"
        }
    }
}

function Test-BuildSystem {
    # Verificar sistema de build
    $cmakeFiles = @(
        "$projectRoot\CMakeLists.txt",
        "$projectRoot\src\CMakeLists.txt"
    )
    
    $cmakeFilesExist = $true
    $missingCMakeFiles = @()
    
    foreach ($file in $cmakeFiles) {
        if (-not (Test-Path -Path $file)) {
            $cmakeFilesExist = $false
            $missingCMakeFiles += $file
        }
    }
    
    if ($cmakeFilesExist) {
        Log-Result -component "Sistema de Build" -status "✅ VALIDADO" -details "Arquivos CMake encontrados"
    } else {
        Log-Result -component "Sistema de Build" -status "⚠️ PARCIAL" -details "Faltam arquivos CMake: $($missingCMakeFiles -join ', ')"
    }
}

function Validate-EmulatorCompilation {
    # Testar a compilação do emulador
    try {
        # Mudar para o diretório de build
        $buildDir = "$projectRoot\build"
        if (-not (Test-Path -Path $buildDir)) {
            New-Item -Path $buildDir -ItemType Directory | Out-Null
        }
        
        Push-Location $buildDir
        
        # Executar o CMake
        $cmakeResult = Invoke-Expression "cmake .." 2>&1
        if ($LASTEXITCODE -ne 0) {
            Log-Result -component "Compilação" -status "❌ FALHOU" -details "Erro ao executar CMake: $cmakeResult"
            return
        }
        
        # Executar o Build
        $buildResult = Invoke-Expression "cmake --build ." 2>&1
        if ($LASTEXITCODE -ne 0) {
            Log-Result -component "Compilação" -status "❌ FALHOU" -details "Erro ao compilar: $buildResult"
            return
        }
        
        Log-Result -component "Compilação" -status "✅ VALIDADO" -details "Compilação bem-sucedida"
    }
    catch {
        Log-Result -component "Compilação" -status "❌ FALHOU" -details "Erro: $_"
    }
    finally {
        Pop-Location
    }
}

# --------------------------
# Execução principal
# --------------------------

Write-Host "Iniciando validação dos componentes do Mega_Emu..."
Log-Result -component "Processo de Validação" -status "INICIADO" -details "Validação iniciada para componentes marcados no ROADMAP.md"

# Executar as validações
Test-CoreComponents
Test-NESComponents
Test-MegaDriveComponents
Test-MasterSystemComponents
Test-BuildSystem

# Adicionar separador de conclusão
Add-Content -Path $resultsFile -Value "`n## Resumo de Validação`n"
Add-Content -Path $resultsFile -Value "Validação concluída em $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')`n"

# Adicionar recomendações
Add-Content -Path $resultsFile -Value "## Recomendações"
Add-Content -Path $resultsFile -Value "1. Corrigir componentes marcados como falha (❌)"
Add-Content -Path $resultsFile -Value "2. Completar a implementação de componentes parciais (⚠️)"
Add-Content -Path $resultsFile -Value "3. Atualizar o ROADMAP.md com o estado atual"
Add-Content -Path $resultsFile -Value "4. Priorizar a implementação de mappers NES faltantes"

Write-Host "`nValidação concluída. Resultados salvos em $resultsFile"
