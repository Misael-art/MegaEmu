param(
    [string]$projectRoot = (Resolve-Path "..\").Path,
    [switch]$updateRoadmap = $false
)

# Configurações
$roadmapFile = Join-Path $projectRoot "docs\ROADMAP.md"
$validationResults = @{}
$logFile = Join-Path $projectRoot "validation_results.log"
$hasError = $false

function Write-LogHeader {
    param (
        [string]$text
    )
    
    Write-Host "`n===== $text =====" -ForegroundColor Cyan
    Add-Content -Path $logFile -Value "`n===== $text ====="
}

function Write-LogSection {
    param (
        [string]$text
    )
    
    Write-Host "`n## $text" -ForegroundColor Yellow
    Add-Content -Path $logFile -Value "`n## $text"
}

function Write-LogSuccess {
    param (
        [string]$text
    )
    
    Write-Host "  [SUCESSO] $text" -ForegroundColor Green
    Add-Content -Path $logFile -Value "  [SUCESSO] $text"
    return $true
}

function Write-LogError {
    param (
        [string]$text
    )
    
    Write-Host "  [ERRO] $text" -ForegroundColor Red
    Add-Content -Path $logFile -Value "  [ERRO] $text"
    $script:hasError = $true
    return $false
}

function Write-LogWarning {
    param (
        [string]$text
    )
    
    Write-Host "  [AVISO] $text" -ForegroundColor Yellow
    Add-Content -Path $logFile -Value "  [AVISO] $text"
    return $false
}

function Write-LogInfo {
    param (
        [string]$text
    )
    
    Write-Host "  [INFO] $text" -ForegroundColor Gray
    Add-Content -Path $logFile -Value "  [INFO] $text"
}

function Test-FileExists {
    param (
        [string]$path,
        [string]$description
    )
    
    $fullPath = Join-Path $projectRoot $path
    if (Test-Path $fullPath) {
        return Write-LogSuccess "$description encontrado em $path"
    } else {
        return Write-LogError "$description não encontrado em $path"
    }
}

function Test-DirectoryExists {
    param (
        [string]$path,
        [string]$description
    )
    
    $fullPath = Join-Path $projectRoot $path
    if (Test-Path $fullPath -PathType Container) {
        return Write-LogSuccess "$description encontrado em $path"
    } else {
        return Write-LogError "$description não encontrado em $path"
    }
}

function Test-SourceContains {
    param (
        [string]$filePath,
        [string]$pattern,
        [string]$description
    )
    
    $fullPath = Join-Path $projectRoot $filePath
    if (Test-Path $fullPath) {
        $content = Get-Content -Path $fullPath -Raw
        if ($content -match $pattern) {
            return Write-LogSuccess "$description verificado em $filePath"
        } else {
            return Write-LogError "$description não encontrado em $filePath"
        }
    } else {
        return Write-LogError "Arquivo $filePath não encontrado"
    }
}

function Update-RoadmapStatus {
    param (
        [string]$section,
        [string]$item,
        [bool]$status
    )
    
    if ($updateRoadmap) {
        $roadmapContent = Get-Content -Path $roadmapFile
        $itemPattern = "- \[([ x])\] $([regex]::Escape($item))"
        $updatedContent = @()
        
        foreach ($line in $roadmapContent) {
            if ($line -match $itemPattern) {
                $newCheckbox = if ($status) { "x" } else { " " }
                $updatedLine = $line -replace $itemPattern, "- [$newCheckbox] $item"
                $updatedContent += $updatedLine
            } else {
                $updatedContent += $line
            }
        }
        
        Set-Content -Path $roadmapFile -Value $updatedContent
        Write-LogInfo "Status do item '$item' em '$section' atualizado para: $status"
    }
}

# Inicialização do log
if (Test-Path $logFile) {
    Remove-Item $logFile -Force
}

$dateTime = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
Add-Content -Path $logFile -Value "Validação do Mega_Emu iniciada em $dateTime"
Add-Content -Path $logFile -Value "Diretório raiz: $projectRoot"

Write-LogHeader "VALIDAÇÃO DOS COMPONENTES DO MEGA_EMU"
Write-LogInfo "Validando componentes marcados como concluídos no ROADMAP.md"

# ======================================================
# Fase 1: Infraestrutura Base
# ======================================================
Write-LogSection "Fase 1: Infraestrutura Base"

# Sistema de log
$validationResults["Sistema de log"] = Test-DirectoryExists "src\core\logging" "Sistema de log" -and 
                                       Test-SourceContains "src\core\logging\log.h" "void\s+log_" "Funções de log"

# Sistema de configuração
$validationResults["Sistema de configuração"] = Test-FileExists "src\core\config_system.hpp" "Sistema de configuração" -and
                                               Test-DirectoryExists "src\core\config" "Diretório de configuração"

# Sistema de gerenciamento de memória
$validationResults["Sistema de gerenciamento de memória"] = Test-DirectoryExists "src\core\memory" "Sistema de gerenciamento de memória"

# Sistema de eventos
$validationResults["Sistema de eventos"] = Test-DirectoryExists "src\core\events" "Sistema de eventos"

# Interface gráfica básica (SDL)
$validationResults["Interface gráfica básica (SDL)"] = Test-DirectoryExists "src\frontend" "Interface gráfica" -and
                                                      (Test-SourceContains "src\main.cpp" "SDL_" "Chamadas de SDL" -or
                                                       Test-SourceContains "src\frontend\renderer.cpp" "SDL_" "Chamadas de SDL")

# Abstração de entrada (teclado, joystick)
$validationResults["Abstração de entrada"] = Test-DirectoryExists "src\core\input" "Abstração de entrada" -and
                                            Test-FileExists "src\core\input_config.h" "Configuração de entrada"

# Ferramentas de depuração
$validationResults["Ferramentas de depuração"] = Test-DirectoryExists "src\core\debug" "Ferramentas de depuração"

# Estruturação de testes unitários
$validationResults["Estruturação de testes unitários"] = Test-DirectoryExists "src\tests" "Estrutura de testes unitários"

# Sistema de build multiplataforma
$validationResults["Sistema de build multiplataforma"] = Test-FileExists "CMakeLists.txt" "Sistema de build" -and
                                                        Test-FileExists "src\CMakeLists.txt" "Sistema de build para src"

# ======================================================
# Fase 2: Implementação NES
# ======================================================
Write-LogSection "Fase 2: Implementação NES"

# CPU (6502/2A03)
$validationResults["CPU (6502/2A03)"] = Test-DirectoryExists "src\platforms\nes\cpu" "CPU NES" -and
                                       Test-FileExists "src\platforms\nes\cpu\nes_cpu.c" "Implementação da CPU NES"

# PPU (Picture Processing Unit)
$validationResults["PPU (NES)"] = Test-DirectoryExists "src\platforms\nes\ppu" "PPU NES" -and
                                 Test-FileExists "src\platforms\nes\ppu\nes_ppu.c" "Implementação da PPU NES"

# APU (Audio Processing Unit) - básico
$validationResults["APU (NES)"] = Test-DirectoryExists "src\platforms\nes\apu" "APU NES" -and
                                 Test-FileExists "src\platforms\nes\apu\nes_apu.c" "Implementação da APU NES"

# Controladores NES
$validationResults["Controladores NES"] = Test-DirectoryExists "src\platforms\nes\input" "Controladores NES"

# Mappers básicos
Write-LogInfo "Validando mappers do NES..."

# NROM (Mapper 0)
$validationResults["NROM (Mapper 0)"] = Test-FileExists "src\platforms\nes\cartridge\mappers\mapper0.cpp" "Mapper 0 (NROM)"

# MMC1 (Mapper 1)
$validationResults["MMC1 (Mapper 1)"] = Test-FileExists "src\platforms\nes\cartridge\mappers\mapper1.cpp" "Mapper 1 (MMC1)"

# UxROM (Mapper 2)
$validationResults["UxROM (Mapper 2)"] = (Test-FileExists "src\platforms\nes\cartridge\mappers\mapper2.cpp" "Mapper 2 (UxROM)") -or
                                         (Test-SourceContains "src\platforms\nes\cartridge\nes_cartridge.c" "mapper_2" "Implementação do Mapper 2")

# CNROM (Mapper 3)
$validationResults["CNROM (Mapper 3)"] = (Test-FileExists "src\platforms\nes\cartridge\mappers\mapper3.cpp" "Mapper 3 (CNROM)") -or
                                         (Test-SourceContains "src\platforms\nes\cartridge\nes_cartridge.c" "mapper_3" "Implementação do Mapper 3")

# Sistema de salvamento NES
$validationResults["Sistema de salvamento NES"] = Test-DirectoryExists "src\platforms\nes\save" "Sistema de salvamento NES"

# Interface gráfica específica NES
$validationResults["Interface gráfica NES"] = Test-SourceContains "src\platforms\nes\nes.c" "render" "Renderização NES específica"

# Formatação da paleta de cores (compatibilidade ARGB)
$validationResults["Paleta de cores NES"] = Test-SourceContains "src\platforms\nes\ppu\nes_ppu.c" "palette" "Paleta de cores NES"

# ======================================================
# Fase 3: Implementação Master System
# ======================================================
Write-LogSection "Fase 3: Implementação Master System"

# CPU (Z80)
$validationResults["CPU (Z80)"] = Test-DirectoryExists "src\platforms\mastersystem\cpu" "CPU Master System" -or
                                 (Test-DirectoryExists "src\core\cpu\z80" "CPU Z80 comum")

# ======================================================
# Fase 4: Implementação Mega Drive
# ======================================================
Write-LogSection "Fase 4: Implementação Mega Drive"

# CPU principal (68000)
$validationResults["CPU principal (68000)"] = Test-DirectoryExists "src\platforms\megadrive\cpu" "CPU Mega Drive" -and
                                            (Test-FileExists "src\platforms\megadrive\cpu\m68k.c" "Implementação da CPU M68K" -or
                                             Test-FileExists "src\platforms\megadrive\cpu\m68000.cpp" "Implementação da CPU M68K")

# CPU secundária (Z80)
$validationResults["CPU secundária (Z80)"] = Test-DirectoryExists "src\platforms\megadrive\cpu" "CPUs Mega Drive" -and
                                           (Test-SourceContains "src\platforms\megadrive\megadrive.c" "z80" "Referência à CPU Z80")

# VDP (Video Display Processor) - básico
$validationResults["VDP (Mega Drive)"] = Test-DirectoryExists "src\platforms\megadrive\video" "VDP Mega Drive"

# Controladores Mega Drive
$validationResults["Controladores Mega Drive"] = Test-DirectoryExists "src\platforms\megadrive\io" "Controladores Mega Drive" -or
                                                (Test-SourceContains "src\platforms\megadrive\megadrive.c" "controller" "Código de controladores Mega Drive")

# Sistema de memória Mega Drive
$validationResults["Sistema de memória Mega Drive"] = Test-DirectoryExists "src\platforms\megadrive\memory" "Sistema de memória Mega Drive"

# ======================================================
# Resumo dos resultados
# ======================================================
Write-LogSection "Resumo da Validação"

$passCount = 0
$failCount = 0

foreach ($key in $validationResults.Keys) {
    $status = if ($validationResults[$key]) { "✓" } else { "✗" }
    $color = if ($validationResults[$key]) { "Green" } else { "Red" }
    Write-Host "  $status $key" -ForegroundColor $color
    
    if ($validationResults[$key]) {
        $passCount++
    } else {
        $failCount++
    }
    
    # Atualizar o ROADMAP.md se solicitado
    Update-RoadmapStatus "Fase correspondente" $key $validationResults[$key]
}

$totalItems = $validationResults.Count
$passRate = [math]::Round(($passCount / $totalItems) * 100, 2)

Write-LogHeader "RESULTADO FINAL"
Write-Host "Total de itens validados: $totalItems" -ForegroundColor Cyan
Write-Host "Itens validados com sucesso: $passCount ($passRate%)" -ForegroundColor $(if ($passRate -ge 80) { "Green" } elseif ($passRate -ge 50) { "Yellow" } else { "Red" })
Write-Host "Itens com falha: $failCount" -ForegroundColor $(if ($failCount -eq 0) { "Green" } else { "Red" })

Add-Content -Path $logFile -Value "`nResultado final: $passCount de $totalItems itens validados com sucesso ($passRate%)"
Add-Content -Path $logFile -Value "Validação concluída em $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"

if ($hasError) {
    Write-LogHeader "AÇÕES RECOMENDADAS"
    Write-Host "1. Verifique os itens marcados como concluídos no ROADMAP.md que falharam na validação" -ForegroundColor Yellow
    Write-Host "2. Implemente os componentes ausentes ou corrija os problemas encontrados" -ForegroundColor Yellow
    Write-Host "3. Atualize o ROADMAP.md para refletir o estado real do projeto" -ForegroundColor Yellow
    
    if ($updateRoadmap) {
        Write-Host "`nO ROADMAP.md foi atualizado para refletir o estado real de implementação." -ForegroundColor Cyan
    } else {
        Write-Host "`nExecute novamente com -updateRoadmap para atualizar automaticamente o ROADMAP.md" -ForegroundColor Cyan
    }
    
    return 1
} else {
    Write-LogHeader "CONCLUSÃO"
    Write-Host "Todos os itens marcados como concluídos no ROADMAP.md foram validados com sucesso!" -ForegroundColor Green
    return 0
}
