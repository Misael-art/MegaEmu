param(
    [string]$filePath = "..\docs\ROADMAP.md"
)

$content = Get-Content -Path $filePath -Raw
$lines = Get-Content -Path $filePath

Write-Host "===== VALIDAÇÃO DO ROADMAP.MD =====" -ForegroundColor Cyan

# 1. Verificar cabeçalhos e formatação
Write-Host "`n[1] Validando estrutura de cabeçalhos..." -ForegroundColor Yellow
$headings = $lines | Where-Object { $_ -match "^#{1,6}\s" }
$invalidHeadings = $headings | Where-Object { $_ -notmatch "^#{1,6}\s+\w+" }

if ($invalidHeadings) {
    Write-Host "  [ERRO] Cabeçalhos malformados encontrados:" -ForegroundColor Red
    foreach ($heading in $invalidHeadings) {
        Write-Host "    - $heading" -ForegroundColor Red
    }
} else {
    Write-Host "  [OK] Todos os cabeçalhos estão formatados corretamente." -ForegroundColor Green
}

# 2. Verificar listas e formatação
Write-Host "`n[2] Validando listas..." -ForegroundColor Yellow
$listItems = $lines | Where-Object { $_ -match "^\s*[-*+]\s" }
$invalidListItems = $listItems | Where-Object { $_ -notmatch "^\s*[-*+]\s+\w+" }

if ($invalidListItems) {
    Write-Host "  [ERRO] Itens de lista malformados encontrados:" -ForegroundColor Red
    foreach ($item in $invalidListItems) {
        Write-Host "    - $item" -ForegroundColor Red
    }
} else {
    Write-Host "  [OK] Todos os itens de lista estão formatados corretamente." -ForegroundColor Green
}

# 3. Verificar referências a outros documentos
Write-Host "`n[3] Validando referências a outros documentos..." -ForegroundColor Yellow
$referencesFound = [regex]::Matches($content, "@\[(.+?)\]")
$foundReferences = @()

foreach ($ref in $referencesFound) {
    $refName = $ref.Groups[1].Value
    $foundReferences += $refName
    $refPath = "..\docs\$refName"
    
    if (-not (Test-Path $refPath)) {
        Write-Host "  [ERRO] Referência não encontrada: $refName" -ForegroundColor Red
    }
}

if ($foundReferences.Count -gt 0) {
    Write-Host "  [OK] $($foundReferences.Count) referências encontradas." -ForegroundColor Green
} else {
    Write-Host "  [AVISO] Nenhuma referência encontrada." -ForegroundColor Yellow
}

# 4. Verificar consistência dos status (✅, 🔄, ⏹️, [ ], [x])
Write-Host "`n[4] Validando indicadores de status..." -ForegroundColor Yellow
$checkboxesTotal = ($lines | Select-String -Pattern "\[[ x]\]").Count
$completeTasksTotal = ($lines | Select-String -Pattern "\[x\]").Count
$pendingTasksTotal = ($lines | Select-String -Pattern "\[ \]").Count
$completePhasesTotal = ($lines | Select-String -Pattern "✅").Count
$inProgressPhasesTotal = ($lines | Select-String -Pattern "🔄").Count
$pendingPhasesTotal = ($lines | Select-String -Pattern "⏹️").Count

Write-Host "  Status de tarefas encontrados:" -ForegroundColor Green
Write-Host "    - Tarefas concluídas ([x]): $completeTasksTotal" -ForegroundColor Green
Write-Host "    - Tarefas pendentes ([ ]): $pendingTasksTotal" -ForegroundColor Yellow
Write-Host "    - Fases completas (✅): $completePhasesTotal" -ForegroundColor Green
Write-Host "    - Fases em andamento (🔄): $inProgressPhasesTotal" -ForegroundColor Yellow
Write-Host "    - Fases pendentes (⏹️): $pendingPhasesTotal" -ForegroundColor Gray

# 5. Verificar cronograma
Write-Host "`n[5] Validando cronograma..." -ForegroundColor Yellow
$cronogramaSection = $false
$datesFound = 0

foreach ($line in $lines) {
    if ($line -match "^## Cronograma") {
        $cronogramaSection = $true
    } elseif ($cronogramaSection -and $line -match "^\|.+\|.+\|.+\|.+\|$") {
        if ($line -match "\d{4}-\d{2}-\d{2}") {
            $datesFound++
        }
    } elseif ($cronogramaSection -and $line -match "^##") {
        $cronogramaSection = $false
    }
}

