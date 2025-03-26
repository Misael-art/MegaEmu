#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar a execução do emulador.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import platform
import signal
import psutil

# Configurações de execução
RUN_CONFIG = {
    'paths': {
        'roms': 'roms',
        'saves': 'saves',
        'cheats': 'cheats',
        'logs': 'logs',
        'config': 'config'
    },
    'video': {
        'resolution': {
            'width': 1280,
            'height': 720
        },
        'fullscreen': False,
        'vsync': True,
        'scale': 2,
        'filter': 'linear',
        'shader': 'default'
    },
    'audio': {
        'enabled': True,
        'volume': 1.0,
        'rate': 44100,
        'channels': 2,
        'buffer': 2048
    },
    'input': {
        'keyboard': {
            'up': 'Up',
            'down': 'Down',
            'left': 'Left',
            'right': 'Right',
            'a': 'X',
            'b': 'Z',
            'c': 'C',
            'start': 'Return',
            'mode': 'Tab'
        },
        'gamepad': {
            'up': 'dpup',
            'down': 'dpdown',
            'left': 'dpleft',
            'right': 'dpright',
            'a': 'x',
            'b': 'a',
            'c': 'b',
            'start': 'start',
            'mode': 'back'
        }
    },
    'emulation': {
        'region': 'auto',
        'framerate': 60,
        'fast_forward': 2,
        'rewind': {
            'enabled': True,
            'frames': 600,
            'buffer': 60
        }
    },
    'debug': {
        'log_level': 'info',
        'breakpoints': [],
        'watches': [],
        'trace': False
    }
}

