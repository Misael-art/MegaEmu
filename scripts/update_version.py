#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para atualizar a versão do projeto.
"""

import os
import re
import sys
from typing import Optional, Tuple

def get_current_version() -> Optional[str]:
    """
    Obtém a versão atual do projeto do arquivo CMakeLists.txt.

    Returns:
        A versão atual do projeto ou None se não encontrada.
    """
    try:
        with open('CMakeLists.txt', 'r', encoding='utf-8') as f:
            content = f.read()
            match = re.search(r'project\s*\(\s*\w+\s+VERSION\s+(\d+\.\d+\.\d+)\s*\)', content)
            if match:
                return match.group(1)
    except FileNotFoundError:
        return None
    return None

def parse_version(version: str) -> Tuple[int, int, int]:
    """
    Converte uma string de versão em uma tupla de números.

    Args:
        version: A string de versão no formato 'major.minor.patch'.

    Returns:
        Uma tupla contendo os números major, minor e patch.

    Raises:
        ValueError: Se a string de versão não estiver no formato correto.
    """
    try:
        major, minor, patch = map(int, version.split('.'))
        return major, minor, patch
    except ValueError:
        raise ValueError(f'Versão inválida: {version}')

def increment_version(version: str, part: str) -> str:
    """
    Incrementa a versão do projeto.

    Args:
        version: A versão atual no formato 'major.minor.patch'.
        part: A parte da versão a ser incrementada ('major', 'minor' ou 'patch').

    Returns:
        A nova versão no formato 'major.minor.patch'.

    Raises:
        ValueError: Se a parte especificada não for válida.
    """
    major, minor, patch = parse_version(version)

    if part == 'major':
        major += 1
        minor = 0
        patch = 0
    elif part == 'minor':
        minor += 1
        patch = 0
    elif part == 'patch':
        patch += 1
    else:
        raise ValueError(f'Parte inválida: {part}')

    return f'{major}.{minor}.{patch}'

def update_version_in_file(file: str, old_version: str, new_version: str) -> bool:
    """
    Atualiza a versão em um arquivo.

    Args:
        file: O arquivo a ser atualizado.
        old_version: A versão antiga.
        new_version: A nova versão.

    Returns:
        True se o arquivo foi atualizado com sucesso, False caso contrário.
    """
    try:
        with open(file, 'r', encoding='utf-8') as f:
            content = f.read()

        # Atualiza a versão
        content = content.replace(old_version, new_version)

        with open(file, 'w', encoding='utf-8') as f:
            f.write(content)

        return True
    except Exception as e:
        print(f'Erro ao atualizar {file}: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se a versão foi atualizada com sucesso, 1 caso contrário.
    """
    # Verifica os argumentos
    if len(sys.argv) != 2 or sys.argv[1] not in ('major', 'minor', 'patch'):
        print('Uso: update_version.py <major|minor|patch>', file=sys.stderr)
        return 1

    # Obtém a versão atual
    current_version = get_current_version()
    if not current_version:
        print('Não foi possível encontrar a versão atual!', file=sys.stderr)
        return 1

    try:
        # Incrementa a versão
        new_version = increment_version(current_version, sys.argv[1])

        # Lista de arquivos a serem atualizados
        files_to_update = [
            'CMakeLists.txt',
            'vcpkg.json',
            'src/version.hpp',
            'docs/conf.py'
        ]

        # Atualiza cada arquivo
        success = True
        for file in files_to_update:
            if os.path.exists(file):
                print(f'Atualizando {file}...')
                if not update_version_in_file(file, current_version, new_version):
                    success = False

        if not success:
            print('\nAlguns arquivos não puderam ser atualizados!', file=sys.stderr)
            return 1

        print(f'\nVersão atualizada com sucesso: {current_version} -> {new_version}')
        return 0

    except ValueError as e:
        print(f'Erro: {e}', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
