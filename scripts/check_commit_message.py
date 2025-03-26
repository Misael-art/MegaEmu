#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para verificar se a mensagem de commit segue o padrão do Conventional Commits.
"""

import re
import sys
from typing import List, Optional, Pattern, Tuple

# Tipos de commit válidos
VALID_TYPES = {
    'feat': 'Nova funcionalidade',
    'fix': 'Correção de bug',
    'docs': 'Alterações na documentação',
    'style': 'Formatação, ponto e vírgula faltando, etc',
    'refactor': 'Refatoração do código de produção',
    'test': 'Adição ou modificação de testes',
    'chore': 'Atualização de tarefas de build',
    'perf': 'Melhorias de performance',
    'ci': 'Alterações nos arquivos de CI',
    'build': 'Alterações que afetam o sistema de build',
    'revert': 'Reverte um commit anterior'
}

# Escopos válidos
VALID_SCOPES = [
    'core',
    'cpu',
    'ppu',
    'apu',
    'frontend',
    'tests',
    'docs',
    'build',
    'ci',
    'deps'
]

# Regex para validar a mensagem de commit
COMMIT_PATTERN: Pattern = re.compile(
    r'^(?P<type>feat|fix|docs|style|refactor|test|chore|perf|ci|build|revert)'
    r'(?:\((?P<scope>[^\)]+)\))?'
    r':\s(?P<description>[^\n]+)'
    r'(?:\n\n(?P<body>(?:.*\n)*)?)?'
    r'(?:\n\n(?P<footer>(?:.*\n)*))?$'
)

def validate_commit_message(message: str) -> Tuple[bool, Optional[str]]:
    """
    Valida a mensagem de commit.

    Args:
        message: A mensagem de commit a ser validada.

    Returns:
        Uma tupla contendo um booleano indicando se a mensagem é válida e uma
        mensagem de erro opcional.
    """
    # Remove comentários e linhas em branco
    lines: List[str] = [
        line for line in message.splitlines()
        if not line.startswith('#') and line.strip()
    ]

    if not lines:
        return False, 'Mensagem de commit vazia'

    # Junta as linhas novamente
    message = '\n'.join(lines)

    # Verifica se a mensagem segue o padrão
    match = COMMIT_PATTERN.match(message)
    if not match:
        return False, (
            'Mensagem de commit inválida. Deve seguir o padrão:\n'
            '<tipo>(<escopo>): <descrição>\n\n'
            '<corpo>\n\n'
            '<rodapé>'
        )

    # Extrai os componentes da mensagem
    commit_type = match.group('type')
    scope = match.group('scope')
    description = match.group('description')

    # Verifica o tipo
    if commit_type not in VALID_TYPES:
        return False, f'Tipo de commit inválido: {commit_type}'

    # Verifica o escopo se presente
    if scope and scope not in VALID_SCOPES:
        return False, f'Escopo inválido: {scope}'

    # Verifica o tamanho da descrição
    if len(description) > 50:
        return False, 'Descrição muito longa (máximo 50 caracteres)'

    # Verifica se a descrição começa com letra minúscula
    if not description[0].islower():
        return False, 'Descrição deve começar com letra minúscula'

    # Verifica se a descrição termina com ponto
    if description.endswith('.'):
        return False, 'Descrição não deve terminar com ponto'

    return True, None

def main() -> int:
    """
    Função principal.

    Returns:
        0 se a mensagem de commit for válida, 1 caso contrário.
    """
    # Lê a mensagem de commit do arquivo
    commit_msg_file = sys.argv[1]
    with open(commit_msg_file, 'r', encoding='utf-8') as f:
        commit_msg = f.read()

    # Valida a mensagem
    is_valid, error_msg = validate_commit_message(commit_msg)
    if not is_valid:
        print(f'Erro: {error_msg}', file=sys.stderr)
        print('\nTipos válidos:', file=sys.stderr)
        for t, desc in VALID_TYPES.items():
            print(f'  {t:8} - {desc}', file=sys.stderr)
        print('\nEscopos válidos:', file=sys.stderr)
        print('  ' + ', '.join(VALID_SCOPES), file=sys.stderr)
        return 1

    return 0

if __name__ == '__main__':
    sys.exit(main())
