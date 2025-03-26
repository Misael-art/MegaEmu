#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar a documentação do código.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import re

# Configurações de documentação
DOCS_CONFIG = {
    'doxygen': {
        'config': {
            'PROJECT_NAME': 'Mega Emu',
            'PROJECT_BRIEF': 'Emulador de Mega Drive/Genesis em C++',
            'OUTPUT_DIRECTORY': 'docs/api',
            'INPUT': 'src',
            'FILE_PATTERNS': ['*.cpp', '*.hpp', '*.h'],
            'RECURSIVE': 'YES',
            'EXTRACT_ALL': 'YES',
            'EXTRACT_PRIVATE': 'YES',
            'EXTRACT_STATIC': 'YES',
            'EXTRACT_LOCAL_CLASSES': 'YES',
            'EXTRACT_LOCAL_METHODS': 'YES',
            'EXTRACT_ANON_NSPACES': 'YES',
            'GENERATE_HTML': 'YES',
            'GENERATE_LATEX': 'NO',
            'GENERATE_XML': 'YES',
            'XML_PROGRAMLISTING': 'YES',
            'GENERATE_TREEVIEW': 'YES',
            'DISABLE_INDEX': 'NO',
            'FULL_SIDEBAR': 'NO',
            'HTML_EXTRA_STYLESHEET': 'docs/api/custom.css',
            'HTML_COLORSTYLE': 'LIGHT',
            'GENERATE_TODOLIST': 'YES',
            'GENERATE_TESTLIST': 'YES',
            'GENERATE_BUGLIST': 'YES',
            'GENERATE_DEPRECATEDLIST': 'YES',
            'SHOW_USED_FILES': 'YES',
            'SHOW_FILES': 'YES',
            'SHOW_NAMESPACES': 'YES',
            'SORT_BRIEF_DOCS': 'YES',
            'SORT_MEMBER_DOCS': 'YES',
            'SORT_GROUP_NAMES': 'YES',
            'SORT_BY_SCOPE_NAME': 'YES',
            'STRICT_PROTO_MATCHING': 'YES',
            'MULTILINE_CPP_IS_BRIEF': 'YES',
            'INHERIT_DOCS': 'YES',
            'INLINE_INHERITED_MEMB': 'YES',
            'INLINE_INFO': 'YES',
            'INLINE_SOURCES': 'NO',
            'SOURCE_BROWSER': 'YES',
            'REFERENCES_RELATION': 'YES',
            'REFERENCES_LINK_SOURCE': 'YES',
            'USE_MDFILE_AS_MAINPAGE': 'README.md',
            'CITE_BIB_FILES': 'docs/api/references.bib',
            'WARN_IF_UNDOCUMENTED': 'YES',
            'WARN_IF_DOC_ERROR': 'YES',
            'WARN_NO_PARAMDOC': 'YES',
            'WARN_AS_ERROR': 'NO'
        },
        'languages': {
            'pt_BR': {
                'OUTPUT_LANGUAGE': 'Brazilian',
                'ALIASES': [
                    'brief=@brief',
                    'details=@details',
                    'param=@param',
                    'return=@return',
                    'note=@note',
                    'warning=@warning',
                    'todo=@todo',
                    'bug=@bug',
                    'deprecated=@deprecated'
                ]
            },
            'en_US': {
                'OUTPUT_LANGUAGE': 'English',
                'ALIASES': [
                    'brief=@brief',
                    'details=@details',
                    'param=@param',
                    'return=@return',
                    'note=@note',
                    'warning=@warning',
                    'todo=@todo',
                    'bug=@bug',
                    'deprecated=@deprecated'
                ]
            }
        }
    },
    'sphinx': {
        'config': {
            'project': 'Mega Emu',
            'copyright': f'2024, {os.getenv("USER", "Your Name")}',
            'author': os.getenv('USER', 'Your Name'),
            'release': '1.0.0',
            'extensions': [
                'sphinx.ext.autodoc',
                'sphinx.ext.doctest',
                'sphinx.ext.intersphinx',
                'sphinx.ext.todo',
                'sphinx.ext.coverage',
                'sphinx.ext.mathjax',
                'sphinx.ext.ifconfig',
                'sphinx.ext.viewcode',
                'sphinx.ext.githubpages',
                'breathe'
            ],
            'templates_path': ['_templates'],
            'exclude_patterns': ['_build', 'Thumbs.db', '.DS_Store'],
            'html_theme': 'sphinx_rtd_theme',
            'html_static_path': ['_static'],
            'todo_include_todos': True,
            'breathe_projects': {'mega_emu': 'docs/api/xml'},
            'breathe_default_project': 'mega_emu'
        },
        'languages': {
            'pt_BR': {
                'language': 'pt_BR',
                'html_title': 'Documentação do Mega Emu',
                'html_short_title': 'Mega Emu',
                'html_search_language': 'pt'
            },
            'en_US': {
                'language': 'en',
                'html_title': 'Mega Emu Documentation',
                'html_short_title': 'Mega Emu',
                'html_search_language': 'en'
            }
        }
    }
}

