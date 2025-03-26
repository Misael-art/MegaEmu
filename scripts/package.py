#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerar pacotes de distribuição.
"""

import os
import platform
import shutil
import subprocess
import sys
from datetime import datetime
from typing import Dict, List, Optional, Tuple

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

def get_version() -> str:
    """
    Obtém a versão atual do projeto.

    Returns:
        A versão do projeto.
    """
    # Lê a versão do CMakeLists.txt
    with open('CMakeLists.txt', 'r', encoding='utf-8') as f:
        for line in f:
            if 'project(MegaEmu VERSION' in line:
                version = line.split('VERSION')[1].strip().rstrip(')')
                return version.strip()
    return '0.0.0'

def build_project() -> bool:
    """
    Compila o projeto em modo Release.

    Returns:
        True se a compilação foi bem sucedida, False caso contrário.
    """
    print('\nCompilando projeto...')

    # Cria diretório de build se não existir
    if not os.path.exists('build'):
        os.makedirs('build')

    # Configura o CMake
    returncode, _, stderr = run_command([
        'cmake',
        '-B', 'build',
        '-S', '.',
        '-DCMAKE_BUILD_TYPE=Release',
        '-DBUILD_TESTING=OFF'
    ])
    if returncode != 0:
        print('Erro ao configurar CMake:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    # Compila o projeto
    returncode, _, stderr = run_command([
        'cmake',
        '--build', 'build',
        '--config', 'Release',
        '-j'
    ])
    if returncode != 0:
        print('Erro ao compilar projeto:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    return True

def create_package_directory(version: str) -> Tuple[bool, str]:
    """
    Cria o diretório para o pacote.

    Args:
        version: A versão do projeto.

    Returns:
        Uma tupla contendo um booleano indicando sucesso e o caminho do diretório.
    """
    print('\nCriando diretório do pacote...')

    # Nome base do pacote
    system = platform.system().lower()
    machine = platform.machine().lower()
    package_name = f'mega_emu-{version}-{system}-{machine}'

    # Cria diretório de pacotes se não existir
    packages_dir = 'packages'
    os.makedirs(packages_dir, exist_ok=True)

    # Cria diretório para este pacote
    package_dir = os.path.join(packages_dir, package_name)
    if os.path.exists(package_dir):
        shutil.rmtree(package_dir)
    os.makedirs(package_dir)

    return True, package_dir

def copy_files(package_dir: str) -> bool:
    """
    Copia os arquivos necessários para o pacote.

    Args:
        package_dir: O diretório do pacote.

    Returns:
        True se os arquivos foram copiados com sucesso, False caso contrário.
    """
    print('\nCopiando arquivos...')

    try:
        # Cria estrutura de diretórios
        os.makedirs(os.path.join(package_dir, 'bin'))
        os.makedirs(os.path.join(package_dir, 'lib'))
        os.makedirs(os.path.join(package_dir, 'share/doc'))
        os.makedirs(os.path.join(package_dir, 'share/shaders'))
        os.makedirs(os.path.join(package_dir, 'share/themes'))

        # Copia o executável
        if platform.system() == 'Windows':
            exe_name = 'mega_emu.exe'
        else:
            exe_name = 'mega_emu'
        shutil.copy2(
            os.path.join('build', 'src', exe_name),
            os.path.join(package_dir, 'bin', exe_name)
        )

        # Copia as bibliotecas
        for file in os.listdir('build/src'):
            if file.endswith(('.dll', '.so', '.dylib')):
                shutil.copy2(
                    os.path.join('build/src', file),
                    os.path.join(package_dir, 'lib', file)
                )

        # Copia a documentação
        doc_files = ['README.md', 'LICENSE', 'CHANGELOG.md']
        for file in doc_files:
            if os.path.exists(file):
                shutil.copy2(file, os.path.join(package_dir, 'share/doc', file))

        # Copia os shaders
        if os.path.exists('shaders'):
            for file in os.listdir('shaders'):
                if file.endswith(('.glsl', '.hlsl')):
                    shutil.copy2(
                        os.path.join('shaders', file),
                        os.path.join(package_dir, 'share/shaders', file)
                    )

        # Copia os temas
        if os.path.exists('themes'):
            for file in os.listdir('themes'):
                if file.endswith('.json'):
                    shutil.copy2(
                        os.path.join('themes', file),
                        os.path.join(package_dir, 'share/themes', file)
                    )

        return True
    except Exception as e:
        print(f'Erro ao copiar arquivos: {e}', file=sys.stderr)
        return False

def create_archive(package_dir: str) -> Tuple[bool, str]:
    """
    Cria um arquivo compactado do pacote.

    Args:
        package_dir: O diretório do pacote.

    Returns:
        Uma tupla contendo um booleano indicando sucesso e o caminho do arquivo.
    """
    print('\nCriando arquivo compactado...')

    try:
        # Nome do arquivo
        if platform.system() == 'Windows':
            archive_name = f'{package_dir}.zip'
            archive_format = 'zip'
        else:
            archive_name = f'{package_dir}.tar.gz'
            archive_format = 'gztar'

        # Cria o arquivo
        shutil.make_archive(
            package_dir,  # Nome base (sem extensão)
            archive_format,  # Formato
            'packages',  # Diretório raiz
            os.path.basename(package_dir)  # Diretório a ser compactado
        )

        return True, archive_name
    except Exception as e:
        print(f'Erro ao criar arquivo: {e}', file=sys.stderr)
        return False, ''

def create_checksum(archive_path: str) -> bool:
    """
    Cria um arquivo de checksum SHA256 para o pacote.

    Args:
        archive_path: O caminho do arquivo compactado.

    Returns:
        True se o checksum foi criado com sucesso, False caso contrário.
    """
    print('\nGerando checksum...')

    try:
        # Executa o comando sha256sum
        if platform.system() == 'Windows':
            returncode, stdout, stderr = run_command([
                'certutil', '-hashfile', archive_path, 'SHA256'
            ])
            if returncode == 0:
                checksum = stdout.split('\n')[1].strip()
        else:
            returncode, stdout, stderr = run_command([
                'sha256sum', archive_path
            ])
            if returncode == 0:
                checksum = stdout.split()[0]

        if returncode != 0:
            print('Erro ao gerar checksum:', file=sys.stderr)
            print(stderr, file=sys.stderr)
            return False

        # Salva o checksum
        with open(f'{archive_path}.sha256', 'w', encoding='utf-8') as f:
            f.write(f'{checksum}  {os.path.basename(archive_path)}\n')

        return True
    except Exception as e:
        print(f'Erro ao criar checksum: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se o pacote foi criado com sucesso, 1 caso contrário.
    """
    print('Iniciando criação do pacote...\n')

    # Obtém a versão do projeto
    version = get_version()
    print(f'Versão do projeto: {version}')

    # Compila o projeto
    if not build_project():
        print('\nErro ao compilar projeto!', file=sys.stderr)
        return 1

    # Cria o diretório do pacote
    success, package_dir = create_package_directory(version)
    if not success:
        print('\nErro ao criar diretório do pacote!', file=sys.stderr)
        return 1

    # Copia os arquivos
    if not copy_files(package_dir):
        print('\nErro ao copiar arquivos!', file=sys.stderr)
        return 1

    # Cria o arquivo compactado
    success, archive_path = create_archive(package_dir)
    if not success:
        print('\nErro ao criar arquivo compactado!', file=sys.stderr)
        return 1

    # Cria o checksum
    if not create_checksum(archive_path):
        print('\nErro ao criar checksum!', file=sys.stderr)
        return 1

    print(f'\nPacote criado com sucesso: {archive_path}')
    print(f'Checksum: {archive_path}.sha256')
    return 0

if __name__ == '__main__':
    sys.exit(main())