if ($datesFound -gt 0) {
    Write-Host "  [OK] Cronograma contém $datesFound entradas com datas." -ForegroundColor Green
} else {
    Write-Host "  [AVISO] Possível problema no formato do cronograma." -ForegroundColor Yellow
}

# 6. Verificar consistência dos trimestres
Write-Host "`n[6] Validando planejamento trimestral..." -ForegroundColor Yellow
$quartersPattern = "Q[1-4] \d{4}"
$quartersFound = [regex]::Matches($content, $quartersPattern)
$quartersList = @()

foreach ($quarter in $quartersFound) {
    $quartersList += $quarter.Value
}

$uniqueQuarters = $quartersList | Sort-Object | Get-Unique
Write-Host "  Trimestres encontrados no planejamento:" -ForegroundColor Green
foreach ($quarter in $uniqueQuarters) {
    Write-Host "    - $quarter" -ForegroundColor Green
}

# 7. Verificar estimativas
Write-Host "`n[7] Validando estimativas..." -ForegroundColor Yellow
$estimativas = [regex]::Matches($content, "\(est: (\d+) semanas\)")
$total = 0

foreach ($est in $estimativas) {
    $semanas = [int]$est.Groups[1].Value
    $total += $semanas
}

$mediaEmAnos = [math]::Round($total / 52, 1)
Write-Host "  [INFO] Total de $($estimativas.Count) estimativas encontradas." -ForegroundColor Green
Write-Host "  [INFO] Tempo total estimado: $total semanas (~$mediaEmAnos anos)." -ForegroundColor Green

# 8. Verificar inconsistências
Write-Host "`n[8] Verificando possíveis inconsistências..." -ForegroundColor Yellow

# Verificar datas
$datesPattern = "\d{4}-\d{2}-\d{2}"
$datesFound = [regex]::Matches($content, $datesPattern)
$datesArray = @()

foreach ($date in $datesFound) {
    try {
        $dateValue = [datetime]::ParseExact($date.Value, "yyyy-MM-dd", $null)
        $datesArray += $dateValue
    } catch {
        Write-Host "  [ERRO] Data inválida encontrada: $($date.Value)" -ForegroundColor Red
    }
}

if ($datesArray.Count -gt 0) {
    $minDate = ($datesArray | Measure-Object -Minimum).Minimum
    $maxDate = ($datesArray | Measure-Object -Maximum).Maximum
    Write-Host "  [INFO] Período do cronograma: $($minDate.ToString('yyyy-MM-dd')) até $($maxDate.ToString('yyyy-MM-dd'))" -ForegroundColor Green
}

# Verificar ortografia
$typosComuns = @(
    "Corrações" # Deve ser "Correções"
)

foreach ($typo in $typosComuns) {
    if ($content -match $typo) {
        Write-Host "  [ERRO] Possível erro de digitação encontrado: '$typo'" -ForegroundColor Red
    }
}

# 9. Resumo
Write-Host "`n[9] Resumo da validação:" -ForegroundColor Magenta
Write-Host "  - Total de linhas: $($lines.Count)" -ForegroundColor Cyan
Write-Host "  - Total de cabeçalhos: $($headings.Count)" -ForegroundColor Cyan
Write-Host "  - Total de itens de lista: $($listItems.Count)" -ForegroundColor Cyan
Write-Host "  - Total de referências: $($foundReferences.Count)" -ForegroundColor Cyan
Write-Host "  - Total de tarefas: $checkboxesTotal (Concluídas: $completeTasksTotal, Pendentes: $pendingTasksTotal)" -ForegroundColor Cyan
Write-Host "  - Total de fases: $($completePhasesTotal + $inProgressPhasesTotal + $pendingPhasesTotal) (Completas: $completePhasesTotal, Em andamento: $inProgressPhasesTotal, Pendentes: $pendingPhasesTotal)" -ForegroundColor Cyan
Write-Host "  - Trimestres planejados: $($uniqueQuarters.Count)" -ForegroundColor Cyan

Write-Host "`n===== FIM DA VALIDAÇÃO =====" -ForegroundColor Cyan
