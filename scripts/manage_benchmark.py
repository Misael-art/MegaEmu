#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de benchmark do emulador.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import platform
import statistics
import matplotlib.pyplot as plt
import numpy as np

# Configurações de benchmark
BENCHMARK_CONFIG = {
    'directories': {
        'benchmarks': 'benchmarks',
        'results': 'benchmarks/results',
        'reports': 'benchmarks/reports',
        'plots': 'benchmarks/plots'
    },
    'tests': {
        'cpu': {
            'name': 'CPU',
            'description': 'Testes de desempenho da CPU',
            'cases': [
                {
                    'name': 'instructions',
                    'description': 'Instruções por segundo',
                    'rom': 'benchmarks/roms/cpu_test.md',
                    'duration': 10,  # segundos
                    'metric': 'ips'  # instruções por segundo
                },
                {
                    'name': 'cycles',
                    'description': 'Ciclos por segundo',
                    'rom': 'benchmarks/roms/cpu_test.md',
                    'duration': 10,
                    'metric': 'cps'  # ciclos por segundo
                }
            ]
        },
        'memory': {
            'name': 'Memory',
            'description': 'Testes de desempenho da memória',
            'cases': [
                {
                    'name': 'read',
                    'description': 'Leituras por segundo',
                    'rom': 'benchmarks/roms/memory_test.md',
                    'duration': 10,
                    'metric': 'rps'  # leituras por segundo
                },
                {
                    'name': 'write',
                    'description': 'Escritas por segundo',
                    'rom': 'benchmarks/roms/memory_test.md',
                    'duration': 10,
                    'metric': 'wps'  # escritas por segundo
                }
            ]
        },
        'video': {
            'name': 'Video',
            'description': 'Testes de desempenho de vídeo',
            'cases': [
                {
                    'name': 'fps',
                    'description': 'Quadros por segundo',
                    'rom': 'benchmarks/roms/video_test.md',
                    'duration': 10,
                    'metric': 'fps'  # quadros por segundo
                },
                {
                    'name': 'sprites',
                    'description': 'Sprites por segundo',
                    'rom': 'benchmarks/roms/sprite_test.md',
                    'duration': 10,
                    'metric': 'sps'  # sprites por segundo
                }
            ]
        },
        'audio': {
            'name': 'Audio',
            'description': 'Testes de desempenho de áudio',
            'cases': [
                {
                    'name': 'latency',
                    'description': 'Latência de áudio',
                    'rom': 'benchmarks/roms/audio_test.md',
                    'duration': 10,
                    'metric': 'ms'  # milissegundos
                },
                {
                    'name': 'buffer',
                    'description': 'Underruns de buffer',
                    'rom': 'benchmarks/roms/audio_test.md',
                    'duration': 10,
                    'metric': 'count'  # contagem
                }
            ]
        }
    },
    'metrics': {
        'ips': {
            'name': 'Instruções por segundo',
            'unit': 'IPS',
            'format': '{:,.0f}',
            'higher_better': True
        },
        'cps': {
            'name': 'Ciclos por segundo',
            'unit': 'Hz',
            'format': '{:,.0f}',
            'higher_better': True
        },
        'rps': {
            'name': 'Leituras por segundo',
            'unit': 'RPS',
            'format': '{:,.0f}',
            'higher_better': True
        },
        'wps': {
            'name': 'Escritas por segundo',
            'unit': 'WPS',
            'format': '{:,.0f}',
            'higher_better': True
        },
        'fps': {
            'name': 'Quadros por segundo',
            'unit': 'FPS',
            'format': '{:.1f}',
            'higher_better': True
        },
        'sps': {
            'name': 'Sprites por segundo',
            'unit': 'SPS',
            'format': '{:,.0f}',
            'higher_better': True
        },
        'ms': {
            'name': 'Milissegundos',
            'unit': 'ms',
            'format': '{:.2f}',
            'higher_better': False
        },
        'count': {
            'name': 'Contagem',
            'unit': '',
            'format': '{:d}',
            'higher_better': False
        }
    }
}

