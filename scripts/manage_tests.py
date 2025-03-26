#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os testes do emulador.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import xml.etree.ElementTree as ET

# Tipos de testes
TEST_TYPES = {
    'unit': {
        'description': 'Testes unitários',
        'directory': 'tests/unit',
        'pattern': '*_test.cpp'
    },
    'integration': {
        'description': 'Testes de integração',
        'directory': 'tests/integration',
        'pattern': '*_test.cpp'
    },
    'system': {
        'description': 'Testes de sistema',
        'directory': 'tests/system',
        'pattern': '*_test.cpp'
    },
    'performance': {
        'description': 'Testes de desempenho',
        'directory': 'tests/performance',
        'pattern': '*_test.cpp'
    }
}

# Categorias de testes
TEST_CATEGORIES = [
    'cpu',          # Testes da CPU
    'memory',       # Testes de memória
    'io',          # Testes de I/O
    'video',       # Testes de vídeo
    'audio',       # Testes de áudio
    'input',       # Testes de entrada
    'cartridge',   # Testes de cartucho
    'save',        # Testes de saves
    'system',      # Testes do sistema
    'other'        # Outros testes
]

def create_test_directories() -> bool:
    """
    Cria a estrutura de diretórios para testes.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        # Cria diretórios principais
        dirs = ['tests']
        for test_type in TEST_TYPES:
            dirs.append(TEST_TYPES[test_type]['directory'])
            dirs.append(f"{TEST_TYPES[test_type]['directory']}/results")

        for directory in dirs:
            os.makedirs(directory, exist_ok=True)

        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def create_test_template(test_type: str, category: str, name: str) -> bool:
    """
    Cria um novo arquivo de teste a partir do template.

    Args:
        test_type: Tipo do teste.
        category: Categoria do teste.
        name: Nome do teste.

    Returns:
        True se o teste foi criado com sucesso, False caso contrário.
    """
    try:
        # Valida tipo de teste
        if test_type not in TEST_TYPES:
            print(f'Tipo de teste inválido: {test_type}', file=sys.stderr)
            return False

        # Valida categoria
        if category not in TEST_CATEGORIES:
            print(f'Categoria de teste inválida: {category}', file=sys.stderr)
            return False

        # Define nome do arquivo
        test_file = f"{TEST_TYPES[test_type]['directory']}/{category}_{name}_test.cpp"

        # Verifica se arquivo já existe
        if os.path.exists(test_file):
            print(f'Arquivo já existe: {test_file}', file=sys.stderr)
            return False

        # Cria arquivo de teste
        with open(test_file, 'w', encoding='utf-8') as f:
            f.write(f'/**\n')
            f.write(f' * {name.replace("_", " ").title()} Test\n')
            f.write(f' * Tipo: {TEST_TYPES[test_type]["description"]}\n')
            f.write(f' * Categoria: {category}\n')
            f.write(f' * Data: {datetime.now().strftime("%Y-%m-%d")}\n')
            f.write(f' */\n\n')
            f.write('#include <gtest/gtest.h>\n')
            f.write('#include <gmock/gmock.h>\n\n')
            f.write('namespace {\n\n')
            f.write(f'class {name.title()}Test : public ::testing::Test {{\n')
            f.write('protected:\n')
            f.write('    void SetUp() override {\n')
            f.write('        // TODO: Inicialização\n')
            f.write('    }\n\n')
            f.write('    void TearDown() override {\n')
            f.write('        // TODO: Limpeza\n')
            f.write('    }\n')
            f.write('};\n\n')
            f.write(f'TEST_F({name.title()}Test, ShouldPass) {{\n')
            f.write('    // TODO: Implementar teste\n')
            f.write('    EXPECT_TRUE(true);\n')
            f.write('}\n\n')
            f.write('}  // namespace\n')

        print(f'Teste criado em: {test_file}')
        return True
    except Exception as e:
        print(f'Erro ao criar teste: {e}', file=sys.stderr)
        return False

def run_tests(test_type: Optional[str] = None, category: Optional[str] = None,
              name: Optional[str] = None, repeat: int = 1) -> bool:
    """
    Executa testes.

    Args:
        test_type: Tipo de teste para filtrar (opcional).
        category: Categoria para filtrar (opcional).
        name: Nome do teste para filtrar (opcional).
        repeat: Número de vezes para repetir os testes.

    Returns:
        True se todos os testes passaram, False caso contrário.
    """
    try:
        # Define diretórios a serem testados
        test_dirs = []
        if test_type:
            if test_type not in TEST_TYPES:
                print(f'Tipo de teste inválido: {test_type}', file=sys.stderr)
                return False
            test_dirs.append(TEST_TYPES[test_type]['directory'])
        else:
            for test_type in TEST_TYPES:
                test_dirs.append(TEST_TYPES[test_type]['directory'])

        # Compila testes
        print('Compilando testes...')
        if subprocess.run(['cmake', '--build', 'build', '--target', 'tests']).returncode != 0:
            print('Erro ao compilar testes.', file=sys.stderr)
            return False

        # Executa testes
        success = True
        for _ in range(repeat):
            for test_dir in test_dirs:
                # Define filtro
                filter_str = ''
                if category:
                    filter_str += f'{category}_'
                if name:
                    filter_str += f'*{name}*'

                # Define arquivo de resultado
                timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
                result_file = f'{test_dir}/results/test_result_{timestamp}.xml'

                # Executa testes
                print(f'\nExecutando testes em {test_dir}...')
                result = subprocess.run(
                    ['ctest', '--test-dir', 'build',
                     '--output-junit', result_file,
                     '--output-on-failure',
                     f'--tests-regex={filter_str}' if filter_str else ''],
                    capture_output=True,
                    text=True
                )

                # Verifica resultado
                if result.returncode != 0:
                    success = False
                    print('Falha nos testes:', file=sys.stderr)
                    print(result.stdout)
                    print(result.stderr, file=sys.stderr)
                else:
                    print('Testes concluídos com sucesso.')

        return success
    except Exception as e:
        print(f'Erro ao executar testes: {e}', file=sys.stderr)
        return False

def analyze_results(test_type: Optional[str] = None,
                   start_time: Optional[str] = None,
                   end_time: Optional[str] = None) -> Dict:
    """
    Analisa resultados dos testes.

    Args:
        test_type: Tipo de teste para filtrar (opcional).
        start_time: Data/hora inicial para filtrar (opcional).
        end_time: Data/hora final para filtrar (opcional).

    Returns:
        Dicionário com análise dos resultados.
    """
    try:
        analysis = {
            'total': 0,
            'passed': 0,
            'failed': 0,
            'skipped': 0,
            'time': 0.0,
            'by_category': {},
            'failures': []
        }

        # Inicializa contadores por categoria
        for category in TEST_CATEGORIES:
            analysis['by_category'][category] = {
                'total': 0,
                'passed': 0,
                'failed': 0,
                'skipped': 0
            }

        # Define diretórios a analisar
        result_dirs = []
        if test_type:
            if test_type not in TEST_TYPES:
                print(f'Tipo de teste inválido: {test_type}', file=sys.stderr)
                return analysis
            result_dirs.append(f"{TEST_TYPES[test_type]['directory']}/results")
        else:
            for test_type in TEST_TYPES:
                result_dirs.append(f"{TEST_TYPES[test_type]['directory']}/results")

        # Analisa arquivos de resultado
        for result_dir in result_dirs:
            if not os.path.exists(result_dir):
                continue

            for result_file in os.listdir(result_dir):
                if not result_file.endswith('.xml'):
                    continue

                # Extrai timestamp do nome do arquivo
                timestamp = result_file.split('_')[2].split('.')[0]
                if start_time and timestamp < start_time:
                    continue
                if end_time and timestamp > end_time:
                    continue

                # Carrega arquivo XML
                tree = ET.parse(f'{result_dir}/{result_file}')
                root = tree.getroot()

                # Analisa resultados
                for test_suite in root.findall('.//testsuite'):
                    # Extrai categoria do nome do teste
                    suite_name = test_suite.get('name', '')
                    category = next((c for c in TEST_CATEGORIES
                                  if suite_name.startswith(c)), 'other')

                    # Atualiza contadores
                    tests = int(test_suite.get('tests', 0))
                    failures = int(test_suite.get('failures', 0))
                    skipped = int(test_suite.get('skipped', 0))
                    time = float(test_suite.get('time', 0))

                    analysis['total'] += tests
                    analysis['failed'] += failures
                    analysis['skipped'] += skipped
                    analysis['passed'] += (tests - failures - skipped)
                    analysis['time'] += time

                    analysis['by_category'][category]['total'] += tests
                    analysis['by_category'][category]['failed'] += failures
                    analysis['by_category'][category]['skipped'] += skipped
                    analysis['by_category'][category]['passed'] += (tests - failures - skipped)

                    # Registra falhas
                    for test_case in test_suite.findall('.//testcase'):
                        failure = test_case.find('failure')
                        if failure is not None:
                            analysis['failures'].append({
                                'suite': suite_name,
                                'test': test_case.get('name', ''),
                                'message': failure.get('message', ''),
                                'details': failure.text
                            })

        return analysis
    except Exception as e:
        print(f'Erro ao analisar resultados: {e}', file=sys.stderr)
        return analysis

def generate_report(analysis: Dict, output_path: str) -> bool:
    """
    Gera relatório de testes.

    Args:
        analysis: Análise dos resultados.
        output_path: Caminho do arquivo de saída.

    Returns:
        True se o relatório foi gerado com sucesso, False caso contrário.
    """
    try:
        # Cria diretório de saída se necessário
        os.makedirs(os.path.dirname(output_path), exist_ok=True)

        # Gera relatório em formato Markdown
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write('# Relatório de Testes\n\n')
            f.write(f'Data: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}\n\n')

            # Resumo
            f.write('## Resumo\n\n')
            f.write(f'- Total de testes: {analysis["total"]}\n')
            f.write(f'- Passou: {analysis["passed"]} ')
            f.write(f'({analysis["passed"]/analysis["total"]*100:.1f}%)\n')
            f.write(f'- Falhou: {analysis["failed"]} ')
            f.write(f'({analysis["failed"]/analysis["total"]*100:.1f}%)\n')
            f.write(f'- Ignorado: {analysis["skipped"]} ')
            f.write(f'({analysis["skipped"]/analysis["total"]*100:.1f}%)\n')
            f.write(f'- Tempo total: {analysis["time"]:.2f}s\n\n')

            # Por categoria
            f.write('## Por Categoria\n\n')
            for category in TEST_CATEGORIES:
                stats = analysis['by_category'][category]
                if stats['total'] > 0:
                    f.write(f'### {category.title()}\n\n')
                    f.write(f'- Total: {stats["total"]}\n')
                    f.write(f'- Passou: {stats["passed"]} ')
                    f.write(f'({stats["passed"]/stats["total"]*100:.1f}%)\n')
                    f.write(f'- Falhou: {stats["failed"]} ')
                    f.write(f'({stats["failed"]/stats["total"]*100:.1f}%)\n')
                    f.write(f'- Ignorado: {stats["skipped"]} ')
                    f.write(f'({stats["skipped"]/stats["total"]*100:.1f}%)\n\n')

            # Falhas
            if analysis['failures']:
                f.write('## Falhas\n\n')
                for failure in analysis['failures']:
                    f.write(f'### {failure["suite"]} - {failure["test"]}\n\n')
                    f.write(f'**Mensagem**: {failure["message"]}\n\n')
                    f.write('**Detalhes**:\n')
                    f.write('```\n')
                    f.write(failure['details'])
                    f.write('\n```\n\n')

        print(f'Relatório gerado em: {output_path}')
        return True
    except Exception as e:
        print(f'Erro ao gerar relatório: {e}', file=sys.stderr)
        return False

def print_analysis(analysis: Dict) -> None:
    """
    Imprime análise dos resultados.

    Args:
        analysis: Dicionário com análise dos resultados.
    """
    print('\nResumo dos Testes:')
    print(f'Total: {analysis["total"]}')
    print(f'Passou: {analysis["passed"]} ({analysis["passed"]/analysis["total"]*100:.1f}%)')
    print(f'Falhou: {analysis["failed"]} ({analysis["failed"]/analysis["total"]*100:.1f}%)')
    print(f'Ignorado: {analysis["skipped"]} ({analysis["skipped"]/analysis["total"]*100:.1f}%)')
    print(f'Tempo total: {analysis["time"]:.2f}s')

    print('\nPor Categoria:')
    for category in TEST_CATEGORIES:
        stats = analysis['by_category'][category]
        if stats['total'] > 0:
            print(f'\n{category.title()}:')
            print(f'  Total: {stats["total"]}')
            print(f'  Passou: {stats["passed"]} ({stats["passed"]/stats["total"]*100:.1f}%)')
            print(f'  Falhou: {stats["failed"]} ({stats["failed"]/stats["total"]*100:.1f}%)')
            print(f'  Ignorado: {stats["skipped"]} ({stats["skipped"]/stats["total"]*100:.1f}%)')

    if analysis['failures']:
        print('\nFalhas:')
        for failure in analysis['failures']:
            print(f'\n{failure["suite"]} - {failure["test"]}')
            print(f'Mensagem: {failure["message"]}')
            print('Detalhes:')
            print(failure['details'])

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_tests.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  create <tipo> <categoria> <nome>')
        print('                        Cria novo teste', file=sys.stderr)
        print('  run [tipo] [categoria] [nome] [repetições]')
        print('                        Executa testes', file=sys.stderr)
        print('  analyze [tipo] [início] [fim]')
        print('                        Analisa resultados', file=sys.stderr)
        print('  report <arquivo>')
        print('                        Gera relatório', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_test_directories() else 1

    elif command == 'create' and len(sys.argv) > 4:
        return 0 if create_test_template(
            sys.argv[2],  # tipo
            sys.argv[3],  # categoria
            sys.argv[4]   # nome
        ) else 1

    elif command == 'run':
        test_type = sys.argv[2] if len(sys.argv) > 2 else None
        category = sys.argv[3] if len(sys.argv) > 3 else None
        name = sys.argv[4] if len(sys.argv) > 4 else None
        repeat = int(sys.argv[5]) if len(sys.argv) > 5 else 1

        return 0 if run_tests(test_type, category, name, repeat) else 1

    elif command == 'analyze':
        test_type = sys.argv[2] if len(sys.argv) > 2 else None
        start_time = sys.argv[3] if len(sys.argv) > 3 else None
        end_time = sys.argv[4] if len(sys.argv) > 4 else None

        analysis = analyze_results(test_type, start_time, end_time)
        print_analysis(analysis)
        return 0

    elif command == 'report' and len(sys.argv) > 2:
        analysis = analyze_results()
        return 0 if generate_report(analysis, sys.argv[2]) else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
