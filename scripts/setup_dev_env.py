#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para configurar o ambiente de desenvolvimento.
"""

import os
import platform
import subprocess
import sys
from typing import Dict, List, Optional, Tuple

# Dependências necessárias por sistema operacional
DEPENDENCIES: Dict[str, List[str]] = {
    'Windows': [
        'cmake',
        'ninja',
        'doxygen',
        'sphinx',
        'clang',
        'clang-tidy',
        'clang-format',
        'cppcheck',
        'python3',
        'git',
        'vcpkg'
    ],
    'Linux': [
        'build-essential',
        'cmake',
        'ninja-build',
        'doxygen',
        'python3-sphinx',
        'clang',
        'clang-tidy',
        'clang-format',
        'cppcheck',
        'python3',
        'git',
        'curl'
    ],
    'Darwin': [  # macOS
        'cmake',
        'ninja',
        'doxygen',
        'sphinx-doc',
        'llvm',
        'cppcheck',
        'python3',
        'git',
        'curl'
    ]
}

def run_command(command: List[str], cwd: Optional[str] = None) -> Tuple[int, str, str]:
    """
    Executa um comando e retorna o código de saída e as saídas padrão e de erro.

    Args:
        command: O comando a ser executado.
        cwd: O diretório de trabalho.

    Returns:
        Uma tupla contendo o código de saída, a saída padrão e a saída de erro.
    """
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=cwd,
        universal_newlines=True
    )
    stdout, stderr = process.communicate()
    return process.returncode, stdout, stderr

def get_package_manager() -> Optional[str]:
    """
    Detecta o gerenciador de pacotes do sistema.

    Returns:
        O nome do gerenciador de pacotes ou None se não encontrado.
    """
    system = platform.system()

    if system == 'Windows':
        # Verifica se o Chocolatey está instalado
        returncode, _, _ = run_command(['choco', '--version'])
        if returncode == 0:
            return 'choco'
        # Verifica se o Scoop está instalado
        if os.path.exists(os.path.expanduser('~/scoop')):
            return 'scoop'
    elif system == 'Linux':
        # Verifica diferentes gerenciadores de pacotes
        package_managers = {
            'apt-get': ['apt-get', '--version'],
            'dnf': ['dnf', '--version'],
            'pacman': ['pacman', '--version'],
            'zypper': ['zypper', '--version']
        }
        for pm, cmd in package_managers.items():
            returncode, _, _ = run_command(cmd)
            if returncode == 0:
                return pm
    elif system == 'Darwin':
        # Verifica se o Homebrew está instalado
        returncode, _, _ = run_command(['brew', '--version'])
        if returncode == 0:
            return 'brew'

    return None

def install_dependencies(package_manager: str) -> bool:
    """
    Instala as dependências necessárias usando o gerenciador de pacotes.

    Args:
        package_manager: O gerenciador de pacotes a ser usado.

    Returns:
        True se todas as dependências foram instaladas com sucesso, False caso contrário.
    """
    system = platform.system()
    if system not in DEPENDENCIES:
        print(f'Sistema operacional não suportado: {system}', file=sys.stderr)
        return False

    print(f'\nInstalando dependências usando {package_manager}...')

    # Comandos de instalação por gerenciador de pacotes
    install_commands = {
        'choco': ['choco', 'install', '-y'],
        'scoop': ['scoop', 'install'],
        'apt-get': ['sudo', 'apt-get', 'install', '-y'],
        'dnf': ['sudo', 'dnf', 'install', '-y'],
        'pacman': ['sudo', 'pacman', '-S', '--noconfirm'],
        'zypper': ['sudo', 'zypper', 'install', '-y'],
        'brew': ['brew', 'install']
    }

    if package_manager not in install_commands:
        print(f'Gerenciador de pacotes não suportado: {package_manager}', file=sys.stderr)
        return False

    success = True
    for dep in DEPENDENCIES[system]:
        print(f'\nInstalando {dep}...')
        cmd = install_commands[package_manager] + [dep]
        returncode, stdout, stderr = run_command(cmd)

        if returncode != 0:
            print(f'Erro ao instalar {dep}:', file=sys.stderr)
            print(stderr, file=sys.stderr)
            success = False
        else:
            print(f'{dep} instalado com sucesso!')

    return success

def setup_vcpkg() -> bool:
    """
    Configura o vcpkg.

    Returns:
        True se o vcpkg foi configurado com sucesso, False caso contrário.
    """
    print('\nConfigurando vcpkg...')

    # Clona o repositório do vcpkg se não existir
    if not os.path.exists('vcpkg'):
        print('\nClonando repositório do vcpkg...')
        returncode, _, stderr = run_command([
            'git', 'clone', 'https://github.com/Microsoft/vcpkg.git'
        ])
        if returncode != 0:
            print('Erro ao clonar vcpkg:', file=sys.stderr)
            print(stderr, file=sys.stderr)
            return False

    # Executa o script de bootstrap
    print('\nExecutando bootstrap do vcpkg...')
    if platform.system() == 'Windows':
        bootstrap_script = 'bootstrap-vcpkg.bat'
    else:
        bootstrap_script = './bootstrap-vcpkg.sh'

    returncode, _, stderr = run_command([bootstrap_script], cwd='vcpkg')
    if returncode != 0:
        print('Erro ao executar bootstrap do vcpkg:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    # Instala as dependências do projeto
    print('\nInstalando dependências do projeto com vcpkg...')
    dependencies = [
        'sdl2',
        'sdl2-image',
        'sdl2-ttf',
        'sdl2-mixer',
        'qt5',
        'boost',
        'gtest',
        'fmt',
        'spdlog',
        'nlohmann-json'
    ]

    for dep in dependencies:
        print(f'\nInstalando {dep}...')
        returncode, _, stderr = run_command(
            ['./vcpkg/vcpkg', 'install', f'{dep}:x64-windows'],
            cwd='.'
        )
        if returncode != 0:
            print(f'Erro ao instalar {dep}:', file=sys.stderr)
            print(stderr, file=sys.stderr)
            return False
        print(f'{dep} instalado com sucesso!')

    return True

def setup_python_deps() -> bool:
    """
    Instala as dependências Python necessárias.

    Returns:
        True se as dependências foram instaladas com sucesso, False caso contrário.
    """
    print('\nInstalando dependências Python...')

    requirements = [
        'sphinx',
        'sphinx-rtd-theme',
        'breathe',
        'pytest',
        'pytest-cov',
        'black',
        'isort',
        'pylint',
        'mypy',
        'pre-commit'
    ]

    returncode, _, stderr = run_command([
        sys.executable, '-m', 'pip', 'install', '-U', 'pip'
    ])
    if returncode != 0:
        print('Erro ao atualizar pip:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    for req in requirements:
        print(f'\nInstalando {req}...')
        returncode, _, stderr = run_command([
            sys.executable, '-m', 'pip', 'install', req
        ])
        if returncode != 0:
            print(f'Erro ao instalar {req}:', file=sys.stderr)
            print(stderr, file=sys.stderr)
            return False
        print(f'{req} instalado com sucesso!')

    return True

def setup_git_hooks() -> bool:
    """
    Configura os hooks do Git.

    Returns:
        True se os hooks foram configurados com sucesso, False caso contrário.
    """
    print('\nConfigurando hooks do Git...')

    returncode, _, stderr = run_command(['pre-commit', 'install'])
    if returncode != 0:
        print('Erro ao configurar pre-commit:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    print('Hooks do Git configurados com sucesso!')
    return True

def main() -> int:
    """
    Função principal.

    Returns:
        0 se a configuração foi bem sucedida, 1 caso contrário.
    """
    print('Configurando ambiente de desenvolvimento...\n')

    # Detecta o gerenciador de pacotes
    package_manager = get_package_manager()
    if not package_manager:
        print('Nenhum gerenciador de pacotes suportado encontrado!', file=sys.stderr)
        return 1

    # Instala as dependências do sistema
    if not install_dependencies(package_manager):
        print('\nErro ao instalar dependências do sistema!', file=sys.stderr)
        return 1

    # Configura o vcpkg
    if not setup_vcpkg():
        print('\nErro ao configurar vcpkg!', file=sys.stderr)
        return 1

    # Instala as dependências Python
    if not setup_python_deps():
        print('\nErro ao instalar dependências Python!', file=sys.stderr)
        return 1

    # Configura os hooks do Git
    if not setup_git_hooks():
        print('\nErro ao configurar hooks do Git!', file=sys.stderr)
        return 1

    print('\nAmbiente de desenvolvimento configurado com sucesso!')
    return 0

if __name__ == '__main__':
    sys.exit(main())
