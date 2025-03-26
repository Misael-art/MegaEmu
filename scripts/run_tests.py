#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para executar os testes do projeto.
"""

import os
import subprocess
import sys
from typing import List, Optional, Tuple

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

def build_project() -> bool:
    """
    Compila o projeto.

    Returns:
        True se a compilação foi bem sucedida, False caso contrário.
    """
    print('Compilando o projeto...')

    # Cria o diretório de build se não existir
    build_dir = 'build'
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    # Configura o CMake
    returncode, stdout, stderr = run_command(
        ['cmake', '-B', build_dir, '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=Debug'],
        cwd=os.getcwd()
    )
    if returncode != 0:
        print('Erro ao configurar o CMake:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    # Compila o projeto
    returncode, stdout, stderr = run_command(
        ['cmake', '--build', build_dir, '--config', 'Debug'],
        cwd=os.getcwd()
    )
    if returncode != 0:
        print('Erro ao compilar o projeto:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    return True

def run_unit_tests() -> bool:
    """
    Executa os testes unitários.

    Returns:
        True se todos os testes passaram, False caso contrário.
    """
    print('\nExecutando testes unitários...')

    # Executa os testes
    returncode, stdout, stderr = run_command(
        ['ctest', '--output-on-failure'],
        cwd='build'
    )
    if returncode != 0:
        print('Erro ao executar os testes:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    print(stdout)
    return True

def run_integration_tests() -> bool:
    """
    Executa os testes de integração.

    Returns:
        True se todos os testes passaram, False caso contrário.
    """
    print('\nExecutando testes de integração...')

    # Executa os testes
    returncode, stdout, stderr = run_command(
        ['python', '-m', 'pytest', 'tests/integration'],
        cwd=os.getcwd()
    )
    if returncode != 0:
        print('Erro ao executar os testes de integração:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    print(stdout)
    return True

def run_performance_tests() -> bool:
    """
    Executa os testes de performance.

    Returns:
        True se todos os testes passaram, False caso contrário.
    """
    print('\nExecutando testes de performance...')

    # Executa os testes
    returncode, stdout, stderr = run_command(
        ['python', '-m', 'pytest', 'tests/performance'],
        cwd=os.getcwd()
    )
    if returncode != 0:
        print('Erro ao executar os testes de performance:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    print(stdout)
    return True

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todos os testes passaram, 1 caso contrário.
    """
    # Compila o projeto
    if not build_project():
        return 1

    # Executa os testes
    success = True
    success &= run_unit_tests()
    success &= run_integration_tests()
    success &= run_performance_tests()

    if not success:
        print('\nAlguns testes falharam!', file=sys.stderr)
        return 1

    print('\nTodos os testes passaram!')
    return 0

if __name__ == '__main__':
    sys.exit(main())