def create_directories() -> bool:
    """
    Cria a estrutura de diretórios necessária.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        for directory in BENCHMARK_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def run_benchmark(test_type: str, case: str, iterations: int = 3) -> List[float]:
    """
    Executa um benchmark.

    Args:
        test_type: Tipo de teste.
        case: Caso de teste.
        iterations: Número de iterações.

    Returns:
        Lista com os resultados das iterações.
    """
    try:
        # Valida tipo de teste
        if test_type not in BENCHMARK_CONFIG['tests']:
            raise ValueError(f'Tipo de teste inválido: {test_type}')

        # Valida caso de teste
        test = BENCHMARK_CONFIG['tests'][test_type]
        case_config = next((c for c in test['cases'] if c['name'] == case), None)
        if not case_config:
            raise ValueError(f'Caso de teste inválido: {case}')

        # Executa benchmark
        results = []
        for i in range(iterations):
            print(f'\nIteração {i+1}/{iterations}...')

            # Executa emulador em modo benchmark
            result = subprocess.run([
                'python', 'scripts/manage_run.py',
                'run', case_config['rom'],
                'benchmark',
                '--duration', str(case_config['duration']),
                '--metric', case_config['metric']
            ], capture_output=True, text=True)

            if result.returncode != 0:
                print('Erro ao executar benchmark:', file=sys.stderr)
                print(result.stderr, file=sys.stderr)
                return []

            # Extrai resultado
            try:
                value = float(result.stdout.strip())
                results.append(value)
            except ValueError:
                print('Erro ao extrair resultado:', file=sys.stderr)
                print(result.stdout, file=sys.stderr)
                return []

        return results
    except Exception as e:
        print(f'Erro ao executar benchmark: {e}', file=sys.stderr)
        return []

def analyze_results(results: List[float], metric: str) -> Dict:
    """
    Analisa resultados de benchmark.

    Args:
        results: Lista de resultados.
        metric: Tipo de métrica.

    Returns:
        Dicionário com estatísticas.
    """
    try:
        if not results:
            return {}

        # Calcula estatísticas
        stats = {
            'min': min(results),
            'max': max(results),
            'mean': statistics.mean(results),
            'median': statistics.median(results),
            'stdev': statistics.stdev(results) if len(results) > 1 else 0.0,
            'samples': len(results)
        }

        # Formata valores
        metric_config = BENCHMARK_CONFIG['metrics'][metric]
        format_str = metric_config['format']
        stats['formatted'] = {
            'min': format_str.format(stats['min']),
            'max': format_str.format(stats['max']),
            'mean': format_str.format(stats['mean']),
            'median': format_str.format(stats['median']),
            'stdev': format_str.format(stats['stdev'])
        }

        return stats
    except Exception as e:
        print(f'Erro ao analisar resultados: {e}', file=sys.stderr)
        return {}

def generate_plot(test_type: str, case: str, results: Dict) -> bool:
    """
    Gera gráfico de resultados.

    Args:
        test_type: Tipo de teste.
        case: Caso de teste.
        results: Resultados do benchmark.

    Returns:
        True se o gráfico foi gerado com sucesso, False caso contrário.
    """
    try:
        # Configura plot
        plt.figure(figsize=(10, 6))
        plt.style.use('seaborn')

        # Plota resultados
        x = np.arange(len(results['data']))
        plt.plot(x, results['data'], 'b-', label='Valor')
        plt.axhline(y=results['stats']['mean'], color='r', linestyle='--',
                   label='Média')
        plt.fill_between(x,
                        results['stats']['mean'] - results['stats']['stdev'],
                        results['stats']['mean'] + results['stats']['stdev'],
                        color='r', alpha=0.2, label='Desvio Padrão')

        # Configura labels
        metric = BENCHMARK_CONFIG['tests'][test_type]['cases'][0]['metric']
        metric_config = BENCHMARK_CONFIG['metrics'][metric]
        plt.title(f'Benchmark: {test_type} - {case}')
        plt.xlabel('Iteração')
        plt.ylabel(f'{metric_config["name"]} ({metric_config["unit"]})')
        plt.legend()
        plt.grid(True)

        # Salva gráfico
        plot_file = os.path.join(
            BENCHMARK_CONFIG['directories']['plots'],
            f'benchmark_{test_type}_{case}.png'
        )
        plt.savefig(plot_file)
        plt.close()

        return True
    except Exception as e:
        print(f'Erro ao gerar gráfico: {e}', file=sys.stderr)
        return False

def generate_report(results: Dict) -> bool:
    """
    Gera relatório de benchmark.

    Args:
        results: Resultados dos benchmarks.

    Returns:
        True se o relatório foi gerado com sucesso, False caso contrário.
    """
    try:
        # Define arquivo de relatório
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        report_file = os.path.join(
            BENCHMARK_CONFIG['directories']['reports'],
            f'benchmark_report_{timestamp}.md'
        )

        with open(report_file, 'w', encoding='utf-8') as f:
            # Cabeçalho
            f.write('# Relatório de Benchmark\n\n')
            f.write(f'Data: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}\n\n')

            # Sistema
            f.write('## Sistema\n\n')
            f.write(f'- Sistema Operacional: {platform.system()}\n')
            f.write(f'- Versão: {platform.version()}\n')
            f.write(f'- Máquina: {platform.machine()}\n')
            f.write(f'- Processador: {platform.processor()}\n\n')

            # Resultados
            f.write('## Resultados\n\n')
            for test_type, test_results in results.items():
                f.write(f'### {BENCHMARK_CONFIG["tests"][test_type]["name"]}\n\n')
                f.write(f'{BENCHMARK_CONFIG["tests"][test_type]["description"]}\n\n')

                for case, case_results in test_results.items():
                    f.write(f'#### {case}\n\n')

                    # Descrição
                    case_config = next(c for c in BENCHMARK_CONFIG['tests'][test_type]['cases']
                                    if c['name'] == case)
                    f.write(f'{case_config["description"]}\n\n')

                    # Estatísticas
                    metric = case_config['metric']
                    metric_config = BENCHMARK_CONFIG['metrics'][metric]
                    stats = case_results['stats']['formatted']

                    f.write('| Métrica | Valor |\n')
                    f.write('|---------|-------|\n')
                    f.write(f'| Mínimo | {stats["min"]} {metric_config["unit"]} |\n')
                    f.write(f'| Máximo | {stats["max"]} {metric_config["unit"]} |\n')
                    f.write(f'| Média | {stats["mean"]} {metric_config["unit"]} |\n')
                    f.write(f'| Mediana | {stats["median"]} {metric_config["unit"]} |\n')
                    f.write(f'| Desvio Padrão | {stats["stdev"]} {metric_config["unit"]} |\n')
                    f.write(f'| Amostras | {case_results["stats"]["samples"]} |\n\n')

                    # Gráfico
                    plot_file = f'benchmark_{test_type}_{case}.png'
                    f.write(f'![Gráfico](../plots/{plot_file})\n\n')

        return True
    except Exception as e:
        print(f'Erro ao gerar relatório: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_benchmark.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  run [tipo] [caso] [iterações]')
        print('                        Executa benchmark', file=sys.stderr)
        print('  report               Gera relatório', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_directories() else 1

    elif command == 'run':
        # Executa todos os testes
        results = {}
        for test_type in BENCHMARK_CONFIG['tests']:
            if len(sys.argv) > 2 and sys.argv[2] != test_type:
                continue

            results[test_type] = {}
            for case in BENCHMARK_CONFIG['tests'][test_type]['cases']:
                if len(sys.argv) > 3 and sys.argv[3] != case['name']:
                    continue

                print(f'\nExecutando benchmark {test_type} - {case["name"]}...')
                iterations = int(sys.argv[4]) if len(sys.argv) > 4 else 3

                # Executa benchmark
                data = run_benchmark(test_type, case['name'], iterations)
                if not data:
                    continue

                # Analisa resultados
                stats = analyze_results(data, case['metric'])
                if not stats:
                    continue

                # Armazena resultados
                results[test_type][case['name']] = {
                    'data': data,
                    'stats': stats
                }

                # Gera gráfico
                generate_plot(test_type, case['name'], results[test_type][case['name']])

        # Gera relatório
        if results and not generate_report(results):
            return 1

        return 0

    elif command == 'report':
        # Carrega resultados anteriores
        results_dir = BENCHMARK_CONFIG['directories']['results']
        if not os.path.exists(results_dir):
            print('Nenhum resultado encontrado.', file=sys.stderr)
            return 1

        results = {}
        for test_type in BENCHMARK_CONFIG['tests']:
            results[test_type] = {}
            for case in BENCHMARK_CONFIG['tests'][test_type]['cases']:
                result_file = os.path.join(results_dir,
                                         f'benchmark_{test_type}_{case["name"]}.json')
                if not os.path.exists(result_file):
                    continue

                with open(result_file, 'r') as f:
                    results[test_type][case['name']] = json.load(f)

        # Gera relatório
        if results and not generate_report(results):
            return 1

        return 0

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
