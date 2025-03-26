#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de distribuição do emulador.
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
import tempfile
import requests
import hashlib
import base64
import hmac

# Configurações de distribuição
DIST_CONFIG = {
    'directories': {
        'dist': 'dist',
        'temp': 'dist/temp',
        'repo': 'dist/repo',
        'logs': 'dist/logs'
    },
    'repository': {
        'github': {
            'owner': 'mega-emu',
            'repo': 'mega-emu',
            'token': None,  # Definido via variável de ambiente GITHUB_TOKEN
            'release': {
                'draft': True,
                'prerelease': False,
                'generate_notes': True,
                'discussion_category': 'Releases'
            }
        },
        'sourceforge': {
            'project': 'mega-emu',
            'username': None,  # Definido via variável de ambiente SF_USERNAME
            'password': None,  # Definido via variável de ambiente SF_PASSWORD
            'release': {
                'folder': '/home/frs/project/mega-emu'
            }
        },
        'itch': {
            'user': 'mega-emu',
            'game': 'mega-emu',
            'token': None,  # Definido via variável de ambiente ITCH_TOKEN
            'channel': {
                'windows': 'windows',
                'linux': 'linux',
                'macos': 'macos'
            }
        }
    },
    'packages': {
        'windows': {
            'installer': {
                'pattern': '*.exe',
                'type': 'application/x-msdownload'
            },
            'portable': {
                'pattern': '*.zip',
                'type': 'application/zip'
            }
        },
        'linux': {
            'deb': {
                'pattern': '*.deb',
                'type': 'application/x-debian-package'
            },
            'rpm': {
                'pattern': '*.rpm',
                'type': 'application/x-rpm'
            },
            'appimage': {
                'pattern': '*.AppImage',
                'type': 'application/x-appimage'
            }
        },
        'macos': {
            'dmg': {
                'pattern': '*.dmg',
                'type': 'application/x-apple-diskimage'
            },
            'pkg': {
                'pattern': '*.pkg',
                'type': 'application/x-newton-compatible-pkg'
            }
        }
    },
    'verification': {
        'enabled': True,
        'algorithms': ['sha256', 'sha512'],
        'signature': True
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
        for directory in DIST_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def collect_packages() -> Dict[str, List[str]]:
    """
    Coleta pacotes para distribuição.

    Returns:
        Dicionário com pacotes por plataforma.
    """
    try:
        print('\nColetando pacotes...')
        packages = {}

        for platform_name, formats in DIST_CONFIG['packages'].items():
            packages[platform_name] = []
            for format_name, format_info in formats.items():
                for path in Path('package/dist').glob(format_info['pattern']):
                    if path.is_file():
                        packages[platform_name].append(str(path))

        return packages
    except Exception as e:
        print(f'Erro ao coletar pacotes: {e}', file=sys.stderr)
        return {}

def verify_packages(packages: Dict[str, List[str]]) -> bool:
    """
    Verifica integridade dos pacotes.

    Args:
        packages: Dicionário com pacotes por plataforma.

    Returns:
        True se todos os pacotes são válidos, False caso contrário.
    """
    try:
        if not DIST_CONFIG['verification']['enabled']:
            return True

        print('\nVerificando pacotes...')

        for platform_name, paths in packages.items():
            for path in paths:
                print(f'\nVerificando {os.path.basename(path)}...')

                # Verifica hashes
                for algorithm in DIST_CONFIG['verification']['algorithms']:
                    hash_path = f'{path}.{algorithm}'
                    if not os.path.exists(hash_path):
                        print(f'Hash {algorithm} não encontrado.')
                        return False

                    # Lê hash esperado
                    with open(hash_path, 'r') as f:
                        expected_hash = f.read().strip().split()[0]

                    # Calcula hash atual
                    hash_obj = hashlib.new(algorithm)
                    with open(path, 'rb') as f:
                        for chunk in iter(lambda: f.read(8192), b''):
                            hash_obj.update(chunk)
                    actual_hash = hash_obj.hexdigest()

                    if actual_hash != expected_hash:
                        print(f'Hash {algorithm} inválido.')
                        return False

                # Verifica assinatura
                if DIST_CONFIG['verification']['signature']:
                    sig_path = f'{path}.sig'
                    if not os.path.exists(sig_path):
                        print('Assinatura não encontrada.')
                        return False

                    # Lê assinatura
                    with open(sig_path, 'r') as f:
                        signature = base64.b64decode(f.read())

                    # Verifica assinatura
                    from cryptography.hazmat.primitives import serialization
                    from cryptography.hazmat.primitives.asymmetric import padding
                    from cryptography.hazmat.primitives import hashes

                    with open('security/keys/public.pem', 'rb') as f:
                        public_key = serialization.load_pem_public_key(f.read())

                    try:
                        public_key.verify(
                            signature,
                            expected_hash.encode(),
                            padding.PSS(
                                mgf=padding.MGF1(hashes.SHA256()),
                                salt_length=padding.PSS.MAX_LENGTH
                            ),
                            hashes.SHA256()
                        )
                    except:
                        print('Assinatura inválida.')
                        return False

        print('\nTodos os pacotes verificados com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao verificar pacotes: {e}', file=sys.stderr)
        return False

def publish_github(packages: Dict[str, List[str]]) -> bool:
    """
    Publica pacotes no GitHub.

    Args:
        packages: Dicionário com pacotes por plataforma.

    Returns:
        True se os pacotes foram publicados com sucesso, False caso contrário.
    """
    try:
        config = DIST_CONFIG['repository']['github']
        print('\nPublicando no GitHub...')

        # Obtém token
        token = os.environ.get('GITHUB_TOKEN')
        if not token:
            print('Token do GitHub não encontrado.')
            return False

        # Cria release
        headers = {
            'Authorization': f'token {token}',
            'Accept': 'application/vnd.github.v3+json'
        }

        # Lê versão
        with open('version.txt', 'r') as f:
            version = f.read().strip()

        # Cria release
        response = requests.post(
            f'https://api.github.com/repos/{config["owner"]}/{config["repo"]}/releases',
            headers=headers,
            json={
                'tag_name': f'v{version}',
                'name': f'Version {version}',
                'draft': config['release']['draft'],
                'prerelease': config['release']['prerelease'],
                'generate_release_notes': config['release']['generate_notes'],
                'discussion_category_name': config['release']['discussion_category']
            }
        )
        response.raise_for_status()
        release = response.json()

        # Faz upload dos pacotes
        for platform_name, paths in packages.items():
            for path in paths:
                print(f'\nEnviando {os.path.basename(path)}...')

                # Determina tipo MIME
                for format_info in DIST_CONFIG['packages'][platform_name].values():
                    if Path(path).match(format_info['pattern']):
                        mime_type = format_info['type']
                        break

                # Faz upload
                with open(path, 'rb') as f:
                    response = requests.post(
                        release['upload_url'].replace(
                            '{?name,label}',
                            f'?name={os.path.basename(path)}'
                        ),
                        headers={
                            'Authorization': f'token {token}',
                            'Content-Type': mime_type
                        },
                        data=f
                    )
                    response.raise_for_status()

                # Faz upload dos hashes
                for algorithm in DIST_CONFIG['verification']['algorithms']:
                    hash_path = f'{path}.{algorithm}'
                    with open(hash_path, 'rb') as f:
                        response = requests.post(
                            release['upload_url'].replace(
                                '{?name,label}',
                                f'?name={os.path.basename(hash_path)}'
                            ),
                            headers={
                                'Authorization': f'token {token}',
                                'Content-Type': 'text/plain'
                            },
                            data=f
                        )
                        response.raise_for_status()

                # Faz upload da assinatura
                if DIST_CONFIG['verification']['signature']:
                    sig_path = f'{path}.sig'
                    with open(sig_path, 'rb') as f:
                        response = requests.post(
                            release['upload_url'].replace(
                                '{?name,label}',
                                f'?name={os.path.basename(sig_path)}'
                            ),
                            headers={
                                'Authorization': f'token {token}',
                                'Content-Type': 'text/plain'
                            },
                            data=f
                        )
                        response.raise_for_status()

        print('\nPacotes publicados com sucesso no GitHub.')
        return True
    except Exception as e:
        print(f'Erro ao publicar no GitHub: {e}', file=sys.stderr)
        return False

def publish_sourceforge(packages: Dict[str, List[str]]) -> bool:
    """
    Publica pacotes no SourceForge.

    Args:
        packages: Dicionário com pacotes por plataforma.

    Returns:
        True se os pacotes foram publicados com sucesso, False caso contrário.
    """
    try:
        config = DIST_CONFIG['repository']['sourceforge']
        print('\nPublicando no SourceForge...')

        # Obtém credenciais
        username = os.environ.get('SF_USERNAME')
        password = os.environ.get('SF_PASSWORD')
        if not username or not password:
            print('Credenciais do SourceForge não encontradas.')
            return False

        # Lê versão
        with open('version.txt', 'r') as f:
            version = f.read().strip()

        # Cria diretório da versão
        version_dir = f'{config["release"]["folder"]}/{version}'
        subprocess.run([
            'ssh',
            f'{username}@frs.sourceforge.net',
            'mkdir',
            '-p',
            version_dir
        ], check=True)

        # Envia pacotes
        for platform_name, paths in packages.items():
            for path in paths:
                print(f'\nEnviando {os.path.basename(path)}...')

                # Envia arquivo
                subprocess.run([
                    'scp',
                    path,
                    f'{username}@frs.sourceforge.net:{version_dir}'
                ], check=True)

                # Envia hashes
                for algorithm in DIST_CONFIG['verification']['algorithms']:
                    hash_path = f'{path}.{algorithm}'
                    subprocess.run([
                        'scp',
                        hash_path,
                        f'{username}@frs.sourceforge.net:{version_dir}'
                    ], check=True)

                # Envia assinatura
                if DIST_CONFIG['verification']['signature']:
                    sig_path = f'{path}.sig'
                    subprocess.run([
                        'scp',
                        sig_path,
                        f'{username}@frs.sourceforge.net:{version_dir}'
                    ], check=True)

        print('\nPacotes publicados com sucesso no SourceForge.')
        return True
    except Exception as e:
        print(f'Erro ao publicar no SourceForge: {e}', file=sys.stderr)
        return False

def publish_itch(packages: Dict[str, List[str]]) -> bool:
    """
    Publica pacotes no itch.io.

    Args:
        packages: Dicionário com pacotes por plataforma.

    Returns:
        True se os pacotes foram publicados com sucesso, False caso contrário.
    """
    try:
        config = DIST_CONFIG['repository']['itch']
        print('\nPublicando no itch.io...')

        # Obtém token
        token = os.environ.get('ITCH_TOKEN')
        if not token:
            print('Token do itch.io não encontrado.')
            return False

        # Configura butler
        subprocess.run([
            'butler',
            'login',
            token
        ], check=True)

        # Envia pacotes
        for platform_name, paths in packages.items():
            for path in paths:
                print(f'\nEnviando {os.path.basename(path)}...')

                # Envia arquivo
                subprocess.run([
                    'butler',
                    'push',
                    path,
                    f'{config["user"]}/{config["game"]}:{config["channel"][platform_name]}'
                ], check=True)

        print('\nPacotes publicados com sucesso no itch.io.')
        return True
    except Exception as e:
        print(f'Erro ao publicar no itch.io: {e}', file=sys.stderr)
        return False

def setup_logging() -> bool:
    """
    Configura sistema de logging.

    Returns:
        True se a configuração foi bem sucedida, False caso contrário.
    """
    try:
        if not DIST_CONFIG['logging']['enabled']:
            return True

        # Configura logger
        logger = logging.getLogger('dist')
        logger.setLevel(DIST_CONFIG['logging']['level'])

        # Handler para arquivo
        log_path = os.path.join(DIST_CONFIG['directories']['logs'],
                             'dist.log')
        handler = logging.handlers.TimedRotatingFileHandler(
            log_path,
            when=DIST_CONFIG['logging']['rotation']['when'],
            interval=DIST_CONFIG['logging']['rotation']['interval'],
            backupCount=DIST_CONFIG['logging']['rotation']['backupCount']
        )
        handler.setFormatter(logging.Formatter(
            DIST_CONFIG['logging']['format']
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
        print('Uso: manage_dist.py <comando> [plataforma]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  publish <plataforma>  Publica pacotes', file=sys.stderr)
        print('\nPlataformas disponíveis:', file=sys.stderr)
        print('  github               GitHub Releases', file=sys.stderr)
        print('  sourceforge          SourceForge Files', file=sys.stderr)
        print('  itch                 itch.io', file=sys.stderr)
        print('  all                  Todas as plataformas', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        if not create_directories():
            return 1
        if not setup_logging():
            return 1
        return 0

    elif command == 'publish':
        if len(sys.argv) < 3:
            print('Erro: plataforma não especificada.', file=sys.stderr)
            return 1

        platform = sys.argv[2]
        if platform not in ['github', 'sourceforge', 'itch', 'all']:
            print(f'Erro: plataforma inválida: {platform}', file=sys.stderr)
            return 1

        # Coleta pacotes
        packages = collect_packages()
        if not packages:
            return 1

        # Verifica pacotes
        if not verify_packages(packages):
            return 1

        # Publica pacotes
        if platform == 'github' or platform == 'all':
            if not publish_github(packages):
                return 1

        if platform == 'sourceforge' or platform == 'all':
            if not publish_sourceforge(packages):
                return 1

        if platform == 'itch' or platform == 'all':
            if not publish_itch(packages):
                return 1

        return 0

    else:
        print('Comando inválido.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
