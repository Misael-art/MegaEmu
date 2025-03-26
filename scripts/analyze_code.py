#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerar relatórios de análise de código.
"""

import json
import os
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

def run_clang_tidy() -> Tuple[bool, str]:
    """
    Executa o clang-tidy e gera um relatório.

    Returns:
        Uma tupla contendo um booleano indicando sucesso e o caminho do relatório.
    """
    print('\nExecutando clang-tidy...')

    # Cria diretório para relatórios se não existir
    report_dir = 'reports/clang-tidy'
    os.makedirs(report_dir, exist_ok=True)

    # Nome do arquivo de relatório
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    report_file = f'{report_dir}/report_{timestamp}.txt'

    # Executa clang-tidy
    returncode, stdout, stderr = run_command([
        'clang-tidy',
        '-p', 'build',
        'src/**/*.cpp',
        '--',
        '-std=c++17'
    ])

    # Salva o relatório
    with open(report_file, 'w', encoding='utf-8') as f:
        f.write('=== Relatório do Clang-Tidy ===\n\n')
        f.write(f'Data: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}\n\n')
        if stdout:
            f.write('Saída:\n')
            f.write(stdout)
        if stderr:
            f.write('\nErros:\n')
            f.write(stderr)

    return returncode == 0, report_file

def run_cppcheck() -> Tuple[bool, str]:
    """
    Executa o cppcheck e gera um relatório.

    Returns:
        Uma tupla contendo um booleano indicando sucesso e o caminho do relatório.
    """
    print('\nExecutando cppcheck...')

    # Cria diretório para relatórios se não existir
    report_dir = 'reports/cppcheck'
    os.makedirs(report_dir, exist_ok=True)

    # Nome do arquivo de relatório
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    report_file = f'{report_dir}/report_{timestamp}.xml'

    # Executa cppcheck
    returncode, stdout, stderr = run_command([
        'cppcheck',
        '--enable=all',
        '--std=c++17',
        '--xml',
        '--xml-version=2',
        'src',
        f'2>{report_file}'
    ])

    return returncode == 0, report_file

def run_include_what_you_use() -> Tuple[bool, str]:
    """
    Executa o include-what-you-use e gera um relatório.

    Returns:
        Uma tupla contendo um booleano indicando sucesso e o caminho do relatório.
    """
    print('\nExecutando include-what-you-use...')

    # Cria diretório para relatórios se não existir
    report_dir = 'reports/iwyu'
    os.makedirs(report_dir, exist_ok=True)

    # Nome do arquivo de relatório
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    report_file = f'{report_dir}/report_{timestamp}.txt'

    # Executa include-what-you-use
    returncode, stdout, stderr = run_command([
        'iwyu_tool.py',
        '-p', 'build',
        'src'
    ])

    # Salva o relatório
    with open(report_file, 'w', encoding='utf-8') as f:
        f.write('=== Relatório do Include-What-You-Use ===\n\n')
        f.write(f'Data: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}\n\n')
        if stdout:
            f.write('Saída:\n')
            f.write(stdout)
        if stderr:
            f.write('\nErros:\n')
            f.write(stderr)

    return returncode == 0, report_file

def run_clang_format_check() -> Tuple[bool, str]:
    """
    Verifica a formatação do código com clang-format.

    Returns:
        Uma tupla contendo um booleano indicando sucesso e o caminho do relatório.
    """
    print('\nVerificando formatação com clang-format...')

    # Cria diretório para relatórios se não existir
    report_dir = 'reports/clang-format'
    os.makedirs(report_dir, exist_ok=True)

    # Nome do arquivo de relatório
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    report_file = f'{report_dir}/report_{timestamp}.txt'

    # Lista todos os arquivos .cpp e .hpp
    cpp_files = []
    for root, _, files in os.walk('src'):
        for file in files:
            if file.endswith(('.cpp', '.hpp')):
                cpp_files.append(os.path.join(root, file))

    # Verifica cada arquivo
    with open(report_file, 'w', encoding='utf-8') as f:
        f.write('=== Relatório do Clang-Format ===\n\n')
        f.write(f'Data: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}\n\n')

        success = True
        for file in cpp_files:
            returncode, stdout, stderr = run_command([
                'clang-format',
                '--style=file',
                '--dry-run',
                '--Werror',
                file
            ])

            if returncode != 0:
                success = False
                f.write(f'Arquivo {file} precisa ser formatado\n')
                if stderr:
                    f.write(f'Erro: {stderr}\n')

    return success, report_file

def run_complexity_analysis() -> Tuple[bool, str]:
    """
    Executa análise de complexidade do código.

    Returns:
        Uma tupla contendo um booleano indicando sucesso e o caminho do relatório.
    """
    print('\nExecutando análise de complexidade...')

    # Cria diretório para relatórios se não existir
    report_dir = 'reports/complexity'
    os.makedirs(report_dir, exist_ok=True)

    # Nome do arquivo de relatório
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    report_file = f'{report_dir}/report_{timestamp}.json'

    # Lista todos os arquivos .cpp
    cpp_files = []
    for root, _, files in os.walk('src'):
        for file in files:
            if file.endswith('.cpp'):
                cpp_files.append(os.path.join(root, file))

    # Analisa cada arquivo
    results = []
    for file in cpp_files:
        returncode, stdout, stderr = run_command([
            'lizard',
            '--CCN', '10',  # Limite de complexidade ciclomática
            '--length', '100',  # Limite de linhas por função
            '--arguments', '5',  # Limite de argumentos por função
            '--json',
            file
        ])

        if stdout:
            try:
                data = json.loads(stdout)
                results.extend(data)
            except json.JSONDecodeError:
                print(f'Erro ao analisar saída do lizard para {file}', file=sys.stderr)

    # Salva o relatório
    with open(report_file, 'w', encoding='utf-8') as f:
        json.dump(results, f, indent=2)

    return True, report_file

def generate_summary(reports: Dict[str, Tuple[bool, str]]) -> str:
    """
    Gera um resumo dos relatórios.

    Args:
        reports: Dicionário com os resultados das análises.

    Returns:
        Caminho do arquivo de resumo.
    """
    print('\nGerando resumo...')

    # Cria diretório para relatórios se não existir
    report_dir = 'reports'
    os.makedirs(report_dir, exist_ok=True)

    # Nome do arquivo de resumo
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    summary_file = f'{report_dir}/summary_{timestamp}.txt'

    with open(summary_file, 'w', encoding='utf-8') as f:
        f.write('=== Resumo das Análises de Código ===\n\n')
        f.write(f'Data: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}\n\n')

        for tool, (success, report_file) in reports.items():
            status = 'Sucesso' if success else 'Falha'
            f.write(f'{tool}:\n')
            f.write(f'  Status: {status}\n')
            f.write(f'  Relatório: {report_file}\n\n')

    return summary_file

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as análises foram bem sucedidas, 1 caso contrário.
    """
    print('Iniciando análise de código...\n')

    # Executa as análises
    reports = {
        'Clang-Tidy': run_clang_tidy(),
        'Cppcheck': run_cppcheck(),
        'Include-What-You-Use': run_include_what_you_use(),
        'Clang-Format': run_clang_format_check(),
        'Análise de Complexidade': run_complexity_analysis()
    }

    # Gera o resumo
    summary_file = generate_summary(reports)
    print(f'\nResumo salvo em: {summary_file}')

    # Verifica se todas as análises foram bem sucedidas
    success = all(result[0] for result in reports.values())
    if not success:
        print('\nAlgumas análises falharam. Verifique o resumo para mais detalhes.',
              file=sys.stderr)
        return 1

    print('\nTodas as análises foram concluídas com sucesso!')
    return 0

if __name__ == '__main__':
    sys.exit(main())
