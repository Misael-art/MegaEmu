#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar a integração com o GitHub.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import requests

# Configurações do GitHub
GITHUB_CONFIG = {
    'api': {
        'base_url': 'https://api.github.com',
        'headers': {
            'Accept': 'application/vnd.github.v3+json'
        }
    },
    'repo': {
        'owner': 'seu-usuario',
        'name': 'mega-emu',
        'description': 'Emulador de Mega Drive/Genesis em C++',
        'homepage': '',
        'topics': [
            'emulator',
            'mega-drive',
            'genesis',
            'cpp',
            'opengl',
            'sdl2'
        ],
        'license': 'MIT'
    },
    'release': {
        'draft': True,
        'prerelease': False,
        'generate_notes': True,
        'discussion_category': 'Announcements'
    },
    'workflow': {
        'name': 'CI/CD',
        'on': {
            'push': {
                'branches': ['main', 'develop'],
                'tags': ['v*']
            },
            'pull_request': {
                'branches': ['main', 'develop']
            }
        },
        'jobs': {
            'build': {
                'runs-on': ['windows-latest', 'ubuntu-latest', 'macos-latest'],
                'steps': [
                    'actions/checkout@v4',
                    'actions/setup-python@v4',
                    'run-cmake',
                    'run-tests',
                    'upload-artifacts'
                ]
            }
        }
    }
}

def get_github_token() -> str:
    """
    Obtém o token de acesso do GitHub.

    Returns:
        Token de acesso do GitHub.
    """
    token = os.getenv('GITHUB_TOKEN')
    if not token:
        raise RuntimeError('Token do GitHub não encontrado. '
                         'Configure a variável de ambiente GITHUB_TOKEN.')
    return token

def create_headers() -> Dict[str, str]:
    """
    Cria cabeçalhos para requisições à API do GitHub.

    Returns:
        Dicionário com cabeçalhos.
    """
    return {
        **GITHUB_CONFIG['api']['headers'],
        'Authorization': f'token {get_github_token()}'
    }

def create_repository() -> bool:
    """
    Cria um repositório no GitHub.

    Returns:
        True se o repositório foi criado com sucesso, False caso contrário.
    """
    try:
        # Monta URL
        url = f"{GITHUB_CONFIG['api']['base_url']}/user/repos"

        # Monta dados
        data = {
            'name': GITHUB_CONFIG['repo']['name'],
            'description': GITHUB_CONFIG['repo']['description'],
            'homepage': GITHUB_CONFIG['repo']['homepage'],
            'private': False,
            'has_issues': True,
            'has_projects': True,
            'has_wiki': True,
            'auto_init': True,
            'license_template': GITHUB_CONFIG['repo']['license'],
            'allow_squash_merge': True,
            'allow_merge_commit': True,
            'allow_rebase_merge': True,
            'delete_branch_on_merge': True
        }

        # Faz requisição
        response = requests.post(url, headers=create_headers(), json=data)
        response.raise_for_status()

        # Adiciona tópicos
        url = (f"{GITHUB_CONFIG['api']['base_url']}/repos/"
               f"{GITHUB_CONFIG['repo']['owner']}/"
               f"{GITHUB_CONFIG['repo']['name']}/topics")
        data = {'names': GITHUB_CONFIG['repo']['topics']}
        response = requests.put(url, headers=create_headers(), json=data)
        response.raise_for_status()

        return True
    except Exception as e:
        print(f'Erro ao criar repositório: {e}', file=sys.stderr)
        return False

def configure_repository() -> bool:
    """
    Configura um repositório existente no GitHub.

    Returns:
        True se o repositório foi configurado com sucesso, False caso contrário.
    """
    try:
        # Monta URL
        url = (f"{GITHUB_CONFIG['api']['base_url']}/repos/"
               f"{GITHUB_CONFIG['repo']['owner']}/"
               f"{GITHUB_CONFIG['repo']['name']}")

        # Monta dados
        data = {
            'description': GITHUB_CONFIG['repo']['description'],
            'homepage': GITHUB_CONFIG['repo']['homepage'],
            'has_issues': True,
            'has_projects': True,
            'has_wiki': True,
            'allow_squash_merge': True,
            'allow_merge_commit': True,
            'allow_rebase_merge': True,
            'delete_branch_on_merge': True
        }

        # Faz requisição
        response = requests.patch(url, headers=create_headers(), json=data)
        response.raise_for_status()

        # Atualiza tópicos
        url = f"{url}/topics"
        data = {'names': GITHUB_CONFIG['repo']['topics']}
        response = requests.put(url, headers=create_headers(), json=data)
        response.raise_for_status()

        return True
    except Exception as e:
        print(f'Erro ao configurar repositório: {e}', file=sys.stderr)
        return False

