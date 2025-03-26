#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de depuração do emulador.
"""

import os
import sys
import json
import shutil
import logging
import logging.handlers
import subprocess
import psutil
import gdb
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

# Configurações de depuração
DEBUG_CONFIG = {
    'directories': {
        'debug': 'debug',
        'dumps': 'debug/dumps',
        'traces': 'debug/traces',
        'symbols': 'debug/symbols',
        'logs': 'debug/logs'
    },
    'breakpoints': {
        'cpu': {
            'enabled': True,
            'points': [
                'cpu_execute',
                'cpu_interrupt',
                'cpu_reset'
            ]
        },
        'memory': {
            'enabled': True,
            'points': [
                'memory_read',
                'memory_write',
                'memory_map'
            ]
        },
        'video': {
            'enabled': True,
            'points': [
                'vdp_render',
                'vdp_update',
                'vdp_status'
            ]
        },
        'audio': {
            'enabled': True,
            'points': [
                'psg_write',
                'fm_write',
                'audio_update'
            ]
        }
    },
    'watchpoints': {
        'memory': {
            'enabled': True,
            'ranges': [
                {'start': 0x0000, 'end': 0x3FFF, 'type': 'r'},  # ROM
                {'start': 0xE000, 'end': 0xFFFF, 'type': 'rw'}  # RAM
            ]
        },
        'io': {
            'enabled': True,
            'ranges': [
                {'start': 0xC00000, 'end': 0xC0001F, 'type': 'rw'},  # VDP
                {'start': 0xC00011, 'end': 0xC00011, 'type': 'w'}    # PSG
            ]
        }
    },
    'tracing': {
        'enabled': True,
        'interval': 1000,  # instruções
        'format': 'text',
        'fields': [
            'timestamp',
            'pc',
            'opcode',
            'registers',
            'flags'
        ]
    },
    'dumps': {
        'enabled': True,
        'interval': 60,  # segundos
        'types': [
            'memory',
            'registers',
            'vram',
            'cram'
        ]
    },
    'symbols': {
        'enabled': True,
        'format': 'dwarf',
        'files': [
            'bin/mega-emu',
            'lib/libmega.so'
        ]
    },
    'logging': {
        'enabled': True,
        'level': 'DEBUG',
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
        for directory in DEBUG_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def attach_debugger(pid: int) -> Optional[gdb.Inferior]:
    """
    Anexa o depurador a um processo.

    Args:
        pid: ID do processo.

    Returns:
        Inferior do GDB ou None em caso de erro.
    """
    try:
        print(f'\nAnexando ao processo {pid}...')

        # Inicializa GDB
        gdb.execute('set pagination off')
        gdb.execute('set confirm off')

        # Anexa ao processo
        inferior = gdb.attach(pid)
        if not inferior:
            print('Falha ao anexar ao processo.')
            return None

        print('Depurador anexado com sucesso.')
        return inferior

    except Exception as e:
        print(f'Erro ao anexar depurador: {e}', file=sys.stderr)
        return None

def set_breakpoints(inferior: gdb.Inferior) -> bool:
    """
    Configura breakpoints.

    Args:
        inferior: Inferior do GDB.

    Returns:
        True se os breakpoints foram configurados com sucesso, False caso contrário.
    """
    try:
        print('\nConfigurando breakpoints...')

        for category, config in DEBUG_CONFIG['breakpoints'].items():
            if not config['enabled']:
                continue

            print(f'\nCategoria: {category}')
            for point in config['points']:
                try:
                    bp = gdb.Breakpoint(point)
                    print(f'Breakpoint configurado: {point}')
                except Exception as e:
                    print(f'Erro ao configurar breakpoint {point}: {e}')

        return True

    except Exception as e:
        print(f'Erro ao configurar breakpoints: {e}', file=sys.stderr)
        return False

def set_watchpoints(inferior: gdb.Inferior) -> bool:
    """
    Configura watchpoints.

    Args:
        inferior: Inferior do GDB.

    Returns:
        True se os watchpoints foram configurados com sucesso, False caso contrário.
    """
    try:
        print('\nConfigurando watchpoints...')

        for category, config in DEBUG_CONFIG['watchpoints'].items():
            if not config['enabled']:
                continue

            print(f'\nCategoria: {category}')
            for watch in config['ranges']:
                try:
                    wp = gdb.Watchpoint(
                        f'*(char[{watch["end"] - watch["start"] + 1}]*){watch["start"]}',
                        wp_class=watch['type']
                    )
                    print(f'Watchpoint configurado: {watch["start"]:04X}-{watch["end"]:04X} ({watch["type"]})')
                except Exception as e:
                    print(f'Erro ao configurar watchpoint {watch}: {e}')

        return True

    except Exception as e:
        print(f'Erro ao configurar watchpoints: {e}', file=sys.stderr)
        return False

def start_tracing(inferior: gdb.Inferior) -> bool:
    """
    Inicia rastreamento de execução.

    Args:
        inferior: Inferior do GDB.

    Returns:
        True se o rastreamento foi iniciado com sucesso, False caso contrário.
    """
    try:
        if not DEBUG_CONFIG['tracing']['enabled']:
            return True

        print('\nIniciando rastreamento...')

        # Configura arquivo de trace
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        trace_path = os.path.join(
            DEBUG_CONFIG['directories']['traces'],
            f'trace_{timestamp}.{DEBUG_CONFIG["tracing"]["format"]}'
        )

        # Configura comando de trace
        fields = ' '.join(DEBUG_CONFIG['tracing']['fields'])
        interval = DEBUG_CONFIG['tracing']['interval']
        gdb.execute(f'set trace-commands on')
        gdb.execute(f'set logging file {trace_path}')
        gdb.execute(f'set logging on')
        gdb.execute(f'set trace-notes {fields}')
        gdb.execute(f'trace-hook-stop')
        gdb.execute(f'commands\nsilent\nprintf "{fields}\\n"\ncontinue\nend')

        print(f'Rastreamento iniciado: {trace_path}')
        return True

    except Exception as e:
        print(f'Erro ao iniciar rastreamento: {e}', file=sys.stderr)
        return False

def dump_state(inferior: gdb.Inferior) -> bool:
    """
    Salva estado atual do processo.

    Args:
        inferior: Inferior do GDB.

    Returns:
        True se o estado foi salvo com sucesso, False caso contrário.
    """
    try:
        if not DEBUG_CONFIG['dumps']['enabled']:
            return True

        print('\nSalvando estado...')

        # Cria diretório do dump
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        dump_dir = os.path.join(
            DEBUG_CONFIG['directories']['dumps'],
            f'dump_{timestamp}'
        )
        os.makedirs(dump_dir)

        # Salva estado
        for dump_type in DEBUG_CONFIG['dumps']['types']:
            try:
                dump_path = os.path.join(dump_dir, f'{dump_type}.bin')

                if dump_type == 'memory':
                    gdb.execute(f'dump binary memory {dump_path} 0 0xFFFFFF')
                elif dump_type == 'registers':
                    with open(dump_path, 'w') as f:
                        f.write(gdb.execute('info registers', to_string=True))
                elif dump_type == 'vram':
                    gdb.execute(f'dump binary memory {dump_path} 0xC00000 0xC0FFFF')
                elif dump_type == 'cram':
                    gdb.execute(f'dump binary memory {dump_path} 0xC00000 0xC000FF')

                print(f'Estado salvo: {dump_path}')

            except Exception as e:
                print(f'Erro ao salvar {dump_type}: {e}')

        return True

    except Exception as e:
        print(f'Erro ao salvar estado: {e}', file=sys.stderr)
        return False

def load_symbols() -> bool:
    """
    Carrega símbolos de depuração.

    Returns:
        True se os símbolos foram carregados com sucesso, False caso contrário.
    """
    try:
        if not DEBUG_CONFIG['symbols']['enabled']:
            return True

        print('\nCarregando símbolos...')

        for file in DEBUG_CONFIG['symbols']['files']:
            try:
                gdb.execute(f'symbol-file {file}')
                print(f'Símbolos carregados: {file}')
            except Exception as e:
                print(f'Erro ao carregar símbolos de {file}: {e}')

        return True

    except Exception as e:
        print(f'Erro ao carregar símbolos: {e}', file=sys.stderr)
        return False

def setup_logging() -> bool:
    """
    Configura sistema de logging.

    Returns:
        True se a configuração foi bem sucedida, False caso contrário.
    """
    try:
        if not DEBUG_CONFIG['logging']['enabled']:
            return True

        # Configura logger
        logger = logging.getLogger('debug')
        logger.setLevel(DEBUG_CONFIG['logging']['level'])

        # Handler para arquivo
        log_path = os.path.join(
            DEBUG_CONFIG['directories']['logs'],
            'debug.log'
        )
        handler = logging.handlers.TimedRotatingFileHandler(
            log_path,
            when=DEBUG_CONFIG['logging']['rotation']['when'],
            interval=DEBUG_CONFIG['logging']['rotation']['interval'],
            backupCount=DEBUG_CONFIG['logging']['rotation']['backupCount']
        )
        handler.setFormatter(logging.Formatter(
            DEBUG_CONFIG['logging']['format']
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
        print('Uso: manage_debug.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  attach <pid>          Anexa ao processo', file=sys.stderr)
        print('  symbols               Carrega símbolos', file=sys.stderr)
        print('  dump                  Salva estado', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        if not create_directories():
            return 1
        if not setup_logging():
            return 1
        return 0

    elif command == 'attach' and len(sys.argv) > 2:
        try:
            pid = int(sys.argv[2])
        except ValueError:
            print('Erro: PID deve ser um número inteiro.', file=sys.stderr)
            return 1

        if not psutil.pid_exists(pid):
            print(f'Erro: processo {pid} não encontrado.', file=sys.stderr)
            return 1

        # Anexa ao processo
        inferior = attach_debugger(pid)
        if not inferior:
            return 1

        # Carrega símbolos
        if not load_symbols():
            return 1

        # Configura breakpoints
        if not set_breakpoints(inferior):
            return 1

        # Configura watchpoints
        if not set_watchpoints(inferior):
            return 1

        # Inicia rastreamento
        if not start_tracing(inferior):
            return 1

        return 0

    elif command == 'symbols':
        return 0 if load_symbols() else 1

    elif command == 'dump':
        if len(sys.argv) > 2:
            try:
                pid = int(sys.argv[2])
            except ValueError:
                print('Erro: PID deve ser um número inteiro.', file=sys.stderr)
                return 1

            if not psutil.pid_exists(pid):
                print(f'Erro: processo {pid} não encontrado.', file=sys.stderr)
                return 1

            inferior = attach_debugger(pid)
            if not inferior:
                return 1
        else:
            inferior = gdb.selected_inferior()
            if not inferior or not inferior.is_valid():
                print('Erro: nenhum processo anexado.', file=sys.stderr)
                return 1

        return 0 if dump_state(inferior) else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
