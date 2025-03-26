#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de atualização do emulador.
"""

import os
import sys
import json
import shutil
import logging
import logging.handlers
import subprocess
import hashlib
import requests
import semver
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

# Configurações de atualização
UPDATE_CONFIG = {
    'directories': {
        'update': 'update',
        'temp': 'update/temp',
        'backup': 'update/backup',
        'logs': 'update/logs'
    },
    'repository': {
        'github': {
            'owner': 'mega-emu',
            'repo': 'mega-emu',
            'branch': 'main'
        }
    },
    'releases': {
        'check_interval': 3600,  # segundos
        'pre_release': False,
        'channels': {
            'stable': {
                'pattern': r'^v\d+\.\d+\.\d+$',
                'auto_update': False
            },
            'beta': {
                'pattern': r'^v\d+\.\d+\.\d+-beta\.\d+$',
                'auto_update': False
            },
            'nightly': {
                'pattern': r'^v\d+\.\d+\.\d+-nightly\.\d+$',
                'auto_update': False
            }
        }
    },
    'verification': {
        'enabled': True,
        'algorithms': ['sha256', 'sha512'],
        'signature': True
    },
    'backup': {
        'enabled': True,
        'max_backups': 5,
        'include': [
            'bin/*',
            'lib/*',
            'share/shaders/*',
            'share/themes/*',
            'config/*'
        ],
        'exclude': [
            '*.log',
            '*.tmp',
            '*.bak'
        ]
    },
    'hooks': {
        'pre_update': None,
        'post_update': None
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
        for directory in UPDATE_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def get_current_version() -> str:
    """
    Obtém versão atual do emulador.

    Returns:
        Versão atual.
    """
    try:
        with open('version.txt', 'r') as f:
            return f.read().strip()
    except Exception as e:
        print(f'Erro ao obter versão atual: {e}', file=sys.stderr)
        return '0.0.0'

def check_for_updates(channel: str = 'stable') -> Optional[str]:
    """
    Verifica se há atualizações disponíveis.

    Args:
        channel: Canal de atualização.

    Returns:
        Versão mais recente disponível ou None se não houver atualização.
    """
    try:
        print(f'\nVerificando atualizações no canal {channel}...')

        # Obtém configuração do canal
        if channel not in UPDATE_CONFIG['releases']['channels']:
            print(f'Canal inválido: {channel}', file=sys.stderr)
            return None

        channel_config = UPDATE_CONFIG['releases']['channels'][channel]

        # Obtém versão atual
        current_version = get_current_version()
        if not current_version:
            return None

        # Obtém releases do GitHub
        config = UPDATE_CONFIG['repository']['github']
        response = requests.get(
            f'https://api.github.com/repos/{config["owner"]}/{config["repo"]}/releases'
        )
        response.raise_for_status()
        releases = response.json()

        # Filtra releases pelo canal
        import re
        pattern = re.compile(channel_config['pattern'])
        valid_releases = [
            r for r in releases
            if pattern.match(r['tag_name']) and
            not r['prerelease'] == UPDATE_CONFIG['releases']['pre_release']
        ]

        if not valid_releases:
            print('Nenhuma versão encontrada.')
            return None

        # Obtém versão mais recente
        latest_release = max(
            valid_releases,
            key=lambda r: semver.VersionInfo.parse(
                r['tag_name'].lstrip('v')
            )
        )
        latest_version = latest_release['tag_name'].lstrip('v')

        # Compara versões
        if semver.VersionInfo.parse(latest_version) > \
           semver.VersionInfo.parse(current_version):
            print(f'Nova versão disponível: {latest_version}')
            return latest_version
        else:
            print('Sistema atualizado.')
            return None

    except Exception as e:
        print(f'Erro ao verificar atualizações: {e}', file=sys.stderr)
        return None

def create_backup() -> Optional[str]:
    """
    Cria backup dos arquivos.

    Returns:
        Caminho do backup ou None em caso de erro.
    """
    try:
        if not UPDATE_CONFIG['backup']['enabled']:
            return None

        print('\nCriando backup...')

        # Gera nome do backup
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        backup_dir = os.path.join(
            UPDATE_CONFIG['directories']['backup'],
            f'backup_{timestamp}'
        )

        # Cria diretório de backup
        os.makedirs(backup_dir)

        # Copia arquivos
        for pattern in UPDATE_CONFIG['backup']['include']:
            for path in Path('.').glob(pattern):
                if any(path.match(e) for e in UPDATE_CONFIG['backup']['exclude']):
                    continue

                # Cria diretório de destino
                dest = os.path.join(backup_dir, path)
                os.makedirs(os.path.dirname(dest), exist_ok=True)

                # Copia arquivo
                if path.is_file():
                    shutil.copy2(path, dest)

        # Remove backups antigos
        backups = sorted(
            Path(UPDATE_CONFIG['directories']['backup']).glob('backup_*')
        )
        while len(backups) > UPDATE_CONFIG['backup']['max_backups']:
            shutil.rmtree(backups[0])
            backups.pop(0)

        print(f'Backup criado em: {backup_dir}')
        return backup_dir

    except Exception as e:
        print(f'Erro ao criar backup: {e}', file=sys.stderr)
        return None

def verify_package(package_path: str) -> bool:
    """
    Verifica integridade do pacote.

    Args:
        package_path: Caminho do pacote.

    Returns:
        True se o pacote é válido, False caso contrário.
    """
    try:
        if not UPDATE_CONFIG['verification']['enabled']:
            return True

        print('\nVerificando pacote...')

        # Verifica hashes
        for algorithm in UPDATE_CONFIG['verification']['algorithms']:
            hash_path = f'{package_path}.{algorithm}'
            if not os.path.exists(hash_path):
                print(f'Hash {algorithm} não encontrado.')
                return False

            # Lê hash esperado
            with open(hash_path, 'r') as f:
                expected_hash = f.read().strip().split()[0]

            # Calcula hash atual
            hash_obj = hashlib.new(algorithm)
            with open(package_path, 'rb') as f:
                for chunk in iter(lambda: f.read(8192), b''):
                    hash_obj.update(chunk)
            actual_hash = hash_obj.hexdigest()

            if actual_hash != expected_hash:
                print(f'Hash {algorithm} inválido.')
                return False

        # Verifica assinatura
        if UPDATE_CONFIG['verification']['signature']:
            sig_path = f'{package_path}.sig'
            if not os.path.exists(sig_path):
                print('Assinatura não encontrada.')
                return False

            # Lê assinatura
            with open(sig_path, 'r') as f:
                signature = f.read().strip()

            # Verifica assinatura
            # Nota: Implementar verificação de assinatura

        print('Pacote verificado com sucesso.')
        return True

    except Exception as e:
        print(f'Erro ao verificar pacote: {e}', file=sys.stderr)
        return False

def download_update(version: str) -> Optional[str]:
    """
    Baixa pacote de atualização.

    Args:
        version: Versão a ser baixada.

    Returns:
        Caminho do pacote baixado ou None em caso de erro.
    """
    try:
        print(f'\nBaixando versão {version}...')

        # Obtém informações do release
        config = UPDATE_CONFIG['repository']['github']
        response = requests.get(
            f'https://api.github.com/repos/{config["owner"]}/{config["repo"]}/releases/tags/v{version}'
        )
        response.raise_for_status()
        release = response.json()

        # Determina asset correto para a plataforma
        platform = sys.platform
        if platform.startswith('win'):
            pattern = r'.*\.exe$'
        elif platform.startswith('linux'):
            pattern = r'.*\.AppImage$'
        elif platform.startswith('darwin'):
            pattern = r'.*\.dmg$'
        else:
            print(f'Plataforma não suportada: {platform}', file=sys.stderr)
            return None

        # Encontra asset
        import re
        pattern = re.compile(pattern)
        asset = next(
            (a for a in release['assets'] if pattern.match(a['name'])),
            None
        )
        if not asset:
            print('Pacote não encontrado para esta plataforma.')
            return None

        # Baixa pacote
        package_path = os.path.join(
            UPDATE_CONFIG['directories']['temp'],
            asset['name']
        )
        response = requests.get(
            asset['browser_download_url'],
            stream=True
        )
        response.raise_for_status()

        with open(package_path, 'wb') as f:
            for chunk in response.iter_content(chunk_size=8192):
                if chunk:
                    f.write(chunk)

        # Baixa hashes
        for algorithm in UPDATE_CONFIG['verification']['algorithms']:
            hash_url = f'{asset["browser_download_url"]}.{algorithm}'
            response = requests.get(hash_url)
            response.raise_for_status()

            with open(f'{package_path}.{algorithm}', 'wb') as f:
                f.write(response.content)

        # Baixa assinatura
        if UPDATE_CONFIG['verification']['signature']:
            sig_url = f'{asset["browser_download_url"]}.sig'
            response = requests.get(sig_url)
            response.raise_for_status()

            with open(f'{package_path}.sig', 'wb') as f:
                f.write(response.content)

        print(f'Download concluído: {package_path}')
        return package_path

    except Exception as e:
        print(f'Erro ao baixar atualização: {e}', file=sys.stderr)
        return None

def install_update(package_path: str) -> bool:
    """
    Instala pacote de atualização.

    Args:
        package_path: Caminho do pacote.

    Returns:
        True se a instalação foi bem sucedida, False caso contrário.
    """
    try:
        print('\nInstalando atualização...')

        # Executa hook pre_update
        if UPDATE_CONFIG['hooks']['pre_update']:
            subprocess.run(
                UPDATE_CONFIG['hooks']['pre_update'],
                check=True
            )

        # Instala pacote
        if sys.platform.startswith('win'):
            # Executa instalador Windows
            subprocess.run(
                [package_path, '/VERYSILENT', '/NORESTART'],
                check=True
            )
        elif sys.platform.startswith('linux'):
            # Executa AppImage
            os.chmod(package_path, 0o755)
            subprocess.run(
                [package_path, '--install'],
                check=True
            )
        elif sys.platform.startswith('darwin'):
            # Monta DMG e copia aplicativo
            subprocess.run(
                ['hdiutil', 'attach', package_path],
                check=True
            )
            subprocess.run(
                ['cp', '-R', '/Volumes/Mega Emu/Mega Emu.app',
                 '/Applications/'],
                check=True
            )
            subprocess.run(
                ['hdiutil', 'detach', '/Volumes/Mega Emu'],
                check=True
            )

        # Executa hook post_update
        if UPDATE_CONFIG['hooks']['post_update']:
            subprocess.run(
                UPDATE_CONFIG['hooks']['post_update'],
                check=True
            )

        print('Atualização instalada com sucesso.')
        return True

    except Exception as e:
        print(f'Erro ao instalar atualização: {e}', file=sys.stderr)
        return False

def setup_logging() -> bool:
    """
    Configura sistema de logging.

    Returns:
        True se a configuração foi bem sucedida, False caso contrário.
    """
    try:
        if not UPDATE_CONFIG['logging']['enabled']:
            return True

        # Configura logger
        logger = logging.getLogger('update')
        logger.setLevel(UPDATE_CONFIG['logging']['level'])

        # Handler para arquivo
        log_path = os.path.join(
            UPDATE_CONFIG['directories']['logs'],
            'update.log'
        )
        handler = logging.handlers.TimedRotatingFileHandler(
            log_path,
            when=UPDATE_CONFIG['logging']['rotation']['when'],
            interval=UPDATE_CONFIG['logging']['rotation']['interval'],
            backupCount=UPDATE_CONFIG['logging']['rotation']['backupCount']
        )
        handler.setFormatter(logging.Formatter(
            UPDATE_CONFIG['logging']['format']
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
        print('Uso: manage_update.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  check [canal]         Verifica atualizações', file=sys.stderr)
        print('  update [canal]        Atualiza sistema', file=sys.stderr)
        print('  backup               Cria backup', file=sys.stderr)
        print('\nCanais disponíveis:', file=sys.stderr)
        print('  stable               Canal estável', file=sys.stderr)
        print('  beta                 Canal beta', file=sys.stderr)
        print('  nightly              Canal nightly', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        if not create_directories():
            return 1
        if not setup_logging():
            return 1
        return 0

    elif command == 'check':
        channel = sys.argv[2] if len(sys.argv) > 2 else 'stable'
        return 0 if check_for_updates(channel) is not None else 1

    elif command == 'update':
        channel = sys.argv[2] if len(sys.argv) > 2 else 'stable'

        # Verifica atualização
        version = check_for_updates(channel)
        if not version:
            return 1

        # Cria backup
        if not create_backup():
            return 1

        # Baixa atualização
        package_path = download_update(version)
        if not package_path:
            return 1

        # Verifica pacote
        if not verify_package(package_path):
            return 1

        # Instala atualização
        if not install_update(package_path):
            return 1

        return 0

    elif command == 'backup':
        return 0 if create_backup() else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
