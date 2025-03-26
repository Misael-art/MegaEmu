#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de documentação do emulador.
"""

import os
import sys
import json
import shutil
import logging
import logging.handlers
import subprocess
import markdown
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

# Configurações de documentação
DOCS_CONFIG = {
    'directories': {
        'docs': 'docs',
        'api': 'docs/api',
        'manual': 'docs/manual',
        'guides': 'docs/guides',
        'examples': 'docs/examples',
        'images': 'docs/images',
        'temp': 'docs/temp',
        'logs': 'docs/logs'
    },
    'manual': {
        'sections': {
            'installation': {
                'title': 'Instalação',
                'order': 1,
                'subsections': [
                    'Requisitos',
                    'Download',
                    'Instalação',
                    'Configuração'
                ]
            },
            'interface': {
                'title': 'Interface',
                'order': 2,
                'subsections': [
                    'Menu Principal',
                    'Barra de Ferramentas',
                    'Área de Jogo',
                    'Configurações'
                ]
            },
            'emulation': {
                'title': 'Emulação',
                'order': 3,
                'subsections': [
                    'Carregando ROMs',
                    'Estados de Save',
                    'Cheats',
                    'Netplay'
                ]
            },
            'video': {
                'title': 'Vídeo',
                'order': 4,
                'subsections': [
                    'Resolução',
                    'Filtros',
                    'Shaders',
                    'Overscan'
                ]
            },
            'audio': {
                'title': 'Áudio',
                'order': 5,
                'subsections': [
                    'Configurações',
                    'Latência',
                    'Efeitos',
                    'Mixagem'
                ]
            },
            'input': {
                'title': 'Controles',
                'order': 6,
                'subsections': [
                    'Teclado',
                    'Gamepad',
                    'Mapeamento',
                    'Hotkeys'
                ]
            },
            'debug': {
                'title': 'Debug',
                'order': 7,
                'subsections': [
                    'Console',
                    'Memória',
                    'Registradores',
                    'Breakpoints'
                ]
            }
        }
    },
    'guides': {
        'categories': {
            'beginner': {
                'title': 'Iniciante',
                'order': 1,
                'guides': [
                    'Primeiros Passos',
                    'Interface Básica',
                    'Controles Básicos'
                ]
            },
            'intermediate': {
                'title': 'Intermediário',
                'order': 2,
                'guides': [
                    'Configuração Avançada',
                    'Shaders e Filtros',
                    'Netplay Básico'
                ]
            },
            'advanced': {
                'title': 'Avançado',
                'order': 3,
                'guides': [
                    'Debugging',
                    'Desenvolvimento',
                    'Contribuição'
                ]
            }
        }
    },
    'examples': {
        'categories': {
            'basic': {
                'title': 'Básico',
                'order': 1,
                'examples': [
                    'Hello World',
                    'Input Básico',
                    'Sprites Simples'
                ]
            },
            'intermediate': {
                'title': 'Intermediário',
                'order': 2,
                'examples': [
                    'Animação',
                    'Colisão',
                    'Áudio'
                ]
            },
            'advanced': {
                'title': 'Avançado',
                'order': 3,
                'examples': [
                    'Parallax',
                    'Mode 7',
                    'FM Synthesis'
                ]
            }
        }
    },
    'api': {
        'modules': [
            'cpu',
            'vdp',
            'audio',
            'input',
            'memory',
            'debug'
        ],
        'format': 'html',
        'private': False,
        'source': 'src'
    },
    'format': {
        'html': {
            'template': 'default',
            'theme': 'light',
            'syntax': 'monokai',
            'toc': True
        },
        'pdf': {
            'paper': 'a4',
            'toc': True,
            'numbered': True,
            'cover': True
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
        for directory in DOCS_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def generate_manual() -> bool:
    """
    Gera manual do usuário.

    Returns:
        True se o manual foi gerado com sucesso, False caso contrário.
    """
    try:
        print('\nGerando manual do usuário...')

        # Gera índice
        index = ['# Manual do Usuário\n']
        for section_id, section in sorted(
            DOCS_CONFIG['manual']['sections'].items(),
            key=lambda x: x[1]['order']
        ):
            # Título da seção
            index.append(f'\n## {section["title"]}\n')

            # Subseções
            for subsection in section['subsections']:
                index.append(f'- {subsection}\n')

            # Gera arquivo da seção
            section_path = os.path.join(
                DOCS_CONFIG['directories']['manual'],
                f'{section_id}.md'
            )
            with open(section_path, 'w') as f:
                f.write(f'# {section["title"]}\n\n')
                for subsection in section['subsections']:
                    f.write(f'## {subsection}\n\n')
                    f.write('TODO: Documentar esta seção.\n\n')

        # Salva índice
        index_path = os.path.join(
            DOCS_CONFIG['directories']['manual'],
            'index.md'
        )
        with open(index_path, 'w') as f:
            f.writelines(index)

        # Converte para HTML
        if DOCS_CONFIG['format']['html']['template'] == 'default':
            for md_file in Path(DOCS_CONFIG['directories']['manual']).glob('*.md'):
                html_file = md_file.with_suffix('.html')
                with open(md_file) as f:
                    md = f.read()
                html = markdown.markdown(md)
                with open(html_file, 'w') as f:
                    f.write('<!DOCTYPE html>\n')
                    f.write('<html>\n')
                    f.write('<head>\n')
                    f.write('  <meta charset="utf-8">\n')
                    f.write('  <title>Manual do Usuário</title>\n')
                    f.write('  <style>\n')
                    f.write('    body { font-family: sans-serif; margin: 2em; }\n')
                    f.write('    h1 { color: #333; }\n')
                    f.write('    h2 { color: #666; }\n')
                    f.write('    code { background: #f4f4f4; padding: 0.2em; }\n')
                    f.write('  </style>\n')
                    f.write('</head>\n')
                    f.write('<body>\n')
                    f.write(html)
                    f.write('</body>\n')
                    f.write('</html>\n')

        return True
    except Exception as e:
        print(f'Erro ao gerar manual: {e}', file=sys.stderr)
        return False

def generate_guides() -> bool:
    """
    Gera guias do usuário.

    Returns:
        True se os guias foram gerados com sucesso, False caso contrário.
    """
    try:
        print('\nGerando guias do usuário...')

        # Gera índice
        index = ['# Guias do Usuário\n']
        for category_id, category in sorted(
            DOCS_CONFIG['guides']['categories'].items(),
            key=lambda x: x[1]['order']
        ):
            # Título da categoria
            index.append(f'\n## {category["title"]}\n')

            # Guias
            for guide in category['guides']:
                index.append(f'- {guide}\n')

            # Gera arquivos dos guias
            category_dir = os.path.join(
                DOCS_CONFIG['directories']['guides'],
                category_id
            )
            os.makedirs(category_dir, exist_ok=True)

            for guide in category['guides']:
                guide_path = os.path.join(
                    category_dir,
                    f'{guide.lower().replace(" ", "_")}.md'
                )
                with open(guide_path, 'w') as f:
                    f.write(f'# {guide}\n\n')
                    f.write('TODO: Documentar este guia.\n\n')

        # Salva índice
        index_path = os.path.join(
            DOCS_CONFIG['directories']['guides'],
            'index.md'
        )
        with open(index_path, 'w') as f:
            f.writelines(index)

        # Converte para HTML
        if DOCS_CONFIG['format']['html']['template'] == 'default':
            for md_file in Path(DOCS_CONFIG['directories']['guides']).rglob('*.md'):
                html_file = md_file.with_suffix('.html')
                with open(md_file) as f:
                    md = f.read()
                html = markdown.markdown(md)
                with open(html_file, 'w') as f:
                    f.write('<!DOCTYPE html>\n')
                    f.write('<html>\n')
                    f.write('<head>\n')
                    f.write('  <meta charset="utf-8">\n')
                    f.write('  <title>Guias do Usuário</title>\n')
                    f.write('  <style>\n')
                    f.write('    body { font-family: sans-serif; margin: 2em; }\n')
                    f.write('    h1 { color: #333; }\n')
                    f.write('    h2 { color: #666; }\n')
                    f.write('    code { background: #f4f4f4; padding: 0.2em; }\n')
                    f.write('  </style>\n')
                    f.write('</head>\n')
                    f.write('<body>\n')
                    f.write(html)
                    f.write('</body>\n')
                    f.write('</html>\n')

        return True
    except Exception as e:
        print(f'Erro ao gerar guias: {e}', file=sys.stderr)
        return False

def generate_examples() -> bool:
    """
    Gera exemplos de código.

    Returns:
        True se os exemplos foram gerados com sucesso, False caso contrário.
    """
    try:
        print('\nGerando exemplos de código...')

        # Gera índice
        index = ['# Exemplos de Código\n']
        for category_id, category in sorted(
            DOCS_CONFIG['examples']['categories'].items(),
            key=lambda x: x[1]['order']
        ):
            # Título da categoria
            index.append(f'\n## {category["title"]}\n')

            # Exemplos
            for example in category['examples']:
                index.append(f'- {example}\n')

            # Gera arquivos dos exemplos
            category_dir = os.path.join(
                DOCS_CONFIG['directories']['examples'],
                category_id
            )
            os.makedirs(category_dir, exist_ok=True)

            for example in category['examples']:
                example_path = os.path.join(
                    category_dir,
                    f'{example.lower().replace(" ", "_")}.md'
                )
                with open(example_path, 'w') as f:
                    f.write(f'# {example}\n\n')
                    f.write('```python\n')
                    f.write('# TODO: Adicionar código de exemplo\n')
                    f.write('```\n\n')

        # Salva índice
        index_path = os.path.join(
            DOCS_CONFIG['directories']['examples'],
            'index.md'
        )
        with open(index_path, 'w') as f:
            f.writelines(index)

        # Converte para HTML
        if DOCS_CONFIG['format']['html']['template'] == 'default':
            for md_file in Path(DOCS_CONFIG['directories']['examples']).rglob('*.md'):
                html_file = md_file.with_suffix('.html')
                with open(md_file) as f:
                    md = f.read()
                html = markdown.markdown(md)
                with open(html_file, 'w') as f:
                    f.write('<!DOCTYPE html>\n')
                    f.write('<html>\n')
                    f.write('<head>\n')
                    f.write('  <meta charset="utf-8">\n')
                    f.write('  <title>Exemplos de Código</title>\n')
                    f.write('  <style>\n')
                    f.write('    body { font-family: sans-serif; margin: 2em; }\n')
                    f.write('    h1 { color: #333; }\n')
                    f.write('    h2 { color: #666; }\n')
                    f.write('    code { background: #f4f4f4; padding: 0.2em; }\n')
                    f.write('  </style>\n')
                    f.write('</head>\n')
                    f.write('<body>\n')
                    f.write(html)
                    f.write('</body>\n')
                    f.write('</html>\n')

        return True
    except Exception as e:
        print(f'Erro ao gerar exemplos: {e}', file=sys.stderr)
        return False

def generate_api() -> bool:
    """
    Gera documentação da API.

    Returns:
        True se a documentação foi gerada com sucesso, False caso contrário.
    """
    try:
        print('\nGerando documentação da API...')

        # Gera documentação com pdoc
        api_dir = DOCS_CONFIG['directories']['api']
        source_dir = DOCS_CONFIG['api']['source']

        cmd = [
            'pdoc',
            '--html',
            '--output-dir', api_dir,
            '--force'
        ]

        if not DOCS_CONFIG['api']['private']:
            cmd.append('--skip-private')

        for module in DOCS_CONFIG['api']['modules']:
            cmd.append(os.path.join(source_dir, module))

        subprocess.run(cmd, check=True)

        return True
    except Exception as e:
        print(f'Erro ao gerar documentação da API: {e}', file=sys.stderr)
        return False

def setup_logging() -> bool:
    """
    Configura sistema de logging.

    Returns:
        True se a configuração foi bem sucedida, False caso contrário.
    """
    try:
        if not DOCS_CONFIG['logging']['enabled']:
            return True

        # Configura logger
        logger = logging.getLogger('docs')
        logger.setLevel(DOCS_CONFIG['logging']['level'])

        # Handler para arquivo
        log_path = os.path.join(
            DOCS_CONFIG['directories']['logs'],
            'docs.log'
        )
        handler = logging.handlers.TimedRotatingFileHandler(
            log_path,
            when=DOCS_CONFIG['logging']['rotation']['when'],
            interval=DOCS_CONFIG['logging']['rotation']['interval'],
            backupCount=DOCS_CONFIG['logging']['rotation']['backupCount']
        )
        handler.setFormatter(logging.Formatter(
            DOCS_CONFIG['logging']['format']
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
        print('Uso: manage_docs.py <comando>', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init      Cria estrutura de diretórios', file=sys.stderr)
        print('  manual    Gera manual do usuário', file=sys.stderr)
        print('  guides    Gera guias do usuário', file=sys.stderr)
        print('  examples  Gera exemplos de código', file=sys.stderr)
        print('  api       Gera documentação da API', file=sys.stderr)
        print('  all       Gera toda a documentação', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        if not create_directories():
            return 1
        if not setup_logging():
            return 1
        return 0

    elif command == 'manual':
        if not generate_manual():
            return 1
        return 0

    elif command == 'guides':
        if not generate_guides():
            return 1
        return 0

    elif command == 'examples':
        if not generate_examples():
            return 1
        return 0

    elif command == 'api':
        if not generate_api():
            return 1
        return 0

    elif command == 'all':
        if not generate_manual():
            return 1
        if not generate_guides():
            return 1
        if not generate_examples():
            return 1
        if not generate_api():
            return 1
        return 0

    else:
        print('Comando inválido.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
