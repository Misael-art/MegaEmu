#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de CI/CD do emulador.
"""

import os
import sys
import json
import shutil
import logging
import logging.handlers
import subprocess
import yaml
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

# Configurações de CI/CD
CI_CONFIG = {
    'directories': {
        'ci': 'ci',
        'workflows': 'ci/workflows',
        'scripts': 'ci/scripts',
        'templates': 'ci/templates',
        'logs': 'ci/logs'
    },
    'workflows': {
        'build': {
            'name': 'Build',
            'triggers': {
                'push': ['main', 'develop'],
                'pull_request': ['main', 'develop']
            },
            'platforms': ['windows', 'linux', 'macos'],
            'steps': [
                {
                    'name': 'Checkout',
                    'uses': 'actions/checkout@v4'
                },
                {
                    'name': 'Setup',
                    'uses': 'actions/setup-python@v4',
                    'with': {
                        'python-version': '3.12'
                    }
                },
                {
                    'name': 'Dependencies',
                    'run': 'pip install -r requirements.txt'
                },
                {
                    'name': 'Build',
                    'run': 'python scripts/manage_build.py build'
                },
                {
                    'name': 'Test',
                    'run': 'python scripts/manage_test.py test'
                }
            ]
        },
        'docs': {
            'name': 'Documentation',
            'triggers': {
                'push': ['main'],
                'schedule': ['0 0 * * 0']  # Domingo 00:00
            },
            'platforms': ['ubuntu-latest'],
            'steps': [
                {
                    'name': 'Checkout',
                    'uses': 'actions/checkout@v4'
                },
                {
                    'name': 'Setup',
                    'uses': 'actions/setup-python@v4',
                    'with': {
                        'python-version': '3.12'
                    }
                },
                {
                    'name': 'Dependencies',
                    'run': 'pip install -r requirements.txt'
                },
                {
                    'name': 'Build',
                    'run': 'python scripts/manage_docs.py all'
                },
                {
                    'name': 'Deploy',
                    'uses': 'peaceiris/actions-gh-pages@v3',
                    'with': {
                        'github_token': '${{ secrets.GITHUB_TOKEN }}',
                        'publish_dir': './docs'
                    }
                }
            ]
        },
        'release': {
            'name': 'Release',
            'triggers': {
                'push': {
                    'tags': ['v*']
                }
            },
            'platforms': ['ubuntu-latest'],
            'steps': [
                {
                    'name': 'Checkout',
                    'uses': 'actions/checkout@v4'
                },
                {
                    'name': 'Setup',
                    'uses': 'actions/setup-python@v4',
                    'with': {
                        'python-version': '3.12'
                    }
                },
                {
                    'name': 'Dependencies',
                    'run': 'pip install -r requirements.txt'
                },
                {
                    'name': 'Build',
                    'run': 'python scripts/manage_build.py build'
                },
                {
                    'name': 'Package',
                    'run': 'python scripts/manage_package.py create'
                },
                {
                    'name': 'Release',
                    'uses': 'softprops/action-gh-release@v1',
                    'with': {
                        'files': 'dist/*'
                    }
                }
            ]
        },
        'coverage': {
            'name': 'Coverage',
            'triggers': {
                'push': ['main', 'develop'],
                'pull_request': ['main', 'develop']
            },
            'platforms': ['ubuntu-latest'],
            'steps': [
                {
                    'name': 'Checkout',
                    'uses': 'actions/checkout@v4'
                },
                {
                    'name': 'Setup',
                    'uses': 'actions/setup-python@v4',
                    'with': {
                        'python-version': '3.12'
                    }
                },
                {
                    'name': 'Dependencies',
                    'run': 'pip install -r requirements.txt'
                },
                {
                    'name': 'Coverage',
                    'run': 'python scripts/manage_test.py test'
                },
                {
                    'name': 'Upload',
                    'uses': 'codecov/codecov-action@v3'
                }
            ]
        },
        'sanitize': {
            'name': 'Sanitize',
            'triggers': {
                'push': ['main', 'develop'],
                'pull_request': ['main', 'develop']
            },
            'platforms': ['ubuntu-latest'],
            'steps': [
                {
                    'name': 'Checkout',
                    'uses': 'actions/checkout@v4'
                },
                {
                    'name': 'Setup',
                    'uses': 'actions/setup-python@v4',
                    'with': {
                        'python-version': '3.12'
                    }
                },
                {
                    'name': 'Dependencies',
                    'run': 'pip install -r requirements.txt'
                },
                {
                    'name': 'Format',
                    'run': 'python scripts/manage_format.py check'
                },
                {
                    'name': 'Lint',
                    'run': 'python scripts/manage_lint.py check'
                },
                {
                    'name': 'Security',
                    'run': 'python scripts/manage_security.py check'
                }
            ]
        }
    },
    'scripts': {
        'setup': {
            'name': 'setup.sh',
            'description': 'Configura ambiente de desenvolvimento',
            'content': [
                '#!/bin/bash',
                '',
                '# Instala dependências',
                'pip install -r requirements.txt',
                '',
                '# Configura git hooks',
                'pre-commit install',
                '',
                '# Inicializa submodules',
                'git submodule update --init --recursive'
            ]
        },
        'format': {
            'name': 'format.sh',
            'description': 'Formata código fonte',
            'content': [
                '#!/bin/bash',
                '',
                '# Formata Python',
                'black .',
                'isort .',
                '',
                '# Formata C++',
                'clang-format -i src/**/*.cpp include/**/*.hpp'
            ]
        },
        'analyze': {
            'name': 'analyze.sh',
            'description': 'Analisa código fonte',
            'content': [
                '#!/bin/bash',
                '',
                '# Análise estática',
                'mypy .',
                'flake8 .',
                'pylint **/*.py',
                '',
                '# Análise de segurança',
                'bandit -r .',
                'safety check'
            ]
        }
    },
    'templates': {
        'issue': {
            'name': 'ISSUE_TEMPLATE.md',
            'content': [
                '## Descrição',
                '',
                'Descreva o problema aqui.',
                '',
                '## Passos para Reproduzir',
                '',
                '1. Primeiro passo',
                '2. Segundo passo',
                '3. ...',
                '',
                '## Comportamento Esperado',
                '',
                'Descreva o que deveria acontecer.',
                '',
                '## Comportamento Atual',
                '',
                'Descreva o que está acontecendo.',
                '',
                '## Ambiente',
                '',
                '- OS: [Windows/Linux/macOS]',
                '- Versão: [x.y.z]',
                '- Hardware: [CPU/GPU/RAM]'
            ]
        },
        'pull_request': {
            'name': 'PULL_REQUEST_TEMPLATE.md',
            'content': [
                '## Descrição',
                '',
                'Descreva as mudanças aqui.',
                '',
                '## Tipo de Mudança',
                '',
                '- [ ] Bug fix',
                '- [ ] Nova feature',
                '- [ ] Breaking change',
                '- [ ] Documentação',
                '',
                '## Checklist',
                '',
                '- [ ] Testes adicionados/atualizados',
                '- [ ] Documentação atualizada',
                '- [ ] Código formatado',
                '- [ ] Lint passou',
                '- [ ] Build passou'
            ]
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
        for directory in CI_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def create_workflows() -> bool:
    """
    Cria workflows do GitHub Actions.

    Returns:
        True se os workflows foram criados com sucesso, False caso contrário.
    """
    try:
        print('\nCriando workflows...')

        for workflow_id, workflow in CI_CONFIG['workflows'].items():
            # Prepara workflow
            data = {
                'name': workflow['name'],
                'on': {},
                'jobs': {
                    'build': {
                        'runs-on': '${{ matrix.os }}',
                        'strategy': {
                            'matrix': {
                                'os': workflow['platforms']
                            }
                        },
                        'steps': workflow['steps']
                    }
                }
            }

            # Configura triggers
            for trigger, value in workflow['triggers'].items():
                if isinstance(value, list):
                    data['on'][trigger] = {
                        'branches': value
                    }
                elif isinstance(value, dict):
                    data['on'][trigger] = value
                else:
                    data['on'][trigger] = value

            # Salva workflow
            workflow_path = os.path.join(
                CI_CONFIG['directories']['workflows'],
                f'{workflow_id}.yml'
            )
            with open(workflow_path, 'w') as f:
                yaml.dump(data, f, sort_keys=False)

        return True
    except Exception as e:
        print(f'Erro ao criar workflows: {e}', file=sys.stderr)
        return False

def create_scripts() -> bool:
    """
    Cria scripts de CI.

    Returns:
        True se os scripts foram criados com sucesso, False caso contrário.
    """
    try:
        print('\nCriando scripts...')

        for script_id, script in CI_CONFIG['scripts'].items():
            # Salva script
            script_path = os.path.join(
                CI_CONFIG['directories']['scripts'],
                script['name']
            )
            with open(script_path, 'w') as f:
                f.write('\n'.join(script['content']))

            # Torna executável
            os.chmod(script_path, 0o755)

        return True
    except Exception as e:
        print(f'Erro ao criar scripts: {e}', file=sys.stderr)
        return False

def create_templates() -> bool:
    """
    Cria templates do GitHub.

    Returns:
        True se os templates foram criados com sucesso, False caso contrário.
    """
    try:
        print('\nCriando templates...')

        for template_id, template in CI_CONFIG['templates'].items():
            # Salva template
            template_path = os.path.join(
                CI_CONFIG['directories']['templates'],
                template['name']
            )
            with open(template_path, 'w') as f:
                f.write('\n'.join(template['content']))

        return True
    except Exception as e:
        print(f'Erro ao criar templates: {e}', file=sys.stderr)
        return False

def setup_logging() -> bool:
    """
    Configura sistema de logging.

    Returns:
        True se a configuração foi bem sucedida, False caso contrário.
    """
    try:
        if not CI_CONFIG['logging']['enabled']:
            return True

        # Configura logger
        logger = logging.getLogger('ci')
        logger.setLevel(CI_CONFIG['logging']['level'])

        # Handler para arquivo
        log_path = os.path.join(
            CI_CONFIG['directories']['logs'],
            'ci.log'
        )
        handler = logging.handlers.TimedRotatingFileHandler(
            log_path,
            when=CI_CONFIG['logging']['rotation']['when'],
            interval=CI_CONFIG['logging']['rotation']['interval'],
            backupCount=CI_CONFIG['logging']['rotation']['backupCount']
        )
        handler.setFormatter(logging.Formatter(
            CI_CONFIG['logging']['format']
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
        print('Uso: manage_ci.py <comando>', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init       Cria estrutura de diretórios', file=sys.stderr)
        print('  workflows  Cria workflows do GitHub Actions', file=sys.stderr)
        print('  scripts    Cria scripts de CI', file=sys.stderr)
        print('  templates  Cria templates do GitHub', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        if not create_directories():
            return 1
        if not setup_logging():
            return 1
        return 0

    elif command == 'workflows':
        if not create_workflows():
            return 1
        return 0

    elif command == 'scripts':
        if not create_scripts():
            return 1
        return 0

    elif command == 'templates':
        if not create_templates():
            return 1
        return 0

    else:
        print('Comando inválido.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
