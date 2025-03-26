#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de perfil do emulador.
"""

import os
import sys
import json
import shutil
import logging
import logging.handlers
import subprocess
import psutil
import cProfile
import pstats
import yappi
import py_spy
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

# Configurações de perfil
PROFILE_CONFIG = {
    'directories': {
        'profile': 'profile',
        'stats': 'profile/stats',
        'reports': 'profile/reports',
        'flamegraphs': 'profile/flamegraphs',
        'logs': 'profile/logs'
    },
    'functions': {
        'cpu_execute': {
            'enabled': True,
            'description': 'Execução da CPU',
            'metrics': [
                'calls',
                'time',
                'time_per_call'
            ]
        },
        'vdp_render': {
            'enabled': True,
            'description': 'Renderização do VDP',
            'metrics': [
                'calls',
                'time',
                'time_per_call'
            ]
        },
        'audio_process': {
            'enabled': True,
            'description': 'Processamento de áudio',
            'metrics': [
                'calls',
                'time',
                'time_per_call'
            ]
        },
        'input_process': {
            'enabled': True,
            'description': 'Processamento de entrada',
            'metrics': [
                'calls',
                'time',
                'time_per_call'
            ]
        }
    },
    'sampling': {
        'enabled': True,
        'interval': 0.001,  # segundos
        'duration': 60,     # segundos
        'native': True,
        'threads': True,
        'gil': True
    },
    'tracing': {
        'enabled': True,
        'builtins': False,
        'threads': True,
        'memory': True,
        'cpu_time': True
    },
    'reporting': {
        'enabled': True,
        'format': 'text',
        'sort_by': 'cumulative',
        'limit': 50,
        'sections': [
            'overview',
            'hotspots',
            'callgraph',
            'timeline'
        ]
    },
    'flamegraph': {
        'enabled': True,
        'format': 'svg',
        'width': 1200,
        'height': 16,
        'colors': {
            'python': '#4584b6',
            'native': '#ffde57'
        }
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
        for directory in PROFILE_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def profile_functions(pid: int) -> bool:
    """
    Perfila funções específicas.

    Args:
        pid: ID do processo.

    Returns:
        True se o perfilamento foi bem sucedido, False caso contrário.
    """
    try:
        print(f'\nPerfilando funções do processo {pid}...')

        # Configura perfilador
        profiler = cProfile.Profile()
        profiler.enable()

        # Anexa ao processo
        process = psutil.Process(pid)
        process.suspend()

        try:
            # Injeta código de perfilamento
            for func_name, config in PROFILE_CONFIG['functions'].items():
                if not config['enabled']:
                    continue

                # Injeta wrapper de perfilamento
                inject_profiling(process, func_name)

            # Resume processo
            process.resume()

            # Aguarda duração do perfilamento
            duration = PROFILE_CONFIG['sampling']['duration']
            print(f'Perfilando por {duration} segundos...')
            import time
            time.sleep(duration)

        finally:
            # Desabilita perfilador
            profiler.disable()

            # Salva estatísticas
            timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
            stats_path = os.path.join(
                PROFILE_CONFIG['directories']['stats'],
                f'functions_{timestamp}.stats'
            )
            profiler.dump_stats(stats_path)

            # Gera relatório
            report_path = os.path.join(
                PROFILE_CONFIG['directories']['reports'],
                f'functions_{timestamp}.txt'
            )
            with open(report_path, 'w') as f:
                stats = pstats.Stats(profiler, stream=f)
                stats.sort_stats(PROFILE_CONFIG['reporting']['sort_by'])
                stats.print_stats(PROFILE_CONFIG['reporting']['limit'])

            print(f'Estatísticas salvas em: {stats_path}')
            print(f'Relatório gerado em: {report_path}')

        return True
    except Exception as e:
        print(f'Erro ao perfilar funções: {e}', file=sys.stderr)
        return False

def sample_process(pid: int) -> bool:
    """
    Amostra processo em execução.

    Args:
        pid: ID do processo.

    Returns:
        True se a amostragem foi bem sucedida, False caso contrário.
    """
    try:
        if not PROFILE_CONFIG['sampling']['enabled']:
            return True

        print(f'\nAmostrando processo {pid}...')

        # Configura sampler
        config = PROFILE_CONFIG['sampling']
        sampler = py_spy.Sampler(
            pid=pid,
            native=config['native'],
            threads=config['threads'],
            gil=config['gil']
        )

        # Inicia amostragem
        samples = sampler.start(
            duration=config['duration'],
            interval=config['interval']
        )

        # Salva amostras
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        samples_path = os.path.join(
            PROFILE_CONFIG['directories']['stats'],
            f'samples_{timestamp}.json'
        )
        with open(samples_path, 'w') as f:
            json.dump(samples, f, indent=2)

        # Gera flamegraph
        if PROFILE_CONFIG['flamegraph']['enabled']:
            flamegraph_path = os.path.join(
                PROFILE_CONFIG['directories']['flamegraphs'],
                f'flamegraph_{timestamp}.{PROFILE_CONFIG["flamegraph"]["format"]}'
            )
            sampler.generate_flamegraph(
                samples,
                flamegraph_path,
                width=PROFILE_CONFIG['flamegraph']['width'],
                height=PROFILE_CONFIG['flamegraph']['height'],
                colors=PROFILE_CONFIG['flamegraph']['colors']
            )
            print(f'Flamegraph gerado em: {flamegraph_path}')

        print(f'Amostras salvas em: {samples_path}')
        return True
    except Exception as e:
        print(f'Erro ao amostrar processo: {e}', file=sys.stderr)
        return False

def trace_process(pid: int) -> bool:
    """
    Rastreia processo em execução.

    Args:
        pid: ID do processo.

    Returns:
        True se o rastreamento foi bem sucedido, False caso contrário.
    """
    try:
        if not PROFILE_CONFIG['tracing']['enabled']:
            return True

        print(f'\nRastreando processo {pid}...')

        # Configura tracer
        yappi.set_clock_type('cpu')
        yappi.start(
            builtins=PROFILE_CONFIG['tracing']['builtins'],
            profile_threads=PROFILE_CONFIG['tracing']['threads']
        )

        try:
            # Aguarda duração do rastreamento
            duration = PROFILE_CONFIG['sampling']['duration']
            print(f'Rastreando por {duration} segundos...')
            import time
            time.sleep(duration)

        finally:
            # Para tracer
            yappi.stop()

            # Obtém estatísticas
            stats = yappi.get_func_stats()

            # Salva estatísticas
            timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
            stats_path = os.path.join(
                PROFILE_CONFIG['directories']['stats'],
                f'trace_{timestamp}.stats'
            )
            stats.save(stats_path, type='pstat')

            # Gera relatório
            report_path = os.path.join(
                PROFILE_CONFIG['directories']['reports'],
                f'trace_{timestamp}.txt'
            )
            stats.print_all(
                out=open(report_path, 'w'),
                columns={
                    0: ('name', 80),
                    1: ('ncall', 10),
                    2: ('tsub', 8),
                    3: ('ttot', 8),
                    4: ('tavg', 8)
                }
            )

            print(f'Estatísticas salvas em: {stats_path}')
            print(f'Relatório gerado em: {report_path}')

        return True
    except Exception as e:
        print(f'Erro ao rastrear processo: {e}', file=sys.stderr)
        return False

def generate_report(stats_file: str) -> bool:
    """
    Gera relatório de perfilamento.

    Args:
        stats_file: Arquivo de estatísticas.

    Returns:
        True se o relatório foi gerado com sucesso, False caso contrário.
    """
    try:
        if not PROFILE_CONFIG['reporting']['enabled']:
            return True

        print(f'\nGerando relatório para {stats_file}...')

        # Carrega estatísticas
        stats = pstats.Stats(stats_file)

        # Gera relatório
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        report_path = os.path.join(
            PROFILE_CONFIG['directories']['reports'],
            f'report_{timestamp}.{PROFILE_CONFIG["reporting"]["format"]}'
        )

        with open(report_path, 'w') as f:
            # Cabeçalho
            f.write('Relatório de Perfilamento\n')
            f.write('=======================\n\n')
            f.write(f'Gerado em: {datetime.now()}\n')
            f.write(f'Arquivo: {stats_file}\n\n')

            # Seções
            for section in PROFILE_CONFIG['reporting']['sections']:
                if section == 'overview':
                    f.write('Visão Geral\n')
                    f.write('-----------\n\n')
                    stats.strip_dirs().sort_stats('cumulative').print_stats(
                        PROFILE_CONFIG['reporting']['limit'],
                        file=f
                    )
                    f.write('\n')

                elif section == 'hotspots':
                    f.write('Pontos Críticos\n')
                    f.write('--------------\n\n')
                    stats.strip_dirs().sort_stats('time').print_stats(
                        PROFILE_CONFIG['reporting']['limit'],
                        file=f
                    )
                    f.write('\n')

                elif section == 'callgraph':
                    f.write('Grafo de Chamadas\n')
                    f.write('----------------\n\n')
                    stats.strip_dirs().sort_stats('cumulative').print_callers(
                        PROFILE_CONFIG['reporting']['limit'],
                        file=f
                    )
                    f.write('\n')

                elif section == 'timeline':
                    f.write('Linha do Tempo\n')
                    f.write('--------------\n\n')
                    stats.strip_dirs().sort_stats('time').print_stats(
                        PROFILE_CONFIG['reporting']['limit'],
                        file=f
                    )
                    f.write('\n')

        print(f'Relatório gerado em: {report_path}')
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
        if not PROFILE_CONFIG['logging']['enabled']:
            return True

        # Configura logger
        logger = logging.getLogger('profile')
        logger.setLevel(PROFILE_CONFIG['logging']['level'])

        # Handler para arquivo
        log_path = os.path.join(
            PROFILE_CONFIG['directories']['logs'],
            'profile.log'
        )
        handler = logging.handlers.TimedRotatingFileHandler(
            log_path,
            when=PROFILE_CONFIG['logging']['rotation']['when'],
            interval=PROFILE_CONFIG['logging']['rotation']['interval'],
            backupCount=PROFILE_CONFIG['logging']['rotation']['backupCount']
        )
        handler.setFormatter(logging.Formatter(
            PROFILE_CONFIG['logging']['format']
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
        print('Uso: manage_profile.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  functions <pid>       Perfila funções', file=sys.stderr)
        print('  sample <pid>          Amostra processo', file=sys.stderr)
        print('  trace <pid>           Rastreia processo', file=sys.stderr)
        print('  report <arquivo>      Gera relatório', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        if not create_directories():
            return 1
        if not setup_logging():
            return 1
        return 0

    elif command == 'functions' and len(sys.argv) > 2:
        try:
            pid = int(sys.argv[2])
        except ValueError:
            print('Erro: PID deve ser um número inteiro.', file=sys.stderr)
            return 1

        if not psutil.pid_exists(pid):
            print(f'Erro: processo {pid} não encontrado.', file=sys.stderr)
            return 1

        return 0 if profile_functions(pid) else 1

    elif command == 'sample' and len(sys.argv) > 2:
        try:
            pid = int(sys.argv[2])
        except ValueError:
            print('Erro: PID deve ser um número inteiro.', file=sys.stderr)
            return 1

        if not psutil.pid_exists(pid):
            print(f'Erro: processo {pid} não encontrado.', file=sys.stderr)
            return 1

        return 0 if sample_process(pid) else 1

    elif command == 'trace' and len(sys.argv) > 2:
        try:
            pid = int(sys.argv[2])
        except ValueError:
            print('Erro: PID deve ser um número inteiro.', file=sys.stderr)
            return 1

        if not psutil.pid_exists(pid):
            print(f'Erro: processo {pid} não encontrado.', file=sys.stderr)
            return 1

        return 0 if trace_process(pid) else 1

    elif command == 'report' and len(sys.argv) > 2:
        stats_file = sys.argv[2]
        if not os.path.exists(stats_file):
            print(f'Erro: arquivo não encontrado: {stats_file}', file=sys.stderr)
            return 1

        return 0 if generate_report(stats_file) else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
