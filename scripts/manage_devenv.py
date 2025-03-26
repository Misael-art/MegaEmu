#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar a configuração do ambiente de desenvolvimento.
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
import venv

# Configurações do ambiente
DEVENV_CONFIG = {
    'python': {
        'version': '3.11',
        'packages': [
            'pytest',
            'pytest-cov',
            'pytest-benchmark',
            'mypy',
            'pylint',
            'black',
            'isort',
            'sphinx',
            'sphinx-rtd-theme',
            'breathe',
            'matplotlib',
            'pandas',
            'seaborn',
            'jinja2'
        ]
    },
    'cpp': {
        'compiler': {
            'windows': {
                'name': 'MSVC',
                'version': '17.0',
                'command': 'cl.exe'
            },
            'linux': {
                'name': 'GCC',
                'version': '13.0',
                'command': 'g++'
            },
            'macos': {
                'name': 'Clang',
                'version': '17.0',
                'command': 'clang++'
            }
        },
        'tools': {
            'windows': [
                'cmake',
                'ninja',
                'doxygen',
                'graphviz',
                'clang-format',
                'clang-tidy',
                'llvm',
                'vcpkg'
            ],
            'linux': [
                'cmake',
                'ninja-build',
                'doxygen',
                'graphviz',
                'clang-format',
                'clang-tidy',
                'llvm',
                'ccache'
            ],
            'macos': [
                'cmake',
                'ninja',
                'doxygen',
                'graphviz',
                'clang-format',
                'clang-tidy',
                'llvm',
                'ccache'
            ]
        }
    },
    'dependencies': {
        'sdl2': {
            'version': '2.30.0',
            'url': 'https://github.com/libsdl-org/SDL/releases/download/release-{version}/SDL2-{version}.zip'
        },
        'openal': {
            'version': '1.23.1',
            'url': 'https://github.com/kcat/openal-soft/archive/refs/tags/{version}.zip'
        },
        'glew': {
            'version': '2.2.0',
            'url': 'https://github.com/nigels-com/glew/releases/download/glew-{version}/glew-{version}.zip'
        },
        'glm': {
            'version': '0.9.9.8',
            'url': 'https://github.com/g-truc/glm/releases/download/{version}/glm-{version}.zip'
        },
        'imgui': {
            'version': '1.90.4',
            'url': 'https://github.com/ocornut/imgui/archive/refs/tags/v{version}.zip'
        }
    },
    'vscode': {
        'extensions': [
            'ms-vscode.cpptools',
            'ms-vscode.cmake-tools',
            'twxs.cmake',
            'ms-python.python',
            'ms-python.vscode-pylance',
            'ms-python.black-formatter',
            'ms-python.isort',
            'donjayamanne.python-extension-pack',
            'streetsidesoftware.code-spell-checker',
            'streetsidesoftware.code-spell-checker-portuguese-brazilian',
            'eamodio.gitlens',
            'mhutchie.git-graph',
            'cschlosser.doxdocgen',
            'jeff-hykin.better-cpp-syntax',
            'xaver.clang-format'
        ],
        'settings': {
            'editor.formatOnSave': True,
            'editor.formatOnType': True,
            'editor.rulers': [80, 100],
            'editor.renderWhitespace': 'all',
            'editor.suggestSelection': 'first',
            'files.trimTrailingWhitespace': True,
            'files.insertFinalNewline': True,
            'files.trimFinalNewlines': True,
            'C_Cpp.clang_format_style': 'file',
            'C_Cpp.default.cppStandard': 'c++20',
            'C_Cpp.default.includePath': [
                '${workspaceFolder}/src',
                '${workspaceFolder}/include',
                '${workspaceFolder}/deps/include'
            ],
            'python.linting.enabled': True,
            'python.linting.pylintEnabled': True,
            'python.formatting.provider': 'black',
            'python.sortImports.args': ['--profile', 'black']
        }
    }
}

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

