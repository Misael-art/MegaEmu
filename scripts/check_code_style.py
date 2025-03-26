#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para verificar o estilo do código usando clang-format e clang-tidy.
"""

import os
import subprocess
import sys
from typing import List, Optional, Set, Tuple

def get_cpp_files(directory: str) -> Set[str]:
    """
    Retorna uma lista de arquivos C++ no diretório especificado.

    Args:
        directory: O diretório a ser pesquisado.

    Returns:
        Um conjunto de caminhos de arquivos C++.
    """
    cpp_files = set()
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(('.cpp', '.hpp', '.h')):
                cpp_files.add(os.path.join(root, file))
    return cpp_files

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

def check_clang_format(file: str) -> bool:
    """
    Verifica se o arquivo está formatado corretamente usando clang-format.

    Args:
        file: O arquivo a ser verificado.

    Returns:
        True se o arquivo está formatado corretamente, False caso contrário.
    """
    # Verifica se o arquivo está formatado corretamente
    returncode, stdout, stderr = run_command(
        ['clang-format', '--style=file', '--dry-run', '--Werror', file]
    )
    if returncode != 0:
        print(f'Erro de formatação em {file}:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    return True

def check_clang_tidy(file: str) -> bool:
    """
    Verifica se o arquivo passa nas verificações do clang-tidy.

    Args:
        file: O arquivo a ser verificado.

    Returns:
        True se o arquivo passa nas verificações, False caso contrário.
    """
    # Verifica se o arquivo passa nas verificações do clang-tidy
    returncode, stdout, stderr = run_command(
        ['clang-tidy', '--config-file=.clang-tidy', file, '--']
    )
    if returncode != 0:
        print(f'Erro de estilo em {file}:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    return True

def check_cppcheck(file: str) -> bool:
    """
    Verifica se o arquivo passa nas verificações do cppcheck.

    Args:
        file: O arquivo a ser verificado.

    Returns:
        True se o arquivo passa nas verificações, False caso contrário.
    """
    # Verifica se o arquivo passa nas verificações do cppcheck
    returncode, stdout, stderr = run_command(
        ['cppcheck', '--enable=all', '--std=c++17', '--error-exitcode=1', file]
    )
    if returncode != 0:
        print(f'Erro de análise estática em {file}:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    return True

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todos os arquivos passaram nas verificações, 1 caso contrário.
    """
    # Obtém a lista de arquivos C++
    cpp_files = get_cpp_files('src')
    if not cpp_files:
        print('Nenhum arquivo C++ encontrado!')
        return 0

    # Verifica cada arquivo
    success = True
    for file in sorted(cpp_files):
        print(f'Verificando {file}...')
        success &= check_clang_format(file)
        success &= check_clang_tidy(file)
        success &= check_cppcheck(file)

    if not success:
        print('\nAlguns arquivos não passaram nas verificações!', file=sys.stderr)
        return 1

    print('\nTodos os arquivos passaram nas verificações!')
    return 0

if __name__ == '__main__':
    sys.exit(main())
