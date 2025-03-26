#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os registros de desempenho do emulador.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import statistics

# Tipos de métricas de desempenho
METRIC_TYPES = {
    'fps': {
        'description': 'Quadros por segundo',
        'unit': 'fps',
        'threshold': 60.0
    },
    'frame_time': {
        'description': 'Tempo de renderização por quadro',
        'unit': 'ms',
        'threshold': 16.67  # ~60fps
    },
    'cpu_usage': {
        'description': 'Uso de CPU',
        'unit': '%',
        'threshold': 80.0
    },
    'memory_usage': {
        'description': 'Uso de memória',
        'unit': 'MB',
        'threshold': 512.0
    },
    'audio_latency': {
        'description': 'Latência de áudio',
        'unit': 'ms',
        'threshold': 20.0
    }
}

def create_performance_directories() -> bool:
    """
    Cria a estrutura de diretórios para registros de desempenho.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        # Cria diretórios principais
        dirs = [
            'performance',
            'performance/logs',
            'performance/reports',
            'performance/graphs'
        ]

        for directory in dirs:
            os.makedirs(directory, exist_ok=True)

        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def validate_metric(metric_type: str, value: float) -> Tuple[bool, Optional[str]]:
    """
    Valida uma métrica de desempenho.

    Args:
        metric_type: Tipo da métrica.
        value: Valor da métrica.

    Returns:
        Uma tupla (válido, mensagem) onde válido é um booleano e mensagem é None
        se válido ou uma mensagem de erro caso contrário.
    """
    try:
        # Verifica se o tipo de métrica é suportado
        if metric_type not in METRIC_TYPES:
            return False, f'Tipo de métrica não suportado: {metric_type}'

        # Verifica se o valor é um número válido
        if not isinstance(value, (int, float)):
            return False, 'Valor deve ser um número'

        # Verifica se o valor é positivo
        if value < 0:
            return False, 'Valor deve ser positivo'

        return True, None
    except Exception as e:
        return False, f'Erro ao validar métrica: {e}'

def record_metric(rom_name: str, metric_type: str, value: float,
                 timestamp: Optional[str] = None) -> bool:
    """
    Registra uma métrica de desempenho.

    Args:
        rom_name: Nome da ROM.
        metric_type: Tipo da métrica.
        value: Valor da métrica.
        timestamp: Data e hora do registro (opcional).

    Returns:
        True se a métrica foi registrada com sucesso, False caso contrário.
    """
    try:
        # Valida a métrica
        valid, message = validate_metric(metric_type, value)
        if not valid:
            print(f'Métrica inválida: {message}', file=sys.stderr)
            return False

        # Define timestamp se não fornecido
        if timestamp is None:
            timestamp = datetime.now().isoformat()

        # Carrega métricas existentes
        metrics = load_metrics(rom_name)

        # Adiciona nova métrica
        metrics.append({
            'type': metric_type,
            'value': value,
            'timestamp': timestamp,
            'threshold': METRIC_TYPES[metric_type]['threshold']
        })

        # Salva métricas
        return save_metrics(rom_name, metrics)
    except Exception as e:
        print(f'Erro ao registrar métrica: {e}', file=sys.stderr)
        return False

def load_metrics(rom_name: str) -> List[Dict]:
    """
    Carrega métricas de uma ROM.

    Args:
        rom_name: Nome da ROM.

    Returns:
        Lista de métricas.
    """
    try:
        metrics_path = f'performance/logs/{rom_name}.json'
        if os.path.exists(metrics_path):
            with open(metrics_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        return []
    except Exception as e:
        print(f'Erro ao carregar métricas: {e}', file=sys.stderr)
        return []

def save_metrics(rom_name: str, metrics: List[Dict]) -> bool:
    """
    Salva métricas de uma ROM.

    Args:
        rom_name: Nome da ROM.
        metrics: Lista de métricas.

    Returns:
        True se as métricas foram salvas com sucesso, False caso contrário.
    """
    try:
        metrics_path = f'performance/logs/{rom_name}.json'
        with open(metrics_path, 'w', encoding='utf-8') as f:
            json.dump(metrics, f, indent=2, ensure_ascii=False)
        return True
    except Exception as e:
        print(f'Erro ao salvar métricas: {e}', file=sys.stderr)
        return False

def analyze_metrics(rom_name: str, metric_type: Optional[str] = None,
                   start_time: Optional[str] = None,
                   end_time: Optional[str] = None) -> Dict:
    """
    Analisa métricas de desempenho.

    Args:
        rom_name: Nome da ROM.
        metric_type: Tipo de métrica para filtrar (opcional).
        start_time: Data/hora inicial para filtrar (opcional).
        end_time: Data/hora final para filtrar (opcional).

    Returns:
        Dicionário com análise das métricas.
    """
    try:
        # Carrega métricas
        metrics = load_metrics(rom_name)
        if not metrics:
            return {}

        # Filtra por tipo de métrica
        if metric_type:
            metrics = [m for m in metrics if m['type'] == metric_type]

        # Filtra por período
        if start_time:
            metrics = [m for m in metrics if m['timestamp'] >= start_time]
        if end_time:
            metrics = [m for m in metrics if m['timestamp'] <= end_time]

        # Agrupa métricas por tipo
        grouped_metrics = {}
        for metric in metrics:
            metric_type = metric['type']
            if metric_type not in grouped_metrics:
                grouped_metrics[metric_type] = []
            grouped_metrics[metric_type].append(metric['value'])

        # Calcula estatísticas
        analysis = {}
        for metric_type, values in grouped_metrics.items():
            analysis[metric_type] = {
                'count': len(values),
                'min': min(values),
                'max': max(values),
                'mean': statistics.mean(values),
                'median': statistics.median(values),
                'std_dev': statistics.stdev(values) if len(values) > 1 else 0,
                'threshold': METRIC_TYPES[metric_type]['threshold'],
                'unit': METRIC_TYPES[metric_type]['unit'],
                'description': METRIC_TYPES[metric_type]['description']
            }

        return analysis
    except Exception as e:
        print(f'Erro ao analisar métricas: {e}', file=sys.stderr)
        return {}

def generate_report(rom_name: str, analysis: Dict, output_path: str) -> bool:
    """
    Gera um relatório de desempenho.

    Args:
        rom_name: Nome da ROM.
        analysis: Análise das métricas.
        output_path: Caminho do arquivo de saída.

    Returns:
        True se o relatório foi gerado com sucesso, False caso contrário.
    """
    try:
        # Cria diretório de saída se necessário
        os.makedirs(os.path.dirname(output_path), exist_ok=True)

        # Gera relatório em formato Markdown
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(f'# Relatório de Desempenho - {rom_name}\n\n')
            f.write(f'Data: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}\n\n')

            for metric_type, stats in analysis.items():
                f.write(f'## {stats["description"]}\n\n')
                f.write(f'- **Unidade**: {stats["unit"]}\n')
                f.write(f'- **Limite**: {stats["threshold"]}\n')
                f.write(f'- **Amostras**: {stats["count"]}\n')
                f.write(f'- **Mínimo**: {stats["min"]:.2f}\n')
                f.write(f'- **Máximo**: {stats["max"]:.2f}\n')
                f.write(f'- **Média**: {stats["mean"]:.2f}\n')
                f.write(f'- **Mediana**: {stats["median"]:.2f}\n')
                f.write(f'- **Desvio Padrão**: {stats["std_dev"]:.2f}\n\n')

                # Avalia desempenho
                if stats["mean"] > stats["threshold"]:
                    if metric_type in ['frame_time', 'cpu_usage', 'memory_usage', 'audio_latency']:
                        f.write('⚠️ **Alerta**: Média acima do limite recomendado\n\n')
                elif metric_type == 'fps' and stats["mean"] < stats["threshold"]:
                    f.write('⚠️ **Alerta**: FPS abaixo do limite recomendado\n\n')
                else:
                    f.write('✅ Desempenho dentro dos limites recomendados\n\n')

        print(f'Relatório gerado em: {output_path}')
        return True
    except Exception as e:
        print(f'Erro ao gerar relatório: {e}', file=sys.stderr)
        return False

def generate_graphs(rom_name: str, analysis: Dict, output_dir: str) -> bool:
    """
    Gera gráficos de desempenho.

    Args:
        rom_name: Nome da ROM.
        analysis: Análise das métricas.
        output_dir: Diretório de saída.

    Returns:
        True se os gráficos foram gerados com sucesso, False caso contrário.
    """
    try:
        import matplotlib.pyplot as plt
        import seaborn as sns
        from matplotlib.dates import DateFormatter
        import pandas as pd

        # Carrega métricas
        metrics = load_metrics(rom_name)
        if not metrics:
            return False

        # Cria diretório de saída se necessário
        os.makedirs(output_dir, exist_ok=True)

        # Configura estilo dos gráficos
        plt.style.use('seaborn')
        sns.set_palette('husl')

        # Para cada tipo de métrica
        for metric_type in METRIC_TYPES:
            # Filtra métricas do tipo atual
            type_metrics = [m for m in metrics if m['type'] == metric_type]
            if not type_metrics:
                continue

            # Cria DataFrame
            df = pd.DataFrame(type_metrics)
            df['timestamp'] = pd.to_datetime(df['timestamp'])

            # Gráfico de linha temporal
            plt.figure(figsize=(12, 6))
            plt.plot(df['timestamp'], df['value'], marker='o')
            plt.axhline(y=METRIC_TYPES[metric_type]['threshold'],
                       color='r', linestyle='--', label='Limite')
            plt.title(f'{METRIC_TYPES[metric_type]["description"]} ao Longo do Tempo')
            plt.xlabel('Data/Hora')
            plt.ylabel(f'{METRIC_TYPES[metric_type]["description"]} ({METRIC_TYPES[metric_type]["unit"]})')
            plt.legend()
            plt.grid(True)
            plt.xticks(rotation=45)
            plt.tight_layout()
            plt.savefig(f'{output_dir}/{rom_name}_{metric_type}_timeline.png')
            plt.close()

            # Histograma
            plt.figure(figsize=(10, 6))
            sns.histplot(data=df, x='value', bins=30)
            plt.axvline(x=METRIC_TYPES[metric_type]['threshold'],
                       color='r', linestyle='--', label='Limite')
            plt.title(f'Distribuição de {METRIC_TYPES[metric_type]["description"]}')
            plt.xlabel(f'{METRIC_TYPES[metric_type]["description"]} ({METRIC_TYPES[metric_type]["unit"]})')
            plt.ylabel('Frequência')
            plt.legend()
            plt.grid(True)
            plt.tight_layout()
            plt.savefig(f'{output_dir}/{rom_name}_{metric_type}_histogram.png')
            plt.close()

        return True
    except ImportError:
        print('Bibliotecas matplotlib, seaborn e pandas são necessárias para gerar gráficos.',
              file=sys.stderr)
        return False
    except Exception as e:
        print(f'Erro ao gerar gráficos: {e}', file=sys.stderr)
        return False

def print_analysis(analysis: Dict) -> None:
    """
    Imprime análise de métricas.

    Args:
        analysis: Dicionário com análise das métricas.
    """
    for metric_type, stats in analysis.items():
        print(f'\n{stats["description"]}:')
        print(f'  Unidade: {stats["unit"]}')
        print(f'  Limite: {stats["threshold"]}')
        print(f'  Amostras: {stats["count"]}')
        print(f'  Mínimo: {stats["min"]:.2f}')
        print(f'  Máximo: {stats["max"]:.2f}')
        print(f'  Média: {stats["mean"]:.2f}')
        print(f'  Mediana: {stats["median"]:.2f}')
        print(f'  Desvio Padrão: {stats["std_dev"]:.2f}')

        if stats["mean"] > stats["threshold"]:
            if metric_type in ['frame_time', 'cpu_usage', 'memory_usage', 'audio_latency']:
                print('  ⚠️ Alerta: Média acima do limite recomendado')
        elif metric_type == 'fps' and stats["mean"] < stats["threshold"]:
            print('  ⚠️ Alerta: FPS abaixo do limite recomendado')
        else:
            print('  ✅ Desempenho dentro dos limites recomendados')

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_performance.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  record <rom> <tipo> <valor> [timestamp]')
        print('                        Registra uma métrica', file=sys.stderr)
        print('  analyze <rom> [tipo] [início] [fim]')
        print('                        Analisa métricas', file=sys.stderr)
        print('  report <rom> <arquivo>')
        print('                        Gera relatório', file=sys.stderr)
        print('  graphs <rom> <diretório>')
        print('                        Gera gráficos', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_performance_directories() else 1

    elif command == 'record' and len(sys.argv) > 4:
        timestamp = sys.argv[5] if len(sys.argv) > 5 else None
        return 0 if record_metric(
            sys.argv[2],  # rom
            sys.argv[3],  # tipo
            float(sys.argv[4]),  # valor
            timestamp
        ) else 1

    elif command == 'analyze' and len(sys.argv) > 2:
        metric_type = sys.argv[3] if len(sys.argv) > 3 else None
        start_time = sys.argv[4] if len(sys.argv) > 4 else None
        end_time = sys.argv[5] if len(sys.argv) > 5 else None
        analysis = analyze_metrics(sys.argv[2], metric_type, start_time, end_time)
        if analysis:
            print_analysis(analysis)
            return 0
        return 1

    elif command == 'report' and len(sys.argv) > 3:
        analysis = analyze_metrics(sys.argv[2])
        if not analysis:
            return 1
        return 0 if generate_report(sys.argv[2], analysis, sys.argv[3]) else 1

    elif command == 'graphs' and len(sys.argv) > 3:
        analysis = analyze_metrics(sys.argv[2])
        if not analysis:
            return 1
        return 0 if generate_graphs(sys.argv[2], analysis, sys.argv[3]) else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