def create_directories() -> bool:
    """
    Cria a estrutura de diretórios necessária.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        # Cria diretórios
        for path in RUN_CONFIG['paths'].values():
            os.makedirs(path, exist_ok=True)

        # Cria subdiretórios de saves
        save_types = ['state', 'sram', 'screenshot']
        for save_type in save_types:
            os.makedirs(os.path.join(RUN_CONFIG['paths']['saves'], save_type),
                       exist_ok=True)

        # Cria subdiretórios de logs
        log_types = ['debug', 'performance', 'error']
        for log_type in log_types:
            os.makedirs(os.path.join(RUN_CONFIG['paths']['logs'], log_type),
                       exist_ok=True)

        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def load_config() -> bool:
    """
    Carrega configurações do arquivo.

    Returns:
        True se as configurações foram carregadas com sucesso, False caso contrário.
    """
    try:
        config_file = os.path.join(RUN_CONFIG['paths']['config'], 'config.json')
        if os.path.exists(config_file):
            with open(config_file, 'r') as f:
                config = json.load(f)
                RUN_CONFIG.update(config)
        return True
    except Exception as e:
        print(f'Erro ao carregar configurações: {e}', file=sys.stderr)
        return False

def save_config() -> bool:
    """
    Salva configurações no arquivo.

    Returns:
        True se as configurações foram salvas com sucesso, False caso contrário.
    """
    try:
        config_file = os.path.join(RUN_CONFIG['paths']['config'], 'config.json')
        os.makedirs(os.path.dirname(config_file), exist_ok=True)
        with open(config_file, 'w') as f:
            json.dump(RUN_CONFIG, f, indent=4)
        return True
    except Exception as e:
        print(f'Erro ao salvar configurações: {e}', file=sys.stderr)
        return False

def get_platform() -> str:
    """
    Detecta a plataforma atual.

    Returns:
        Nome da plataforma ('windows', 'linux' ou 'macos').
    """
    if sys.platform.startswith('win'):
        return 'windows'
    elif sys.platform.startswith('linux'):
        return 'linux'
    elif sys.platform.startswith('darwin'):
        return 'macos'
    else:
        raise RuntimeError(f'Plataforma não suportada: {sys.platform}')

def get_binary_path() -> str:
    """
    Obtém o caminho do binário do emulador.

    Returns:
        Caminho do binário.
    """
    platform = get_platform()
    binary = 'mega_emu.exe' if platform == 'windows' else 'mega_emu'
    return os.path.join('build', 'release', 'bin', binary)

def run_emulator(rom_path: str, mode: str = 'normal',
                options: Optional[Dict] = None) -> bool:
    """
    Executa o emulador.

    Args:
        rom_path: Caminho da ROM.
        mode: Modo de execução ('normal', 'headless', 'debug' ou 'benchmark').
        options: Opções adicionais.

    Returns:
        True se a execução foi bem sucedida, False caso contrário.
    """
    try:
        # Verifica se ROM existe
        if not os.path.exists(rom_path):
            print(f'ROM não encontrada: {rom_path}', file=sys.stderr)
            return False

        # Monta comando
        binary = get_binary_path()
        if not os.path.exists(binary):
            print(f'Binário não encontrado: {binary}', file=sys.stderr)
            return False

        cmd = [binary, rom_path]

        # Adiciona opções do modo
        if mode == 'headless':
            cmd.append('--headless')
        elif mode == 'debug':
            cmd.extend(['--debug', '--log-level', RUN_CONFIG['debug']['log_level']])
            if RUN_CONFIG['debug']['trace']:
                cmd.append('--trace')
        elif mode == 'benchmark':
            cmd.extend(['--benchmark', '--no-audio', '--no-video'])

        # Adiciona opções de vídeo
        if mode != 'headless' and mode != 'benchmark':
            cmd.extend([
                '--resolution',
                f"{RUN_CONFIG['video']['resolution']['width']}x"
                f"{RUN_CONFIG['video']['resolution']['height']}",
                '--scale', str(RUN_CONFIG['video']['scale']),
                '--filter', RUN_CONFIG['video']['filter']
            ])
            if RUN_CONFIG['video']['fullscreen']:
                cmd.append('--fullscreen')
            if RUN_CONFIG['video']['vsync']:
                cmd.append('--vsync')

        # Adiciona opções de áudio
        if RUN_CONFIG['audio']['enabled'] and mode != 'benchmark':
            cmd.extend([
                '--audio-rate', str(RUN_CONFIG['audio']['rate']),
                '--audio-channels', str(RUN_CONFIG['audio']['channels']),
                '--audio-buffer', str(RUN_CONFIG['audio']['buffer']),
                '--volume', str(RUN_CONFIG['audio']['volume'])
            ])

        # Adiciona opções de emulação
        cmd.extend([
            '--region', RUN_CONFIG['emulation']['region'],
            '--framerate', str(RUN_CONFIG['emulation']['framerate'])
        ])
        if RUN_CONFIG['emulation']['rewind']['enabled']:
            cmd.extend([
                '--rewind',
                '--rewind-frames', str(RUN_CONFIG['emulation']['rewind']['frames']),
                '--rewind-buffer', str(RUN_CONFIG['emulation']['rewind']['buffer'])
            ])

        # Adiciona opções adicionais
        if options:
            for key, value in options.items():
                if isinstance(value, bool):
                    if value:
                        cmd.append(f'--{key}')
                else:
                    cmd.extend([f'--{key}', str(value)])

        # Executa emulador
        print(f'Executando: {" ".join(cmd)}')
        process = subprocess.Popen(cmd)

        # Aguarda sinal de interrupção
        try:
            process.wait()
        except KeyboardInterrupt:
            print('\nInterrompendo emulador...')
            if get_platform() == 'windows':
                process.terminate()
            else:
                process.send_signal(signal.SIGTERM)
            process.wait()

        return process.returncode == 0
    except Exception as e:
        print(f'Erro ao executar emulador: {e}', file=sys.stderr)
        return False

def list_roms() -> List[str]:
    """
    Lista ROMs disponíveis.

    Returns:
        Lista de caminhos das ROMs.
    """
    roms = []
    extensions = ['.md', '.bin', '.gen', '.smd']
    for root, _, files in os.walk(RUN_CONFIG['paths']['roms']):
        for file in files:
            if any(file.lower().endswith(ext) for ext in extensions):
                roms.append(os.path.join(root, file))
    return sorted(roms)

def print_info(rom_path: str) -> bool:
    """
    Exibe informações sobre uma ROM.

    Args:
        rom_path: Caminho da ROM.

    Returns:
        True se as informações foram exibidas com sucesso, False caso contrário.
    """
    try:
        # Executa emulador no modo info
        binary = get_binary_path()
        if not os.path.exists(binary):
            print(f'Binário não encontrado: {binary}', file=sys.stderr)
            return False

        result = subprocess.run(
            [binary, rom_path, '--info'],
            capture_output=True,
            text=True
        )
        if result.returncode != 0:
            print('Erro ao obter informações:', file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            return False

        print(result.stdout)
        return True
    except Exception as e:
        print(f'Erro ao exibir informações: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_run.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  config               Salva configurações', file=sys.stderr)
        print('  list                 Lista ROMs disponíveis', file=sys.stderr)
        print('  info <rom>           Exibe informações sobre ROM', file=sys.stderr)
        print('  run <rom> [modo]     Executa ROM', file=sys.stderr)
        return 1

    command = sys.argv[1]

    # Carrega configurações
    if not load_config():
        print('\nErro ao carregar configurações!', file=sys.stderr)
        return 1

    if command == 'init':
        return 0 if create_directories() else 1

    elif command == 'config':
        return 0 if save_config() else 1

    elif command == 'list':
        roms = list_roms()
        if not roms:
            print('Nenhuma ROM encontrada.')
            return 0

        print('ROMs disponíveis:')
        for rom in roms:
            print(f'  {rom}')
        return 0

    elif command == 'info' and len(sys.argv) > 2:
        return 0 if print_info(sys.argv[2]) else 1

    elif command == 'run' and len(sys.argv) > 2:
        rom_path = sys.argv[2]
        mode = sys.argv[3] if len(sys.argv) > 3 else 'normal'

        if mode not in ['normal', 'headless', 'debug', 'benchmark']:
            print(f'Modo inválido: {mode}', file=sys.stderr)
            print('Modos válidos: normal, headless, debug, benchmark', file=sys.stderr)
            return 1

        return 0 if run_emulator(rom_path, mode) else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
