#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de build do emulador.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import platform
import shutil
import logging
import logging.handlers
import multiprocessing
import tempfile
import re
import requests

# Configurações de build
BUILD_CONFIG = {
    'directories': {
        'build': 'build',
        'cache': 'build/cache',
        'deps': 'build/deps',
        'logs': 'build/logs'
    },
    'toolchains': {
        'windows': {
            'msvc': {
                'version': '14.0',
                'arch': ['x86_64'],
                'flags': [
                    '/W4',
                    '/WX',
                    '/O2',
                    '/GL',
                    '/MT'
                ]
            },
            'clang': {
                'version': '16.0.0',
                'arch': ['x86_64'],
                'flags': [
                    '-Wall',
                    '-Werror',
                    '-O3',
                    '-flto'
                ]
            }
        },
        'linux': {
            'gcc': {
                'version': '12.0.0',
                'arch': ['x86_64', 'aarch64'],
                'flags': [
                    '-Wall',
                    '-Werror',
                    '-O3',
                    '-flto'
                ]
            },
            'clang': {
                'version': '16.0.0',
                'arch': ['x86_64', 'aarch64'],
                'flags': [
                    '-Wall',
                    '-Werror',
                    '-O3',
                    '-flto'
                ]
            }
        },
        'macos': {
            'clang': {
                'version': '16.0.0',
                'arch': ['x86_64', 'arm64'],
                'flags': [
                    '-Wall',
                    '-Werror',
                    '-O3',
                    '-flto'
                ]
            }
        }
    },
    'dependencies': {
        'sdl2': {
            'version': '2.30.0',
            'url': 'https://github.com/libsdl-org/SDL/releases/download/release-{version}/SDL2-{version}.tar.gz',
            'cmake': True
        },
        'openal': {
            'version': '1.23.1',
            'url': 'https://github.com/kcat/openal-soft/archive/refs/tags/{version}.tar.gz',
            'cmake': True
        },
        'glew': {
            'version': '2.2.0',
            'url': 'https://github.com/nigels-com/glew/releases/download/glew-{version}/glew-{version}.tgz',
            'cmake': True
        }
    },
    'targets': {
        'debug': {
            'cmake': {
                'BUILD_TYPE': 'Debug',
                'ENABLE_TESTING': 'ON',
                'ENABLE_ASAN': 'ON',
                'ENABLE_UBSAN': 'ON'
            }
        },
        'release': {
            'cmake': {
                'BUILD_TYPE': 'Release',
                'ENABLE_TESTING': 'OFF',
                'ENABLE_LTO': 'ON',
                'ENABLE_PCH': 'ON'
            }
        },
        'profile': {
            'cmake': {
                'BUILD_TYPE': 'RelWithDebInfo',
                'ENABLE_TESTING': 'OFF',
                'ENABLE_PROFILING': 'ON',
                'ENABLE_PCH': 'ON'
            }
        }
    },
    'logging': {
        'enabled': True,
        'level': 'INFO',
        'format': '%(asctime)s [%(levelname)s] %(message)s',
        'rotation': {
            'when': 'D',
            'interval': 1,
            'backupCount': 7
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
        for directory in BUILD_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def check_toolchain() -> bool:
    """
    Verifica toolchain do sistema.

    Returns:
        True se o toolchain está disponível, False caso contrário.
    """
    try:
        system = platform.system().lower()
        if system not in BUILD_CONFIG['toolchains']:
            print(f'Sistema não suportado: {system}')
            return False

        print('\nVerificando toolchain...')

        if system == 'windows':
            # Verifica MSVC
            if 'msvc' in BUILD_CONFIG['toolchains'][system]:
                try:
                    import win32api
                    win32api.RegOpenKeyEx(
                        win32api.HKEY_LOCAL_MACHINE,
                        'SOFTWARE\\Microsoft\\VisualStudio\\14.0',
                        0,
                        win32api.KEY_READ
                    )
                except:
                    print('MSVC não encontrado.')
                    return False

            # Verifica Clang
            if 'clang' in BUILD_CONFIG['toolchains'][system]:
                result = subprocess.run(['clang', '--version'],
                                    capture_output=True,
                                    text=True)
                if result.returncode != 0:
                    print('Clang não encontrado.')
                    return False

                version = re.search(r'version (\d+\.\d+\.\d+)',
                                result.stdout)
                if not version:
                    print('Versão do Clang não encontrada.')
                    return False

                if version.group(1) < BUILD_CONFIG['toolchains'][system]['clang']['version']:
                    print(f'Versão do Clang insuficiente: {version.group(1)}')
                    return False

        elif system == 'linux':
            # Verifica GCC
            if 'gcc' in BUILD_CONFIG['toolchains'][system]:
                result = subprocess.run(['gcc', '--version'],
                                    capture_output=True,
                                    text=True)
                if result.returncode != 0:
                    print('GCC não encontrado.')
                    return False

                version = re.search(r'(\d+\.\d+\.\d+)',
                                result.stdout)
                if not version:
                    print('Versão do GCC não encontrada.')
                    return False

                if version.group(1) < BUILD_CONFIG['toolchains'][system]['gcc']['version']:
                    print(f'Versão do GCC insuficiente: {version.group(1)}')
                    return False

            # Verifica Clang
            if 'clang' in BUILD_CONFIG['toolchains'][system]:
                result = subprocess.run(['clang', '--version'],
                                    capture_output=True,
                                    text=True)
                if result.returncode != 0:
                    print('Clang não encontrado.')
                    return False

                version = re.search(r'version (\d+\.\d+\.\d+)',
                                result.stdout)
                if not version:
                    print('Versão do Clang não encontrada.')
                    return False

                if version.group(1) < BUILD_CONFIG['toolchains'][system]['clang']['version']:
                    print(f'Versão do Clang insuficiente: {version.group(1)}')
                    return False

        elif system == 'macos':
            # Verifica Clang
            if 'clang' in BUILD_CONFIG['toolchains'][system]:
                result = subprocess.run(['clang', '--version'],
                                    capture_output=True,
                                    text=True)
                if result.returncode != 0:
                    print('Clang não encontrado.')
                    return False

                version = re.search(r'version (\d+\.\d+\.\d+)',
                                result.stdout)
                if not version:
                    print('Versão do Clang não encontrada.')
                    return False

                if version.group(1) < BUILD_CONFIG['toolchains'][system]['clang']['version']:
                    print(f'Versão do Clang insuficiente: {version.group(1)}')
                    return False

        print('Toolchain verificado com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao verificar toolchain: {e}', file=sys.stderr)
        return False

def build_dependency(name: str) -> bool:
    """
    Compila uma dependência.

    Args:
        name: Nome da dependência.

    Returns:
        True se a dependência foi compilada com sucesso, False caso contrário.
    """
    try:
        if name not in BUILD_CONFIG['dependencies']:
            print(f'Dependência não encontrada: {name}')
            return False

        dep = BUILD_CONFIG['dependencies'][name]
        print(f'\nCompilando {name} {dep["version"]}...')

        # Cria diretório temporário
        temp_dir = tempfile.mkdtemp(dir=BUILD_CONFIG['directories']['temp'])

        try:
            # Baixa fonte
            url = dep['url'].format(version=dep['version'])
            archive = os.path.join(temp_dir,
                               os.path.basename(url))

            response = requests.get(url, stream=True)
            response.raise_for_status()

            with open(archive, 'wb') as f:
                for chunk in response.iter_content(chunk_size=8192):
                    f.write(chunk)

            # Extrai fonte
            if archive.endswith('.tar.gz') or archive.endswith('.tgz'):
                import tarfile
                with tarfile.open(archive, 'r:gz') as tar:
                    tar.extractall(temp_dir)
            elif archive.endswith('.zip'):
                import zipfile
                with zipfile.ZipFile(archive, 'r') as zip:
                    zip.extractall(temp_dir)

            # Encontra diretório fonte
            src_dir = next(
                d for d in os.listdir(temp_dir)
                if os.path.isdir(os.path.join(temp_dir, d))
            )
            src_dir = os.path.join(temp_dir, src_dir)

            # Cria diretório de build
            build_dir = os.path.join(temp_dir, 'build')
            os.makedirs(build_dir)

            if dep['cmake']:
                # Configura CMake
                subprocess.run([
                    'cmake',
                    '-S', src_dir,
                    '-B', build_dir,
                    '-DCMAKE_BUILD_TYPE=Release',
                    f'-DCMAKE_INSTALL_PREFIX={BUILD_CONFIG["directories"]["deps"]}'
                ], check=True)

                # Compila
                subprocess.run([
                    'cmake',
                    '--build', build_dir,
                    '--config', 'Release',
                    '-j', str(multiprocessing.cpu_count())
                ], check=True)

                # Instala
                subprocess.run([
                    'cmake',
                    '--install', build_dir
                ], check=True)

            else:
                # Compila com make
                subprocess.run([
                    'make',
                    '-C', src_dir,
                    f'PREFIX={BUILD_CONFIG["directories"]["deps"]}',
                    '-j', str(multiprocessing.cpu_count())
                ], check=True)

                # Instala
                subprocess.run([
                    'make',
                    '-C', src_dir,
                    f'PREFIX={BUILD_CONFIG["directories"]["deps"]}',
                    'install'
                ], check=True)

            print(f'{name} compilado com sucesso.')
            return True
        finally:
            shutil.rmtree(temp_dir)
    except Exception as e:
        print(f'Erro ao compilar {name}: {e}', file=sys.stderr)
        return False

def build_target(target: str) -> bool:
    """
    Compila um alvo.

    Args:
        target: Nome do alvo.

    Returns:
        True se o alvo foi compilado com sucesso, False caso contrário.
    """
    try:
        if target not in BUILD_CONFIG['targets']:
            print(f'Alvo não encontrado: {target}')
            return False

        print(f'\nCompilando alvo {target}...')

        # Cria diretório de build
        build_dir = os.path.join(BUILD_CONFIG['directories']['build'],
                              target)
        os.makedirs(build_dir, exist_ok=True)

        # Configura CMake
        cmake_args = [
            'cmake',
            '-S', '.',
            '-B', build_dir,
            f'-DCMAKE_PREFIX_PATH={BUILD_CONFIG["directories"]["deps"]}'
        ]

        for key, value in BUILD_CONFIG['targets'][target]['cmake'].items():
            cmake_args.append(f'-D{key}={value}')

        subprocess.run(cmake_args, check=True)

        # Compila
        subprocess.run([
            'cmake',
            '--build', build_dir,
            '--config', BUILD_CONFIG['targets'][target]['cmake']['BUILD_TYPE'],
            '-j', str(multiprocessing.cpu_count())
        ], check=True)

        print(f'Alvo {target} compilado com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao compilar alvo {target}: {e}', file=sys.stderr)
        return False

def clean_target(target: str) -> bool:
    """
    Limpa um alvo.

    Args:
        target: Nome do alvo.

    Returns:
        True se o alvo foi limpo com sucesso, False caso contrário.
    """
    try:
        if target not in BUILD_CONFIG['targets']:
            print(f'Alvo não encontrado: {target}')
            return False

        print(f'\nLimpando alvo {target}...')

        # Remove diretório de build
        build_dir = os.path.join(BUILD_CONFIG['directories']['build'],
                              target)
        if os.path.exists(build_dir):
            shutil.rmtree(build_dir)

        print(f'Alvo {target} limpo com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao limpar alvo {target}: {e}', file=sys.stderr)
        return False

def setup_logging() -> bool:
    """
    Configura sistema de logging.

    Returns:
        True se a configuração foi bem sucedida, False caso contrário.
    """
    try:
        if not BUILD_CONFIG['logging']['enabled']:
            return True

        # Configura logger
        logger = logging.getLogger('build')
        logger.setLevel(BUILD_CONFIG['logging']['level'])

        # Handler para arquivo
        log_path = os.path.join(BUILD_CONFIG['directories']['logs'],
                             'build.log')
        handler = logging.handlers.TimedRotatingFileHandler(
            log_path,
            when=BUILD_CONFIG['logging']['rotation']['when'],
            interval=BUILD_CONFIG['logging']['rotation']['interval'],
            backupCount=BUILD_CONFIG['logging']['rotation']['backupCount']
        )
        handler.setFormatter(logging.Formatter(
            BUILD_CONFIG['logging']['format']
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
        print('Uso: manage_build.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  check                 Verifica toolchain', file=sys.stderr)
        print('  deps <nome>           Compila dependência', file=sys.stderr)
        print('  build <alvo>          Compila alvo', file=sys.stderr)
        print('  clean <alvo>          Limpa alvo', file=sys.stderr)
        print('\nAlvos disponíveis:', file=sys.stderr)
        for target in BUILD_CONFIG['targets']:
            print(f'  {target}', file=sys.stderr)
        print('\nDependências disponíveis:', file=sys.stderr)
        for dep in BUILD_CONFIG['dependencies']:
            print(f'  {dep}', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        if not create_directories():
            return 1
        if not setup_logging():
            return 1
        return 0

    elif command == 'check':
        return 0 if check_toolchain() else 1

    elif command == 'deps':
        if len(sys.argv) < 3:
            print('Erro: dependência não especificada.', file=sys.stderr)
            return 1

        dep = sys.argv[2]
        return 0 if build_dependency(dep) else 1

    elif command == 'build':
        if len(sys.argv) < 3:
            print('Erro: alvo não especificado.', file=sys.stderr)
            return 1

        target = sys.argv[2]
        return 0 if build_target(target) else 1

    elif command == 'clean':
        if len(sys.argv) < 3:
            print('Erro: alvo não especificado.', file=sys.stderr)
            return 1

        target = sys.argv[2]
        return 0 if clean_target(target) else 1

    else:
        print('Comando inválido.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
