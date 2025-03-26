#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de monitoramento do emulador.
"""

import os
import sys
import json
import shutil
import logging
import logging.handlers
import subprocess
import psutil
import time
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

# Configurações de monitoramento
MONITOR_CONFIG = {
    'directories': {
        'monitor': 'monitor',
        'metrics': 'monitor/metrics',
        'alerts': 'monitor/alerts',
        'reports': 'monitor/reports',
        'logs': 'monitor/logs'
    },
    'metrics': {
        'cpu': {
            'enabled': True,
            'interval': 1.0,  # segundos
            'thresholds': {
                'warning': 80.0,  # porcentagem
                'critical': 90.0
            }
        },
        'memory': {
            'enabled': True,
            'interval': 1.0,
            'thresholds': {
                'warning': 80.0,  # porcentagem
                'critical': 90.0
            }
        },
        'fps': {
            'enabled': True,
            'interval': 1.0,
            'thresholds': {
                'warning': 55.0,  # frames por segundo
                'critical': 50.0
            }
        },
        'latency': {
            'enabled': True,
            'interval': 1.0,
            'thresholds': {
                'warning': 16.67,  # milissegundos (60 FPS)
                'critical': 33.33  # milissegundos (30 FPS)
            }
        }
    },
    'alerts': {
        'enabled': True,
        'channels': {
            'console': True,
            'file': True,
            'email': False,
            'webhook': False
        },
        'throttling': {
            'enabled': True,
            'window': 300,  # segundos
            'max_alerts': 10
        },
        'templates': {
            'warning': '[AVISO] {metric}: {value:.2f} {unit} (limite: {threshold:.2f} {unit})',
            'critical': '[CRÍTICO] {metric}: {value:.2f} {unit} (limite: {threshold:.2f} {unit})'
        }
    },
    'reports': {
        'enabled': True,
        'interval': 3600,  # segundos
        'formats': ['txt', 'json', 'csv'],
        'metrics': ['cpu', 'memory', 'fps', 'latency'],
        'statistics': ['min', 'max', 'avg', 'p95', 'p99']
    },
    'logging': {
        'enabled': True,
        'level': 'INFO',
        'format': '%(asctime)s [%(levelname)s] %(message)s',
        'rotation': {
            'when': 'D',
            'interval': 1,
            'backupCount': 30
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
        for directory in MONITOR_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def monitor_process(pid: int) -> bool:
    """
    Monitora um processo.

    Args:
        pid: ID do processo.

    Returns:
        True se o monitoramento foi bem sucedido, False caso contrário.
    """
    try:
        print(f'\nMonitorando processo {pid}...')

        # Obtém processo
        process = psutil.Process(pid)

        # Inicializa métricas
        metrics = {
            'cpu': [],
            'memory': [],
            'fps': [],
            'latency': []
        }

        # Monitora processo
        start_time = time.time()
        last_report = start_time

        while True:
            try:
                # CPU
                if MONITOR_CONFIG['metrics']['cpu']['enabled']:
                    cpu_percent = process.cpu_percent()
                    metrics['cpu'].append({
                        'timestamp': time.time(),
                        'value': cpu_percent
                    })

                    # Verifica limites
                    if cpu_percent >= MONITOR_CONFIG['metrics']['cpu']['thresholds']['critical']:
                        alert('cpu', cpu_percent, 'critical', '%')
                    elif cpu_percent >= MONITOR_CONFIG['metrics']['cpu']['thresholds']['warning']:
                        alert('cpu', cpu_percent, 'warning', '%')

                # Memória
                if MONITOR_CONFIG['metrics']['memory']['enabled']:
                    memory_percent = process.memory_percent()
                    metrics['memory'].append({
                        'timestamp': time.time(),
                        'value': memory_percent
                    })

                    # Verifica limites
                    if memory_percent >= MONITOR_CONFIG['metrics']['memory']['thresholds']['critical']:
                        alert('memory', memory_percent, 'critical', '%')
                    elif memory_percent >= MONITOR_CONFIG['metrics']['memory']['thresholds']['warning']:
                        alert('memory', memory_percent, 'warning', '%')

                # FPS e latência são métricas específicas do emulador
                # que precisam ser obtidas através de IPC ou logs

                # Gera relatório periódico
                current_time = time.time()
                if MONITOR_CONFIG['reports']['enabled'] and \
                   current_time - last_report >= MONITOR_CONFIG['reports']['interval']:
                    generate_report(metrics)
                    last_report = current_time

                # Aguarda próxima iteração
                time.sleep(min(
                    MONITOR_CONFIG['metrics']['cpu']['interval'],
                    MONITOR_CONFIG['metrics']['memory']['interval'],
                    MONITOR_CONFIG['metrics']['fps']['interval'],
                    MONITOR_CONFIG['metrics']['latency']['interval']
                ))

            except psutil.NoSuchProcess:
                print(f'\nProcesso {pid} encerrado.')
                break

        # Gera relatório final
        if MONITOR_CONFIG['reports']['enabled']:
            generate_report(metrics)

        return True
    except Exception as e:
        print(f'Erro ao monitorar processo: {e}', file=sys.stderr)
        return False

def alert(metric: str, value: float, level: str, unit: str) -> None:
    """
    Gera um alerta.

    Args:
        metric: Nome da métrica.
        value: Valor atual.
        level: Nível do alerta (warning/critical).
        unit: Unidade de medida.
    """
    try:
        if not MONITOR_CONFIG['alerts']['enabled']:
            return

        # Formata mensagem
        threshold = MONITOR_CONFIG['metrics'][metric]['thresholds'][level]
        message = MONITOR_CONFIG['alerts']['templates'][level].format(
            metric=metric,
            value=value,
            threshold=threshold,
            unit=unit
        )

        # Verifica throttling
        alert_path = os.path.join(
            MONITOR_CONFIG['directories']['alerts'],
            f'{metric}_{level}.json'
        )

        if MONITOR_CONFIG['alerts']['throttling']['enabled']:
            try:
                with open(alert_path) as f:
                    alerts = json.load(f)
            except:
                alerts = []

            # Remove alertas antigos
            current_time = time.time()
            window_start = current_time - MONITOR_CONFIG['alerts']['throttling']['window']
            alerts = [
                alert for alert in alerts
                if alert['timestamp'] >= window_start
            ]

            # Verifica limite
            if len(alerts) >= MONITOR_CONFIG['alerts']['throttling']['max_alerts']:
                return

            # Adiciona novo alerta
            alerts.append({
                'timestamp': current_time,
                'value': value
            })

            # Salva alertas
            with open(alert_path, 'w') as f:
                json.dump(alerts, f, indent=2)

        # Envia alerta
        if MONITOR_CONFIG['alerts']['channels']['console']:
            print(message)

        if MONITOR_CONFIG['alerts']['channels']['file']:
            log_path = os.path.join(
                MONITOR_CONFIG['directories']['alerts'],
                'alerts.log'
            )
            with open(log_path, 'a') as f:
                f.write(f'{datetime.now().isoformat()} {message}\n')

        if MONITOR_CONFIG['alerts']['channels']['email']:
            # TODO: Implementar envio por email
            pass

        if MONITOR_CONFIG['alerts']['channels']['webhook']:
            # TODO: Implementar envio por webhook
            pass

    except Exception as e:
        print(f'Erro ao gerar alerta: {e}', file=sys.stderr)

def generate_report(metrics: Dict[str, List[Dict[str, Union[float, str]]]]) -> bool:
    """
    Gera relatório de métricas.

    Args:
        metrics: Dicionário com métricas coletadas.

    Returns:
        True se o relatório foi gerado com sucesso, False caso contrário.
    """
    try:
        if not MONITOR_CONFIG['reports']['enabled']:
            return True

        print('\nGerando relatório...')

        # Calcula estatísticas
        stats = {}
        for metric in MONITOR_CONFIG['reports']['metrics']:
            if not metrics[metric]:
                continue

            values = [m['value'] for m in metrics[metric]]
            timestamps = [m['timestamp'] for m in metrics[metric]]

            stats[metric] = {
                'start_time': datetime.fromtimestamp(timestamps[0]).isoformat(),
                'end_time': datetime.fromtimestamp(timestamps[-1]).isoformat(),
                'samples': len(values),
                'min': min(values),
                'max': max(values),
                'avg': sum(values) / len(values),
                'p95': sorted(values)[int(len(values) * 0.95)],
                'p99': sorted(values)[int(len(values) * 0.99)]
            }

        # Gera relatórios
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')

        for format in MONITOR_CONFIG['reports']['formats']:
            report_path = os.path.join(
                MONITOR_CONFIG['directories']['reports'],
                f'report_{timestamp}.{format}'
            )

            if format == 'txt':
                with open(report_path, 'w') as f:
                    f.write('=== Relatório de Monitoramento ===\n\n')
                    for metric, data in stats.items():
                        f.write(f'Métrica: {metric}\n')
                        for key, value in data.items():
                            if isinstance(value, float):
                                f.write(f'  {key}: {value:.2f}\n')
                            else:
                                f.write(f'  {key}: {value}\n')
                        f.write('\n')

            elif format == 'json':
                with open(report_path, 'w') as f:
                    json.dump(stats, f, indent=2)

            elif format == 'csv':
                with open(report_path, 'w') as f:
                    # Cabeçalho
                    headers = ['metric']
                    for metric in stats.values():
                        headers.extend(metric.keys())
                        break
                    f.write(','.join(headers) + '\n')

                    # Dados
                    for metric, data in stats.items():
                        row = [metric]
                        for value in data.values():
                            if isinstance(value, float):
                                row.append(f'{value:.2f}')
                            else:
                                row.append(str(value))
                        f.write(','.join(row) + '\n')

        print('Relatório gerado com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao gerar relatório: {e}', file=sys.stderr)
        return False

def setup_logging() -> bool:
    """
    Configura sistema de logging.

    Returns:
        True se a configuração foi bem sucedida, False caso contrário.
    """
    try:
        if not MONITOR_CONFIG['logging']['enabled']:
            return True

        # Configura logger
        logger = logging.getLogger('monitor')
        logger.setLevel(MONITOR_CONFIG['logging']['level'])

        # Handler para arquivo
        log_path = os.path.join(
            MONITOR_CONFIG['directories']['logs'],
            'monitor.log'
        )
        handler = logging.handlers.TimedRotatingFileHandler(
            log_path,
            when=MONITOR_CONFIG['logging']['rotation']['when'],
            interval=MONITOR_CONFIG['logging']['rotation']['interval'],
            backupCount=MONITOR_CONFIG['logging']['rotation']['backupCount']
        )
        handler.setFormatter(logging.Formatter(
            MONITOR_CONFIG['logging']['format']
        ))
        logger.addHandler(handler)

        return True
    except Exception as e:
        print(f'Erro ao configurar logging: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_monitor.py <comando> [pid]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init      Cria estrutura de diretórios', file=sys.stderr)
        print('  monitor   Monitora processo', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        if not create_directories():
            return 1
        if not setup_logging():
            return 1
        return 0

    elif command == 'monitor':
        if len(sys.argv) < 3:
            print('PID não especificado.', file=sys.stderr)
            return 1

        try:
            pid = int(sys.argv[2])
        except ValueError:
            print('PID inválido.', file=sys.stderr)
            return 1

        if not monitor_process(pid):
            return 1
        return 0

    else:
        print('Comando inválido.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
