#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de teste do emulador.
"""

import os
import sys
import json
import shutil
import logging
import logging.handlers
import subprocess
import unittest
import coverage
import pytest
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

# Configurações de teste
TEST_CONFIG = {
    'directories': {
        'test': 'test',
        'fixtures': 'test/fixtures',
        'coverage': 'test/coverage',
        'reports': 'test/reports',
        'logs': 'test/logs'
    },
    'suites': {
        'unit': {
            'enabled': True,
            'pattern': 'test_*.py',
            'directory': 'test/unit',
            'timeout': 60,  # segundos
            'parallel': True
        },
        'integration': {
            'enabled': True,
            'pattern': 'test_*.py',
            'directory': 'test/integration',
            'timeout': 300,
            'parallel': True
        },
        'system': {
            'enabled': True,
            'pattern': 'test_*.py',
            'directory': 'test/system',
            'timeout': 600,
            'parallel': False
        }
    },
    'fixtures': {
        'roms': {
            'directory': 'test/fixtures/roms',
            'files': [
                'test_cpu.md',
                'test_vdp.md',
                'test_audio.md',
                'test_input.md'
            ]
        },
        'saves': {
            'directory': 'test/fixtures/saves',
            'files': [
                'test_save.srm',
                'test_state.st0'
            ]
        },
        'configs': {
            'directory': 'test/fixtures/configs',
            'files': [
                'test_config.json',
                'test_mapping.json'
            ]
        }
    },
    'coverage': {
        'enabled': True,
        'branch': True,
        'source': ['src'],
        'omit': [
            'src/tests/*',
            'src/scripts/*'
        ],
        'report': {
            'show_missing': True,
            'skip_covered': True,
            'fail_under': 80
        }
    },
    'reports': {
        'formats': ['txt', 'html', 'xml'],
        'sections': {
            'summary': True,
            'errors': True,
            'failures': True,
            'skipped': True,
            'coverage': True
        }
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
        # Cria diretórios principais
        for directory in TEST_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)

        # Cria diretórios de suites
        for suite in TEST_CONFIG['suites'].values():
            os.makedirs(suite['directory'], exist_ok=True)

        # Cria diretórios de fixtures
        for fixture in TEST_CONFIG['fixtures'].values():
            os.makedirs(fixture['directory'], exist_ok=True)

        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def setup_fixtures() -> bool:
    """
    Configura fixtures de teste.

    Returns:
        True se as fixtures foram configuradas com sucesso, False caso contrário.
    """
    try:
        print('\nConfigurando fixtures...')

        # ROMs de teste
        rom_dir = TEST_CONFIG['fixtures']['roms']['directory']
        for rom in TEST_CONFIG['fixtures']['roms']['files']:
            rom_path = os.path.join(rom_dir, rom)
            if not os.path.exists(rom_path):
                # Cria ROM de teste
                with open(rom_path, 'wb') as f:
                    # Cabeçalho ROM
                    f.write(b'SEGA MEGA DRIVE')
                    # Dados de teste
                    f.write(os.urandom(16384))

        # Saves de teste
        save_dir = TEST_CONFIG['fixtures']['saves']['directory']
        for save in TEST_CONFIG['fixtures']['saves']['files']:
            save_path = os.path.join(save_dir, save)
            if not os.path.exists(save_path):
                # Cria save de teste
                with open(save_path, 'wb') as f:
                    # Dados de teste
                    f.write(os.urandom(8192))

        # Configs de teste
        config_dir = TEST_CONFIG['fixtures']['configs']['directory']
        for config in TEST_CONFIG['fixtures']['configs']['files']:
            config_path = os.path.join(config_dir, config)
            if not os.path.exists(config_path):
                # Cria config de teste
                if config.endswith('.json'):
                    with open(config_path, 'w') as f:
                        json.dump({
                            'test': True,
                            'timestamp': datetime.now().isoformat()
                        }, f, indent=2)

        return True
    except Exception as e:
        print(f'Erro ao configurar fixtures: {e}', file=sys.stderr)
        return False

def run_tests(suite: str) -> Optional[unittest.TestResult]:
    """
    Executa suite de testes.

    Args:
        suite: Nome da suite de testes.

    Returns:
        Resultado dos testes ou None em caso de erro.
    """
    try:
        if suite not in TEST_CONFIG['suites']:
            print(f'Suite inválida: {suite}', file=sys.stderr)
            return None

        config = TEST_CONFIG['suites'][suite]
        if not config['enabled']:
            print(f'Suite desabilitada: {suite}', file=sys.stderr)
            return None

        print(f'\nExecutando suite {suite}...')

        # Configura cobertura
        if TEST_CONFIG['coverage']['enabled']:
            cov = coverage.Coverage(
                branch=TEST_CONFIG['coverage']['branch'],
                source=TEST_CONFIG['coverage']['source'],
                omit=TEST_CONFIG['coverage']['omit']
            )
            cov.start()

        # Executa testes
        loader = unittest.TestLoader()
        tests = loader.discover(
            config['directory'],
            pattern=config['pattern']
        )

        runner = unittest.TextTestRunner(
            verbosity=2,
            failfast=False
        )

        result = runner.run(tests)

        # Gera relatório de cobertura
        if TEST_CONFIG['coverage']['enabled']:
            cov.stop()
            cov.save()

            # Relatório em texto
            cov.report(
                show_missing=TEST_CONFIG['coverage']['report']['show_missing'],
                skip_covered=TEST_CONFIG['coverage']['report']['skip_covered']
            )

            # Relatório HTML
            cov.html_report(
                directory=os.path.join(
                    TEST_CONFIG['directories']['coverage'],
                    suite
                )
            )

            # Relatório XML
            cov.xml_report(
                outfile=os.path.join(
                    TEST_CONFIG['directories']['coverage'],
                    f'{suite}.xml'
                )
            )

            # Verifica cobertura mínima
            if cov.report() < TEST_CONFIG['coverage']['report']['fail_under']:
                print('\nCobertura abaixo do mínimo exigido.', file=sys.stderr)
                return None

        return result
    except Exception as e:
        print(f'Erro ao executar testes: {e}', file=sys.stderr)
        return None

def generate_report(results: Dict[str, unittest.TestResult]) -> bool:
    """
    Gera relatório de testes.

    Args:
        results: Resultados dos testes por suite.

    Returns:
        True se o relatório foi gerado com sucesso, False caso contrário.
    """
    try:
        if not results:
            return False

        print('\nGerando relatório...')

        # Prepara dados
        timestamp = datetime.now()
        summary = {
            'timestamp': timestamp.isoformat(),
            'suites': len(results),
            'tests': sum(result.testsRun for result in results.values()),
            'errors': sum(len(result.errors) for result in results.values()),
            'failures': sum(len(result.failures) for result in results.values()),
            'skipped': sum(len(result.skipped) for result in results.values())
        }

        # Gera relatórios
        for format in TEST_CONFIG['reports']['formats']:
            report_path = os.path.join(
                TEST_CONFIG['directories']['reports'],
                f'report_{timestamp.strftime("%Y%m%d_%H%M%S")}.{format}'
            )

            if format == 'txt':
                with open(report_path, 'w') as f:
                    # Cabeçalho
                    f.write('=== Relatório de Testes ===\n\n')

                    # Resumo
                    if TEST_CONFIG['reports']['sections']['summary']:
                        f.write('Resumo\n')
                        f.write('------\n')
                        for key, value in summary.items():
                            f.write(f'{key}: {value}\n')
                        f.write('\n')

                    # Detalhes por suite
                    for suite, result in results.items():
                        f.write(f'Suite: {suite}\n')
                        f.write('-' * (7 + len(suite)) + '\n')
                        f.write(f'Tests: {result.testsRun}\n')

                        if TEST_CONFIG['reports']['sections']['errors'] and result.errors:
                            f.write('\nErros:\n')
                            for test, error in result.errors:
                                f.write(f'- {test}\n')
                                f.write(f'  {error}\n')

                        if TEST_CONFIG['reports']['sections']['failures'] and result.failures:
                            f.write('\nFalhas:\n')
                            for test, failure in result.failures:
                                f.write(f'- {test}\n')
                                f.write(f'  {failure}\n')

                        if TEST_CONFIG['reports']['sections']['skipped'] and result.skipped:
                            f.write('\nIgnorados:\n')
                            for test, reason in result.skipped:
                                f.write(f'- {test}\n')
                                f.write(f'  {reason}\n')

                        f.write('\n')

            elif format == 'html':
                with open(report_path, 'w') as f:
                    f.write('<!DOCTYPE html>\n')
                    f.write('<html>\n')
                    f.write('<head>\n')
                    f.write('  <title>Relatório de Testes</title>\n')
                    f.write('  <style>\n')
                    f.write('    body { font-family: sans-serif; margin: 2em; }\n')
                    f.write('    h1 { color: #333; }\n')
                    f.write('    .summary { margin: 1em 0; }\n')
                    f.write('    .suite { margin: 2em 0; }\n')
                    f.write('    .error { color: red; }\n')
                    f.write('    .failure { color: orange; }\n')
                    f.write('    .skipped { color: gray; }\n')
                    f.write('  </style>\n')
                    f.write('</head>\n')
                    f.write('<body>\n')

                    f.write('  <h1>Relatório de Testes</h1>\n')

                    if TEST_CONFIG['reports']['sections']['summary']:
                        f.write('  <div class="summary">\n')
                        f.write('    <h2>Resumo</h2>\n')
                        f.write('    <ul>\n')
                        for key, value in summary.items():
                            f.write(f'      <li>{key}: {value}</li>\n')
                        f.write('    </ul>\n')
                        f.write('  </div>\n')

                    for suite, result in results.items():
                        f.write('  <div class="suite">\n')
                        f.write(f'    <h2>Suite: {suite}</h2>\n')
                        f.write(f'    <p>Tests: {result.testsRun}</p>\n')

                        if TEST_CONFIG['reports']['sections']['errors'] and result.errors:
                            f.write('    <h3>Erros</h3>\n')
                            f.write('    <ul class="error">\n')
                            for test, error in result.errors:
                                f.write(f'      <li>{test}<br><pre>{error}</pre></li>\n')
                            f.write('    </ul>\n')

                        if TEST_CONFIG['reports']['sections']['failures'] and result.failures:
                            f.write('    <h3>Falhas</h3>\n')
                            f.write('    <ul class="failure">\n')
                            for test, failure in result.failures:
                                f.write(f'      <li>{test}<br><pre>{failure}</pre></li>\n')
                            f.write('    </ul>\n')

                        if TEST_CONFIG['reports']['sections']['skipped'] and result.skipped:
                            f.write('    <h3>Ignorados</h3>\n')
                            f.write('    <ul class="skipped">\n')
                            for test, reason in result.skipped:
                                f.write(f'      <li>{test}<br><pre>{reason}</pre></li>\n')
                            f.write('    </ul>\n')

                        f.write('  </div>\n')

                    f.write('</body>\n')
                    f.write('</html>\n')

            elif format == 'xml':
                with open(report_path, 'w') as f:
                    f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
                    f.write('<testsuites>\n')

                    for suite, result in results.items():
                        f.write(f'  <testsuite name="{suite}" ')
                        f.write(f'tests="{result.testsRun}" ')
                        f.write(f'errors="{len(result.errors)}" ')
                        f.write(f'failures="{len(result.failures)}" ')
                        f.write(f'skipped="{len(result.skipped)}">\n')

                        if TEST_CONFIG['reports']['sections']['errors']:
                            for test, error in result.errors:
                                f.write(f'    <testcase name="{test}">\n')
                                f.write('      <error ')
                                f.write(f'message="{error.splitlines()[0]}">')
                                f.write(f'<![CDATA[{error}]]></error>\n')
                                f.write('    </testcase>\n')

                        if TEST_CONFIG['reports']['sections']['failures']:
                            for test, failure in result.failures:
                                f.write(f'    <testcase name="{test}">\n')
                                f.write('      <failure ')
                                f.write(f'message="{failure.splitlines()[0]}">')
                                f.write(f'<![CDATA[{failure}]]></failure>\n')
                                f.write('    </testcase>\n')

                        if TEST_CONFIG['reports']['sections']['skipped']:
                            for test, reason in result.skipped:
                                f.write(f'    <testcase name="{test}">\n')
                                f.write('      <skipped ')
                                f.write(f'message="{reason}"/>\n')
                                f.write('    </testcase>\n')

                        f.write('  </testsuite>\n')

                    f.write('</testsuites>\n')

        print('Relatório gerado com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao gerar relatório: {e}', file=sys.stderr)
        return False

def setup_logging() -> bool:
    """
    Configura sistema de logging.

    Returns:
        True se a configuração foi bem sucedida, False caso contrário.
    """
    try:
        if not TEST_CONFIG['logging']['enabled']:
            return True

        # Configura logger
        logger = logging.getLogger('test')
        logger.setLevel(TEST_CONFIG['logging']['level'])

        # Handler para arquivo
        log_path = os.path.join(
            TEST_CONFIG['directories']['logs'],
            'test.log'
        )
        handler = logging.handlers.TimedRotatingFileHandler(
            log_path,
            when=TEST_CONFIG['logging']['rotation']['when'],
            interval=TEST_CONFIG['logging']['rotation']['interval'],
            backupCount=TEST_CONFIG['logging']['rotation']['backupCount']
        )
        handler.setFormatter(logging.Formatter(
            TEST_CONFIG['logging']['format']
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
        print('Uso: manage_test.py <comando> [suite]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init      Cria estrutura de diretórios', file=sys.stderr)
        print('  fixtures  Configura fixtures de teste', file=sys.stderr)
        print('  test      Executa testes', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        if not create_directories():
            return 1
        if not setup_logging():
            return 1
        return 0

    elif command == 'fixtures':
        if not setup_fixtures():
            return 1
        return 0

    elif command == 'test':
        # Determina suites a executar
        if len(sys.argv) > 2:
            suites = [sys.argv[2]]
            if suites[0] not in TEST_CONFIG['suites']:
                print(f'Suite inválida: {suites[0]}', file=sys.stderr)
                return 1
        else:
            suites = [
                suite for suite, config in TEST_CONFIG['suites'].items()
                if config['enabled']
            ]

        # Executa testes
        results = {}
        for suite in suites:
            result = run_tests(suite)
            if not result:
                return 1
            results[suite] = result

        # Gera relatório
        if not generate_report(results):
            return 1

        # Verifica falhas
        if any(
            len(result.errors) + len(result.failures) > 0
            for result in results.values()
        ):
            return 1

        return 0

    else:
        print('Comando inválido.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
