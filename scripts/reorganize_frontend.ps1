# Script para reorganizar o frontend React
$ErrorActionPreference = "Stop"

# Definir caminhos
$sourceDir = Join-Path $PSScriptRoot ".." "frontend"
$nodeModulesDir = Join-Path $PSScriptRoot ".." "node_modules"
$targetDir = Join-Path $PSScriptRoot ".." "src" "frontend" "react"

Write-Host "🔍 Verificando ambiente..."

# Verificar se os diretórios existem
if (-not (Test-Path $sourceDir)) {
    Write-Host "❌ Diretório frontend não encontrado!" -ForegroundColor Red
    exit 1
}

# Criar estrutura de diretórios
Write-Host "📁 Criando estrutura de diretórios..."
$directories = @(
    "@types",
    "assets",
    "components/atoms",
    "components/molecules",
    "components/organisms",
    "components/templates",
    "components/pages",
    "config",
    "core/emulator",
    "core/audio",
    "core/video",
    "features/rom-library",
    "features/save-states",
    "features/debugger",
    "features/settings",
    "hooks",
    "lib",
    "services",
    "store",
    "utils"
)

foreach ($dir in $directories) {
    $path = Join-Path $targetDir $dir
    New-Item -ItemType Directory -Force -Path $path | Out-Null
}

# Parar processos Node.js
Write-Host "🛑 Parando processos Node.js..."
Get-Process -Name "node", "npm" -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Seconds 5

# Mover e reorganizar arquivos
Write-Host "📦 Movendo e reorganizando arquivos..."
try {
    # Mover arquivos de configuração
    Move-Item -Path (Join-Path $sourceDir "package.json") -Destination $targetDir -Force
    Move-Item -Path (Join-Path $sourceDir "tsconfig.json") -Destination $targetDir -Force
    Move-Item -Path (Join-Path $sourceDir ".eslintrc*") -Destination $targetDir -Force -ErrorAction SilentlyContinue
    Move-Item -Path (Join-Path $sourceDir ".prettierrc*") -Destination $targetDir -Force -ErrorAction SilentlyContinue
    Move-Item -Path (Join-Path $sourceDir "vite.config.*") -Destination $targetDir -Force -ErrorAction SilentlyContinue

    # Mover arquivos fonte
    $srcDir = Join-Path $sourceDir "src"
    if (Test-Path $srcDir) {
        # Mover componentes para estrutura atômica
        Get-ChildItem -Path (Join-Path $srcDir "components") -Filter "*.tsx" | ForEach-Object {
            $destDir = Join-Path $targetDir "components/atoms"
            Move-Item -Path $_.FullName -Destination $destDir -Force
        }

        # Mover features para nova estrutura
        Get-ChildItem -Path (Join-Path $srcDir "features") -Directory | ForEach-Object {
            $featureName = $_.Name
            $destDir = Join-Path $targetDir "features/$featureName"
            Move-Item -Path $_.FullName -Destination $destDir -Force
        }

        # Mover outros diretórios
        $dirMap = @{
            "hooks"    = "hooks"
            "services" = "services"
            "utils"    = "utils"
            "store"    = "store"
            "types"    = "@types"
            "assets"   = "assets"
        }

        foreach ($dir in $dirMap.Keys) {
            $sourcePath = Join-Path $srcDir $dir
            if (Test-Path $sourcePath) {
                $destPath = Join-Path $targetDir $dirMap[$dir]
                Get-ChildItem -Path $sourcePath | Move-Item -Destination $destPath -Force
            }
        }
    }

    # Mover node_modules
    if (Test-Path $nodeModulesDir) {
        Move-Item -Path $nodeModulesDir -Destination $targetDir -Force
    }

    # Remover diretórios antigos
    if (Test-Path $sourceDir) {
        Remove-Item -Path $sourceDir -Recurse -Force
    }
}
catch {
    Write-Host "❌ Erro ao mover arquivos: $_" -ForegroundColor Red
    exit 1
}

Write-Host "✅ Frontend reorganizado com sucesso!" -ForegroundColor Green

# Verificar resultado
Write-Host "`n📋 Estrutura final:"
Get-ChildItem -Path $targetDir -Recurse -Depth 2 | Select-Object FullName

# Criar arquivo README.md com instruções
$readmePath = Join-Path $targetDir "README.md"
$readmeContent = @"
# Frontend Mega_Emu

## Estrutura

Este frontend segue uma arquitetura modular baseada em:
- Design Atômico para componentes
- Feature-First para organização de código
- Sistemas core isolados

## Desenvolvimento

1. Instalar dependências:
\`\`\`bash
npm install
\`\`\`

2. Iniciar em modo desenvolvimento:
\`\`\`bash
npm run dev
\`\`\`

3. Build para produção:
\`\`\`bash
npm run build
\`\`\`

## Documentação

Consulte \`docs/architecture/FRONTEND_ARCHITECTURE.md\` para detalhes completos da arquitetura.
"@

Set-Content -Path $readmePath -Value $readmeContent

Write-Host "`n📘 README.md criado em $readmePath"
