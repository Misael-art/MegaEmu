#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerar a documentação do projeto usando Doxygen e Sphinx.
"""

import os
import shutil
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

def generate_doxygen_docs() -> bool:
    """
    Gera a documentação usando Doxygen.

    Returns:
        True se a documentação foi gerada com sucesso, False caso contrário.
    """
    print('Gerando documentação com Doxygen...')

    # Verifica se o Doxyfile existe
    if not os.path.exists('Doxyfile'):
        print('Arquivo Doxyfile não encontrado!', file=sys.stderr)
        return False

    # Executa o Doxygen
    returncode, stdout, stderr = run_command(['doxygen', 'Doxyfile'])
    if returncode != 0:
        print('Erro ao gerar documentação com Doxygen:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    return True

def generate_sphinx_docs() -> bool:
    """
    Gera a documentação usando Sphinx.

    Returns:
        True se a documentação foi gerada com sucesso, False caso contrário.
    """
    print('\nGerando documentação com Sphinx...')

    # Verifica se o diretório docs existe
    if not os.path.exists('docs'):
        print('Diretório docs não encontrado!', file=sys.stderr)
        return False

    # Limpa o diretório de build
    build_dir = 'docs/_build'
    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)

    # Executa o Sphinx
    returncode, stdout, stderr = run_command(
        ['sphinx-build', '-b', 'html', 'docs', build_dir]
    )
    if returncode != 0:
        print('Erro ao gerar documentação com Sphinx:', file=sys.stderr)
        print(stderr, file=sys.stderr)
        return False

    return True

def copy_docs_to_pages() -> bool:
    """
    Copia a documentação para o diretório de páginas do GitHub.

    Returns:
        True se a documentação foi copiada com sucesso, False caso contrário.
    """
    print('\nCopiando documentação para GitHub Pages...')

    # Verifica se o diretório de build existe
    build_dir = 'docs/_build'
    if not os.path.exists(build_dir):
        print('Diretório de build não encontrado!', file=sys.stderr)
        return False

    # Cria o diretório de páginas se não existir
    pages_dir = 'docs/pages'
    if not os.path.exists(pages_dir):
        os.makedirs(pages_dir)

    try:
        # Copia os arquivos
        for item in os.listdir(build_dir):
            src = os.path.join(build_dir, item)
            dst = os.path.join(pages_dir, item)
            if os.path.isdir(src):
                if os.path.exists(dst):
                    shutil.rmtree(dst)
                shutil.copytree(src, dst)
            else:
                shutil.copy2(src, dst)
    except Exception as e:
        print(f'Erro ao copiar documentação: {e}', file=sys.stderr)
        return False

    return True

def update_readme() -> bool:
    """
    Atualiza o README.md com links para a documentação.

    Returns:
        True se o README foi atualizado com sucesso, False caso contrário.
    """
    print('\nAtualizando README.md...')

    try:
        with open('README.md', 'r', encoding='utf-8') as f:
            content = f.read()

        # Adiciona os links para a documentação
        docs_section = """
## Documentação

- [Documentação do Usuário](docs/pages/index.html)
- [Documentação da API](docs/html/index.html)
- [Guia de Contribuição](CONTRIBUTING.md)
- [Código de Conduta](CODE_OF_CONDUCT.md)
"""

        if '## Documentação' not in content:
            content += docs_section

        with open('README.md', 'w', encoding='utf-8') as f:
            f.write(content)

        return True
    except Exception as e:
        print(f'Erro ao atualizar README.md: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se a documentação foi gerada com sucesso, 1 caso contrário.
    """
    success = True

    # Gera a documentação com Doxygen
    success &= generate_doxygen_docs()

    # Gera a documentação com Sphinx
    success &= generate_sphinx_docs()

    # Copia a documentação para o diretório de páginas
    success &= copy_docs_to_pages()

    # Atualiza o README
    success &= update_readme()

    if not success:
        print('\nErro ao gerar documentação!', file=sys.stderr)
        return 1

    print('\nDocumentação gerada com sucesso!')
    return 0

if __name__ == '__main__':
    sys.exit(main())