def create_directories() -> bool:
    """
    Cria a estrutura de diretórios do projeto.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        # Cria diretórios principais
        dirs = [
            'src',
            'include',
            'deps',
            'deps/include',
            'deps/lib',
            'deps/bin',
            'scripts',
            'tests',
            'docs',
            'build',
            'dist',
            'tools'
        ]

        # Cria diretórios
        for directory in dirs:
            os.makedirs(directory, exist_ok=True)

        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def setup_python_env() -> bool:
    """
    Configura ambiente Python.

    Returns:
        True se o ambiente foi configurado com sucesso, False caso contrário.
    """
    try:
        # Cria ambiente virtual
        venv_dir = 'venv'
        if not os.path.exists(venv_dir):
            print('Criando ambiente virtual...')
            venv.create(venv_dir, with_pip=True)

        # Define comando pip
        platform = get_platform()
        if platform == 'windows':
            pip = os.path.join(venv_dir, 'Scripts', 'pip.exe')
        else:
            pip = os.path.join(venv_dir, 'bin', 'pip')

        # Atualiza pip
        subprocess.run([pip, 'install', '--upgrade', 'pip'],
                      capture_output=True, text=True)

        # Instala pacotes
        print('Instalando pacotes Python...')
        for package in DEVENV_CONFIG['python']['packages']:
            print(f'  {package}')
            result = subprocess.run([pip, 'install', package],
                                 capture_output=True, text=True)
            if result.returncode != 0:
                print(f'Erro ao instalar {package}:', file=sys.stderr)
                print(result.stderr, file=sys.stderr)
                return False

        return True
    except Exception as e:
        print(f'Erro ao configurar ambiente Python: {e}', file=sys.stderr)
        return False

def setup_cpp_tools() -> bool:
    """
    Configura ferramentas C++.

    Returns:
        True se as ferramentas foram configuradas com sucesso, False caso contrário.
    """
    try:
        platform = get_platform()
        tools = DEVENV_CONFIG['cpp']['tools'][platform]

        # Instala ferramentas
        print('Instalando ferramentas C++...')
        if platform == 'windows':
            # No Windows, usa vcpkg
            if not os.path.exists('vcpkg'):
                print('Clonando vcpkg...')
                result = subprocess.run(
                    ['git', 'clone', 'https://github.com/Microsoft/vcpkg.git'],
                    capture_output=True, text=True
                )
                if result.returncode != 0:
                    print('Erro ao clonar vcpkg:', file=sys.stderr)
                    print(result.stderr, file=sys.stderr)
                    return False

                # Executa bootstrap
                print('Configurando vcpkg...')
                result = subprocess.run(
                    ['vcpkg/bootstrap-vcpkg.bat'],
                    capture_output=True, text=True
                )
                if result.returncode != 0:
                    print('Erro ao configurar vcpkg:', file=sys.stderr)
                    print(result.stderr, file=sys.stderr)
                    return False

            # Instala pacotes
            for tool in tools:
                print(f'  {tool}')
                result = subprocess.run(
                    ['vcpkg/vcpkg.exe', 'install', f'{tool}:x64-windows'],
                    capture_output=True, text=True
                )
                if result.returncode != 0:
                    print(f'Erro ao instalar {tool}:', file=sys.stderr)
                    print(result.stderr, file=sys.stderr)
                    return False

        elif platform == 'linux':
            # No Linux, usa apt
            print('Atualizando lista de pacotes...')
            subprocess.run(['sudo', 'apt', 'update'],
                         capture_output=True, text=True)

            for tool in tools:
                print(f'  {tool}')
                result = subprocess.run(
                    ['sudo', 'apt', 'install', '-y', tool],
                    capture_output=True, text=True
                )
                if result.returncode != 0:
                    print(f'Erro ao instalar {tool}:', file=sys.stderr)
                    print(result.stderr, file=sys.stderr)
                    return False

        else:  # macOS
            # No macOS, usa Homebrew
            print('Atualizando Homebrew...')
            subprocess.run(['brew', 'update'],
                         capture_output=True, text=True)

            for tool in tools:
                print(f'  {tool}')
                result = subprocess.run(
                    ['brew', 'install', tool],
                    capture_output=True, text=True
                )
                if result.returncode != 0:
                    print(f'Erro ao instalar {tool}:', file=sys.stderr)
                    print(result.stderr, file=sys.stderr)
                    return False

        return True
    except Exception as e:
        print(f'Erro ao configurar ferramentas C++: {e}', file=sys.stderr)
        return False

def download_dependencies() -> bool:
    """
    Baixa e configura dependências.

    Returns:
        True se as dependências foram configuradas com sucesso, False caso contrário.
    """
    try:
        # Para cada dependência
        for name, config in DEVENV_CONFIG['dependencies'].items():
            print(f'Configurando {name}...')

            # Define URLs
            url = config['url'].format(version=config['version'])
            filename = os.path.basename(url)
            download_dir = os.path.join('deps', 'download')
            extract_dir = os.path.join('deps', 'extract', name)

            # Cria diretórios
            os.makedirs(download_dir, exist_ok=True)
            os.makedirs(extract_dir, exist_ok=True)

            # Baixa arquivo
            print(f'  Baixando {filename}...')
            result = subprocess.run(
                ['curl', '-L', '-o', os.path.join(download_dir, filename), url],
                capture_output=True, text=True
            )
            if result.returncode != 0:
                print(f'Erro ao baixar {name}:', file=sys.stderr)
                print(result.stderr, file=sys.stderr)
                return False

            # Extrai arquivo
            print('  Extraindo...')
            shutil.unpack_archive(
                os.path.join(download_dir, filename),
                extract_dir
            )

            # Copia arquivos
            print('  Copiando arquivos...')
            for root, _, files in os.walk(extract_dir):
                for file in files:
                    src = os.path.join(root, file)
                    if file.endswith(('.h', '.hpp')):
                        dst = os.path.join('deps', 'include', file)
                    elif file.endswith(('.lib', '.a', '.so', '.dylib')):
                        dst = os.path.join('deps', 'lib', file)
                    elif file.endswith(('.dll', '.exe')):
                        dst = os.path.join('deps', 'bin', file)
                    else:
                        continue
                    shutil.copy2(src, dst)

        return True
    except Exception as e:
        print(f'Erro ao configurar dependências: {e}', file=sys.stderr)
        return False

def configure_vscode() -> bool:
    """
    Configura Visual Studio Code.

    Returns:
        True se o VS Code foi configurado com sucesso, False caso contrário.
    """
    try:
        # Cria diretório .vscode
        os.makedirs('.vscode', exist_ok=True)

        # Cria settings.json
        settings_file = os.path.join('.vscode', 'settings.json')
        with open(settings_file, 'w') as f:
            json.dump(DEVENV_CONFIG['vscode']['settings'], f, indent=4)

        # Instala extensões
        print('Instalando extensões do VS Code...')
        for extension in DEVENV_CONFIG['vscode']['extensions']:
            print(f'  {extension}')
            result = subprocess.run(
                ['code', '--install-extension', extension, '--force'],
                capture_output=True, text=True
            )
            if result.returncode != 0:
                print(f'Erro ao instalar extensão {extension}:', file=sys.stderr)
                print(result.stderr, file=sys.stderr)
                return False

        return True
    except Exception as e:
        print(f'Erro ao configurar VS Code: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_devenv.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  python               Configura ambiente Python', file=sys.stderr)
        print('  cpp                  Configura ferramentas C++', file=sys.stderr)
        print('  deps                 Baixa dependências', file=sys.stderr)
        print('  vscode              Configura VS Code', file=sys.stderr)
        print('  all                  Configura tudo', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_directories() else 1

    elif command == 'python':
        return 0 if setup_python_env() else 1

    elif command == 'cpp':
        return 0 if setup_cpp_tools() else 1

    elif command == 'deps':
        return 0 if download_dependencies() else 1

    elif command == 'vscode':
        return 0 if configure_vscode() else 1

    elif command == 'all':
        if not create_directories():
            print('\nErro ao criar diretórios!', file=sys.stderr)
            return 1

        if not setup_python_env():
            print('\nErro ao configurar ambiente Python!', file=sys.stderr)
            return 1

        if not setup_cpp_tools():
            print('\nErro ao configurar ferramentas C++!', file=sys.stderr)
            return 1

        if not download_dependencies():
            print('\nErro ao baixar dependências!', file=sys.stderr)
            return 1

        if not configure_vscode():
            print('\nErro ao configurar VS Code!', file=sys.stderr)
            return 1

        return 0

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