def create_directories() -> bool:
    """
    Cria a estrutura de diretórios para documentação.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        # Cria diretórios principais
        dirs = [
            'docs/api',
            'docs/user',
            'docs/dev',
            'docs/examples'
        ]

        # Adiciona diretórios por idioma
        for language in DOCS_CONFIG['doxygen']['languages']:
            dirs.extend([
                f'docs/api/{language}',
                f'docs/user/{language}/source',
                f'docs/user/{language}/build',
                f'docs/dev/{language}'
            ])

        # Cria diretórios
        for directory in dirs:
            os.makedirs(directory, exist_ok=True)

        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def create_doxygen_config(language: str) -> bool:
    """
    Cria arquivo de configuração do Doxygen.

    Args:
        language: Idioma da documentação.

    Returns:
        True se o arquivo foi criado com sucesso, False caso contrário.
    """
    try:
        # Define caminho do arquivo
        config_file = f'docs/api/{language}/Doxyfile'

        # Obtém configurações
        config = {
            **DOCS_CONFIG['doxygen']['config'],
            **DOCS_CONFIG['doxygen']['languages'][language]
        }

        # Ajusta caminhos
        config['OUTPUT_DIRECTORY'] = f'docs/api/{language}'
        config['HTML_EXTRA_STYLESHEET'] = f'docs/api/{language}/custom.css'
        config['CITE_BIB_FILES'] = f'docs/api/{language}/references.bib'

        # Cria arquivo
        with open(config_file, 'w') as f:
            for key, value in config.items():
                if isinstance(value, list):
                    f.write(f'{key} = {" ".join(value)}\n')
                else:
                    f.write(f'{key} = {value}\n')

        return True
    except Exception as e:
        print(f'Erro ao criar arquivo de configuração do Doxygen: {e}',
              file=sys.stderr)
        return False

def create_sphinx_config(language: str) -> bool:
    """
    Cria arquivo de configuração do Sphinx.

    Args:
        language: Idioma da documentação.

    Returns:
        True se o arquivo foi criado com sucesso, False caso contrário.
    """
    try:
        # Define caminho do arquivo
        config_dir = f'docs/user/{language}/source'
        config_file = os.path.join(config_dir, 'conf.py')

        # Obtém configurações
        config = {
            **DOCS_CONFIG['sphinx']['config'],
            **DOCS_CONFIG['sphinx']['languages'][language]
        }

        # Ajusta caminhos
        config['breathe_projects']['mega_emu'] = f'../../api/{language}/xml'

        # Cria arquivo
        with open(config_file, 'w') as f:
            for key, value in config.items():
                if isinstance(value, list):
                    f.write(f'{key} = {value!r}\n')
                elif isinstance(value, dict):
                    f.write(f'{key} = {value!r}\n')
                elif isinstance(value, bool):
                    f.write(f'{key} = {str(value)}\n')
                else:
                    f.write(f'{key} = {value!r}\n')

        return True
    except Exception as e:
        print(f'Erro ao criar arquivo de configuração do Sphinx: {e}',
              file=sys.stderr)
        return False

def create_sphinx_index(language: str) -> bool:
    """
    Cria arquivo de índice do Sphinx.

    Args:
        language: Idioma da documentação.

    Returns:
        True se o arquivo foi criado com sucesso, False caso contrário.
    """
    try:
        # Define caminho do arquivo
        index_file = f'docs/user/{language}/source/index.rst'

        # Define conteúdo
        if language == 'pt_BR':
            content = """
Documentação do Mega Emu
=======================

Bem-vindo à documentação do Mega Emu!