def create_workflow() -> bool:
    """
    Cria arquivo de workflow do GitHub Actions.

    Returns:
        True se o workflow foi criado com sucesso, False caso contrário.
    """
    try:
        # Cria diretório
        workflow_dir = '.github/workflows'
        os.makedirs(workflow_dir, exist_ok=True)

        # Define caminho do arquivo
        workflow_file = os.path.join(workflow_dir, 'ci.yml')

        # Cria arquivo
        with open(workflow_file, 'w') as f:
            # Nome do workflow
            f.write(f"name: {GITHUB_CONFIG['workflow']['name']}\n\n")

            # Eventos que disparam o workflow
            f.write("on:\n")
            for event, config in GITHUB_CONFIG['workflow']['on'].items():
                f.write(f"  {event}:\n")
                for key, value in config.items():
                    if isinstance(value, list):
                        f.write(f"    {key}:\n")
                        for item in value:
                            f.write(f"      - {item}\n")
                    else:
                        f.write(f"    {key}: {value}\n")
            f.write("\n")

            # Jobs
            f.write("jobs:\n")
            for job_name, job_config in GITHUB_CONFIG['workflow']['jobs'].items():
                f.write(f"  {job_name}:\n")
                f.write("    strategy:\n")
                f.write("      matrix:\n")
                f.write("        os: ")
                f.write(str(job_config['runs-on']).replace("'", ""))
                f.write("\n\n")
                f.write("    runs-on: ${{ matrix.os }}\n\n")
                f.write("    steps:\n")
                for step in job_config['steps']:
                    if '/' in step:  # Ação do GitHub
                        f.write(f"      - uses: {step}\n")
                    else:  # Comando
                        f.write(f"      - name: {step}\n")
                        if step == 'run-cmake':
                            f.write("        run: |\n")
                            f.write("          cmake -B build -DCMAKE_BUILD_TYPE=Release\n")
                            f.write("          cmake --build build --config Release\n")
                        elif step == 'run-tests':
                            f.write("        run: |\n")
                            f.write("          cd build\n")
                            f.write("          ctest -C Release --output-on-failure\n")
                        elif step == 'upload-artifacts':
                            f.write("        uses: actions/upload-artifact@v3\n")
                            f.write("        with:\n")
                            f.write("          name: binaries-${{ matrix.os }}\n")
                            f.write("          path: build/bin\n")

        return True
    except Exception as e:
        print(f'Erro ao criar workflow: {e}', file=sys.stderr)
        return False

def create_release(version: str, body: str = '') -> bool:
    """
    Cria uma release no GitHub.

    Args:
        version: Versão da release (ex: v1.0.0).
        body: Descrição da release.

    Returns:
        True se a release foi criada com sucesso, False caso contrário.
    """
    try:
        # Monta URL
        url = (f"{GITHUB_CONFIG['api']['base_url']}/repos/"
               f"{GITHUB_CONFIG['repo']['owner']}/"
               f"{GITHUB_CONFIG['repo']['name']}/releases")

        # Monta dados
        data = {
            'tag_name': version,
            'target_commitish': 'main',
            'name': f'Release {version}',
            'body': body,
            'draft': GITHUB_CONFIG['release']['draft'],
            'prerelease': GITHUB_CONFIG['release']['prerelease'],
            'generate_release_notes': GITHUB_CONFIG['release']['generate_notes'],
            'discussion_category_name': GITHUB_CONFIG['release']['discussion_category']
        }

        # Faz requisição
        response = requests.post(url, headers=create_headers(), json=data)
        response.raise_for_status()

        return True
    except Exception as e:
        print(f'Erro ao criar release: {e}', file=sys.stderr)
        return False

def upload_assets(release_id: str, assets: List[str]) -> bool:
    """
    Faz upload de assets para uma release.

    Args:
        release_id: ID da release.
        assets: Lista de caminhos dos arquivos.

    Returns:
        True se os assets foram enviados com sucesso, False caso contrário.
    """
    try:
        # Para cada asset
        for asset_path in assets:
            # Verifica se arquivo existe
            if not os.path.exists(asset_path):
                print(f'Arquivo não encontrado: {asset_path}', file=sys.stderr)
                continue

            # Monta URL
            url = (f"{GITHUB_CONFIG['api']['base_url']}/repos/"
                  f"{GITHUB_CONFIG['repo']['owner']}/"
                  f"{GITHUB_CONFIG['repo']['name']}/releases/{release_id}/assets")

            # Define nome do asset
            asset_name = os.path.basename(asset_path)

            # Define tipo MIME
            mime_type = 'application/octet-stream'
            if asset_path.endswith('.zip'):
                mime_type = 'application/zip'
            elif asset_path.endswith('.tar.gz'):
                mime_type = 'application/gzip'
            elif asset_path.endswith('.json'):
                mime_type = 'application/json'

            # Faz upload
            with open(asset_path, 'rb') as f:
                headers = {
                    **create_headers(),
                    'Content-Type': mime_type
                }
                params = {'name': asset_name}
                response = requests.post(url, headers=headers, params=params, data=f)
                response.raise_for_status()

        return True
    except Exception as e:
        print(f'Erro ao fazer upload de assets: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_github.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  create              Cria repositório', file=sys.stderr)
        print('  configure           Configura repositório', file=sys.stderr)
        print('  workflow            Cria arquivo de workflow', file=sys.stderr)
        print('  release <versão>    Cria release', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'create':
        return 0 if create_repository() else 1

    elif command == 'configure':
        return 0 if configure_repository() else 1

    elif command == 'workflow':
        return 0 if create_workflow() else 1

    elif command == 'release' and len(sys.argv) > 2:
        version = sys.argv[2]
        if not version.startswith('v'):
            version = f'v{version}'

        # Cria release
        if not create_release(version):
            print('\nErro ao criar release!', file=sys.stderr)
            return 1

        # Procura assets na pasta dist
        assets = []
        if os.path.exists('dist'):
            for file in os.listdir('dist'):
                if file.endswith(('.zip', '.tar.gz', '.json')):
                    assets.append(os.path.join('dist', file))

        # Faz upload de assets
        if assets and not upload_assets(version, assets):
            print('\nErro ao fazer upload de assets!', file=sys.stderr)
            return 1

        return 0

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
