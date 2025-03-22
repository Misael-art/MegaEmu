# Script de verificação de componentes do Mega_Emu
$projectRoot = (Resolve-Path "..").Path
$logFile = Join-Path $projectRoot "validation_results.log"

# Limpar log anterior, se existir
if (Test-Path $logFile) {
    Remove-Item $logFile -Force
}

# Função para registrar mensagens
function Log-Message {
    param (
        [string]$message,
        [string]$type = "INFO"
    )
    
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$type] $message"
    
    # Definir cores para console
    $color = switch ($type) {
        "SUCCESS" { "Green" }
        "ERROR" { "Red" }
        "WARNING" { "Yellow" }
        "INFO" { "Gray" }
        "HEADER" { "Cyan" }
        default { "White" }
    }
    
    # Exibir no console
    Write-Host $logMessage -ForegroundColor $color
    
    # Salvar no arquivo de log
    Add-Content -Path $logFile -Value $logMessage
}

# Verificar existência do diretório ou arquivo
function Test-ComponentExists {
    param (
        [string]$path,
        [string]$component,
        [string]$type = "file"
    )
    
    $fullPath = Join-Path $projectRoot $path
    
    if ($type -eq "dir") {
        $exists = Test-Path $fullPath -PathType Container
    } else {
        $exists = Test-Path $fullPath -PathType Leaf
    }
    
    if ($exists) {
        Log-Message "$component verificado com sucesso" "SUCCESS"
        return $true
    } else {
        Log-Message "$component não encontrado ($path)" "ERROR"
        return $false
    }
}

# Iniciar verificação
Log-Message "Iniciando verificação de componentes do Mega_Emu" "HEADER"
Log-Message "Diretório do projeto: $projectRoot" "INFO"

# Resultados da verificação
$results = @{}

# ===== Fase 1: Infraestrutura Base =====
Log-Message "Verificando Fase 1: Infraestrutura Base" "HEADER"

# Sistema de log
$results["Sistema de log"] = Test-ComponentExists "src\core\logging" "Sistema de log" "dir"

# Sistema de configuração
$results["Sistema de configuração"] = Test-ComponentExists "src\core\config_system.hpp" "Sistema de configuração" "file"

# Sistema de gerenciamento de memória
$results["Sistema de gerenciamento de memória"] = Test-ComponentExists "src\core\memory" "Sistema de gerenciamento de memória" "dir"

# Sistema de eventos
$results["Sistema de eventos"] = Test-ComponentExists "src\core\events" "Sistema de eventos" "dir"

# Interface gráfica básica (SDL)
$results["Interface gráfica básica (SDL)"] = Test-ComponentExists "src\frontend" "Interface gráfica" "dir"

# Abstração de entrada (teclado, joystick)
$results["Abstração de entrada"] = Test-ComponentExists "src\core\input" "Abstração de entrada" "dir"

# Ferramentas de depuração
$results["Ferramentas de depuração"] = Test-ComponentExists "src\core\debug" "Ferramentas de depuração" "dir"

# Estruturação de testes unitários
$results["Estruturação de testes unitários"] = Test-ComponentExists "src\tests" "Estrutura de testes unitários" "dir"

# Sistema de build multiplataforma
$results["Sistema de build multiplataforma"] = Test-ComponentExists "CMakeLists.txt" "Sistema de build" "file"

# ===== Fase 2: Implementação NES =====
Log-Message "Verificando Fase 2: Implementação NES" "HEADER"

# CPU (6502/2A03)
$results["CPU (6502/2A03)"] = Test-ComponentExists "src\platforms\nes\cpu" "CPU NES" "dir"

# PPU (Picture Processing Unit)
$results["PPU (NES)"] = Test-ComponentExists "src\platforms\nes\ppu" "PPU NES" "dir"

# APU (Audio Processing Unit) - básico
$results["APU (NES)"] = Test-ComponentExists "src\platforms\nes\apu" "APU NES" "dir"

# Controladores NES
$results["Controladores NES"] = Test-ComponentExists "src\platforms\nes\input" "Controladores NES" "dir"

# Mappers básicos
Log-Message "Verificando Mappers do NES..." "INFO"

# NROM (Mapper 0)
$results["NROM (Mapper 0)"] = Test-ComponentExists "src\platforms\nes\cartridge\mappers\mapper0.cpp" "Mapper 0 (NROM)" "file"

# MMC1 (Mapper 1)
$results["MMC1 (Mapper 1)"] = Test-ComponentExists "src\platforms\nes\cartridge\mappers\mapper1.cpp" "Mapper 1 (MMC1)" "file"

# ===== Fase 3: Implementação Master System =====
Log-Message "Verificando Fase 3: Implementação Master System" "HEADER"

# CPU (Z80)
$results["CPU (Z80)"] = Test-ComponentExists "src\platforms\mastersystem\cpu" "CPU Master System" "dir"

# ===== Fase 4: Implementação Mega Drive =====
Log-Message "Verificando Fase 4: Implementação Mega Drive" "HEADER"

# CPU principal (68000)
$results["CPU principal (68000)"] = Test-ComponentExists "src\platforms\megadrive\cpu" "CPU Mega Drive" "dir"

# CPU secundária (Z80)
$cpuZ80 = Test-ComponentExists "src\platforms\megadrive\cpu" "CPUs Mega Drive" "dir"
$results["CPU secundária (Z80)"] = $cpuZ80

# VDP (Video Display Processor) - básico
$results["VDP (Mega Drive)"] = Test-ComponentExists "src\platforms\megadrive\video" "VDP Mega Drive" "dir"

# Controladores Mega Drive
$results["Controladores Mega Drive"] = Test-ComponentExists "src\platforms\megadrive\io" "Controladores Mega Drive" "dir"

# Sistema de memória Mega Drive
$results["Sistema de memória Mega Drive"] = Test-ComponentExists "src\platforms\megadrive\memory" "Sistema de memória Mega Drive" "dir"

# ===== Resumo da verificação =====
Log-Message "Resumo da verificação" "HEADER"

$totalItems = $results.Count
$successItems = ($results.Values | Where-Object { $_ -eq $true }).Count
$failedItems = $totalItems - $successItems
$successRate = [math]::Round(($successItems / $totalItems) * 100, 2)

Log-Message "Total de itens verificados: $totalItems" "INFO"
Log-Message "Itens verificados com sucesso: $successItems ($successRate%)" "INFO"
Log-Message "Itens com falha: $failedItems" "INFO"

# Exibir resultados detalhados
Log-Message "Resultados detalhados:" "INFO"
foreach ($key in $results.Keys) {
    $status = if ($results[$key]) { "✓" } else { "✗" }
    $type = if ($results[$key]) { "SUCCESS" } else { "ERROR" }
    Log-Message "$status $key" $type
}

# Resultado final
if ($failedItems -eq 0) {
    Log-Message "Todos os componentes verificados estão presentes!" "SUCCESS"
    exit 0
} else {
    Log-Message "Alguns componentes verificados não foram encontrados. Verifique os detalhes acima." "ERROR"
    exit 1
}