.. toctree::
   :maxdepth: 2
   :caption: Conteúdo:

   intro
   install
   usage
   api
   dev
   examples
   changelog

Índices e tabelas
================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
"""
        else:
            content = """
Mega Emu Documentation
=====================

Welcome to Mega Emu's documentation!

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   intro
   install
   usage
   api
   dev
   examples
   changelog

Indices and tables
=================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
"""

        # Cria arquivo
        with open(index_file, 'w') as f:
            f.write(content.lstrip())

        return True
    except Exception as e:
        print(f'Erro ao criar arquivo de índice do Sphinx: {e}', file=sys.stderr)
        return False

def build_doxygen(language: str) -> bool:
    """
    Compila documentação com Doxygen.

    Args:
        language: Idioma da documentação.

    Returns:
        True se a documentação foi compilada com sucesso, False caso contrário.
    """
    try:
        # Define caminho do arquivo de configuração
        config_file = f'docs/api/{language}/Doxyfile'

        # Executa Doxygen
        result = subprocess.run(['doxygen', config_file],
                              capture_output=True, text=True)
        if result.returncode != 0:
            print('Erro ao executar Doxygen:', file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            return False

        return True
    except Exception as e:
        print(f'Erro ao compilar documentação com Doxygen: {e}', file=sys.stderr)
        return False

def build_sphinx(language: str) -> bool:
    """
    Compila documentação com Sphinx.

    Args:
        language: Idioma da documentação.

    Returns:
        True se a documentação foi compilada com sucesso, False caso contrário.
    """
    try:
        # Define diretórios
        source_dir = f'docs/user/{language}/source'
        build_dir = f'docs/user/{language}/build/html'

        # Executa Sphinx
        result = subprocess.run(['sphinx-build', '-b', 'html',
                               source_dir, build_dir],
                              capture_output=True, text=True)
        if result.returncode != 0:
            print('Erro ao executar Sphinx:', file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            return False

        return True
    except Exception as e:
        print(f'Erro ao compilar documentação com Sphinx: {e}', file=sys.stderr)
        return False

def clean_docs() -> bool:
    """
    Remove arquivos de documentação gerados.

    Returns:
        True se a limpeza foi bem sucedida, False caso contrário.
    """
    try:
        # Remove diretórios de build
        for language in DOCS_CONFIG['doxygen']['languages']:
            dirs = [
                f'docs/api/{language}',
                f'docs/user/{language}/build'
            ]
            for directory in dirs:
                if os.path.exists(directory):
                    shutil.rmtree(directory)

        return True
    except Exception as e:
        print(f'Erro ao limpar documentação: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_code_docs.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  config <idioma>       Cria arquivos de configuração', file=sys.stderr)
        print('  build <idioma>        Compila documentação', file=sys.stderr)
        print('  clean                 Remove arquivos gerados', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_directories() else 1

    elif command == 'config' and len(sys.argv) > 2:
        language = sys.argv[2]
        if language not in DOCS_CONFIG['doxygen']['languages']:
            print(f'Idioma inválido: {language}', file=sys.stderr)
            print('Idiomas válidos: ' +
                  ', '.join(DOCS_CONFIG['doxygen']['languages'].keys()),
                  file=sys.stderr)
            return 1

        if not create_doxygen_config(language):
            print('\nErro ao criar configuração do Doxygen!', file=sys.stderr)
            return 1

        if not create_sphinx_config(language):
            print('\nErro ao criar configuração do Sphinx!', file=sys.stderr)
            return 1

        if not create_sphinx_index(language):
            print('\nErro ao criar índice do Sphinx!', file=sys.stderr)
            return 1

        return 0

    elif command == 'build' and len(sys.argv) > 2:
        language = sys.argv[2]
        if language not in DOCS_CONFIG['doxygen']['languages']:
            print(f'Idioma inválido: {language}', file=sys.stderr)
            print('Idiomas válidos: ' +
                  ', '.join(DOCS_CONFIG['doxygen']['languages'].keys()),
                  file=sys.stderr)
            return 1

        if not build_doxygen(language):
            print('\nErro ao compilar documentação com Doxygen!', file=sys.stderr)
            return 1

        if not build_sphinx(language):
            print('\nErro ao compilar documentação com Sphinx!', file=sys.stderr)
            return 1

        return 0

    elif command == 'clean':
        return 0 if clean_docs() else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
