# Diretório de Relatórios

Este diretório contém todos os relatórios e logs gerados pelo Mega_Emu.

## Estrutura

- `*_test_report.txt`: Relatórios detalhados de testes de ROMs
- `*_test.log`: Logs de execução dos testes
- `performance_*.txt`: Relatórios de performance e benchmarks

## Uso

Os relatórios são gerados automaticamente pelas ferramentas em `tools/`:

```bash
# Teste de ROM do NES
python tools/test_nes_rom.py --rom path/to/rom.nes

# Análise de performance
python tools/analyze_performance.py --platform nes
```

## Retenção

Os relatórios são mantidos por 30 dias por padrão. Use `tools/cleanup.ps1` para limpar relatórios antigos.