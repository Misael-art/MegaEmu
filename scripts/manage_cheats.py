#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os cheats e códigos de trapaça.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

# Formatos de códigos suportados por sistema
CHEAT_FORMATS = {
    'mega-drive': {
        'game-genie': {
            'format': r'^[A-Z0-9]{6}(-[A-Z0-9]{4})?$',
            'description': 'Código Game Genie (XXXX-XX ou XXXX-XXXX)'
        },
        'action-replay': {
            'format': r'^[A-F0-9]{8}:[A-F0-9]{4}$',
            'description': 'Código Action Replay (XXXXXXXX:XXXX)'
        }
    },
    'master-system': {
        'game-genie': {
            'format': r'^[A-Z0-9]{6}(-[A-Z0-9]{3})?$',
            'description': 'Código Game Genie (XXXXXX ou XXXXXX-XXX)'
        }
    },
    'game-gear': {
        'game-genie': {
            'format': r'^[A-Z0-9]{6}(-[A-Z0-9]{3})?$',
            'description': 'Código Game Genie (XXXXXX ou XXXXXX-XXX)'
        }
    },
    'nes': {
        'game-genie': {
            'format': r'^[A-Z0-9]{6}(-[A-Z0-9]{2})?$',
            'description': 'Código Game Genie (XXXXXX ou XXXXXX-XX)'
        }
    }
}

# Categorias de cheats
CHEAT_CATEGORIES = [
    'vidas-infinitas',
    'energia-infinita',
    'invencibilidade',
    'municao-infinita',
    'itens',
    'personagens',
    'fases',
    'debug',
    'outros'
]

def create_cheat_directories() -> bool:
    """
    Cria a estrutura de diretórios para cheats.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        # Cria diretórios principais
        dirs = ['cheats']
        for system in CHEAT_FORMATS:
            dirs.append(f'cheats/{system}')

        for directory in dirs:
            os.makedirs(directory, exist_ok=True)

        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def validate_cheat_code(system: str, cheat_type: str, code: str) -> Tuple[bool, Optional[str]]:
    """
    Valida um código de trapaça.

    Args:
        system: Sistema do jogo.
        cheat_type: Tipo do código (game-genie, action-replay).
        code: Código a ser validado.

    Returns:
        Uma tupla (válido, mensagem) onde válido é um booleano e mensagem é None
        se válido ou uma mensagem de erro caso contrário.
    """
    import re

    try:
        # Verifica se o sistema é suportado
        if system not in CHEAT_FORMATS:
            return False, f'Sistema não suportado: {system}'

        # Verifica se o tipo de código é suportado
        if cheat_type not in CHEAT_FORMATS[system]:
            return False, f'Tipo de código não suportado: {cheat_type}'

        # Valida o formato do código
        pattern = CHEAT_FORMATS[system][cheat_type]['format']
        if not re.match(pattern, code):
            return False, (f'Formato inválido. Use: '
                         f'{CHEAT_FORMATS[system][cheat_type]["description"]}')

        return True, None
    except Exception as e:
        return False, f'Erro ao validar código: {e}'

def create_cheat(system: str, rom_name: str, cheat_type: str, code: str,
                description: str, category: str, enabled: bool = False) -> bool:
    """
    Cria um novo cheat.

    Args:
        system: Sistema do jogo.
        rom_name: Nome da ROM.
        cheat_type: Tipo do código.
        code: Código de trapaça.
        description: Descrição do cheat.
        category: Categoria do cheat.
        enabled: Se o cheat deve estar ativado por padrão.

    Returns:
        True se o cheat foi criado com sucesso, False caso contrário.
    """
    try:
        # Valida o código
        valid, message = validate_cheat_code(system, cheat_type, code)
        if not valid:
            print(f'Código inválido: {message}', file=sys.stderr)
            return False

        # Valida a categoria
        if category not in CHEAT_CATEGORIES:
            print(f'Categoria inválida: {category}', file=sys.stderr)
            return False

        # Carrega cheats existentes
        cheats = load_cheats(system, rom_name)

        # Verifica se o código já existe
        for cheat in cheats:
            if cheat['code'] == code:
                print('Código já existe.', file=sys.stderr)
                return False

        # Adiciona o novo cheat
        cheats.append({
            'type': cheat_type,
            'code': code,
            'description': description,
            'category': category,
            'enabled': enabled,
            'created': datetime.now().isoformat(),
            'modified': datetime.now().isoformat()
        })

        # Salva os cheats
        return save_cheats(system, rom_name, cheats)
    except Exception as e:
        print(f'Erro ao criar cheat: {e}', file=sys.stderr)
        return False

def load_cheats(system: str, rom_name: str) -> List[Dict]:
    """
    Carrega os cheats de uma ROM.

    Args:
        system: Sistema do jogo.
        rom_name: Nome da ROM.

    Returns:
        Lista de cheats.
    """
    try:
        cheats_path = f'cheats/{system}/{rom_name}.json'
        if os.path.exists(cheats_path):
            with open(cheats_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        return []
    except Exception as e:
        print(f'Erro ao carregar cheats: {e}', file=sys.stderr)
        return []

def save_cheats(system: str, rom_name: str, cheats: List[Dict]) -> bool:
    """
    Salva os cheats de uma ROM.

    Args:
        system: Sistema do jogo.
        rom_name: Nome da ROM.
        cheats: Lista de cheats.

    Returns:
        True se os cheats foram salvos com sucesso, False caso contrário.
    """
    try:
        cheats_path = f'cheats/{system}/{rom_name}.json'
        with open(cheats_path, 'w', encoding='utf-8') as f:
            json.dump(cheats, f, indent=2, ensure_ascii=False)
        return True
    except Exception as e:
        print(f'Erro ao salvar cheats: {e}', file=sys.stderr)
        return False

def list_cheats(system: Optional[str] = None, rom_name: Optional[str] = None,
                category: Optional[str] = None) -> List[Dict]:
    """
    Lista cheats.

    Args:
        system: Sistema para filtrar (opcional).
        rom_name: ROM para filtrar (opcional).
        category: Categoria para filtrar (opcional).

    Returns:
        Lista de cheats.
    """
    all_cheats = []
    try:
        # Define sistemas a serem pesquisados
        systems = [system] if system else CHEAT_FORMATS.keys()

        for sys in systems:
            if not os.path.exists(f'cheats/{sys}'):
                continue

            # Lista arquivos de cheats
            for file_name in os.listdir(f'cheats/{sys}'):
                if not file_name.endswith('.json'):
                    continue

                current_rom = file_name[:-5]
                if rom_name and current_rom != rom_name:
                    continue

                # Carrega cheats da ROM
                cheats = load_cheats(sys, current_rom)
                for cheat in cheats:
                    if category and cheat['category'] != category:
                        continue

                    # Adiciona informações do sistema e ROM
                    cheat_info = cheat.copy()
                    cheat_info['system'] = sys
                    cheat_info['rom_name'] = current_rom
                    all_cheats.append(cheat_info)

        return all_cheats
    except Exception as e:
        print(f'Erro ao listar cheats: {e}', file=sys.stderr)
        return []

def update_cheat(system: str, rom_name: str, code: str,
                description: Optional[str] = None,
                category: Optional[str] = None,
                enabled: Optional[bool] = None) -> bool:
    """
    Atualiza um cheat existente.

    Args:
        system: Sistema do jogo.
        rom_name: Nome da ROM.
        code: Código do cheat.
        description: Nova descrição (opcional).
        category: Nova categoria (opcional).
        enabled: Novo estado (opcional).

    Returns:
        True se o cheat foi atualizado com sucesso, False caso contrário.
    """
    try:
        # Carrega cheats existentes
        cheats = load_cheats(system, rom_name)

        # Procura o cheat
        for cheat in cheats:
            if cheat['code'] == code:
                # Atualiza campos
                if description is not None:
                    cheat['description'] = description
                if category is not None:
                    if category not in CHEAT_CATEGORIES:
                        print(f'Categoria inválida: {category}', file=sys.stderr)
                        return False
                    cheat['category'] = category
                if enabled is not None:
                    cheat['enabled'] = enabled
                cheat['modified'] = datetime.now().isoformat()

                # Salva alterações
                return save_cheats(system, rom_name, cheats)

        print('Cheat não encontrado.', file=sys.stderr)
        return False
    except Exception as e:
        print(f'Erro ao atualizar cheat: {e}', file=sys.stderr)
        return False

def delete_cheat(system: str, rom_name: str, code: str) -> bool:
    """
    Remove um cheat.

    Args:
        system: Sistema do jogo.
        rom_name: Nome da ROM.
        code: Código do cheat.

    Returns:
        True se o cheat foi removido com sucesso, False caso contrário.
    """
    try:
        # Carrega cheats existentes
        cheats = load_cheats(system, rom_name)

        # Remove o cheat
        original_length = len(cheats)
        cheats = [c for c in cheats if c['code'] != code]
        if len(cheats) == original_length:
            print('Cheat não encontrado.', file=sys.stderr)
            return False

        # Salva alterações
        return save_cheats(system, rom_name, cheats)
    except Exception as e:
        print(f'Erro ao remover cheat: {e}', file=sys.stderr)
        return False

def export_cheats(system: str, rom_name: str, output_path: str) -> bool:
    """
    Exporta os cheats de uma ROM.

    Args:
        system: Sistema do jogo.
        rom_name: Nome da ROM.
        output_path: Caminho do arquivo de saída.

    Returns:
        True se os cheats foram exportados com sucesso, False caso contrário.
    """
    try:
        # Carrega cheats
        cheats = load_cheats(system, rom_name)
        if not cheats:
            print('Nenhum cheat encontrado.', file=sys.stderr)
            return False

        # Cria diretório de saída se necessário
        os.makedirs(os.path.dirname(output_path), exist_ok=True)

        # Salva cheats
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(cheats, f, indent=2, ensure_ascii=False)

        print(f'Cheats exportados para: {output_path}')
        return True
    except Exception as e:
        print(f'Erro ao exportar cheats: {e}', file=sys.stderr)
        return False

def import_cheats(system: str, rom_name: str, input_path: str) -> bool:
    """
    Importa cheats para uma ROM.

    Args:
        system: Sistema do jogo.
        rom_name: Nome da ROM.
        input_path: Caminho do arquivo de entrada.

    Returns:
        True se os cheats foram importados com sucesso, False caso contrário.
    """
    try:
        # Verifica se o arquivo existe
        if not os.path.exists(input_path):
            print(f'Arquivo não encontrado: {input_path}', file=sys.stderr)
            return False

        # Carrega cheats do arquivo
        with open(input_path, 'r', encoding='utf-8') as f:
            cheats = json.load(f)

        # Valida os cheats
        for cheat in cheats:
            if 'type' not in cheat or 'code' not in cheat:
                print('Formato de cheat inválido.', file=sys.stderr)
                return False

            valid, message = validate_cheat_code(system, cheat['type'], cheat['code'])
            if not valid:
                print(f'Código inválido: {message}', file=sys.stderr)
                return False

            if 'category' in cheat and cheat['category'] not in CHEAT_CATEGORIES:
                print(f'Categoria inválida: {cheat["category"]}', file=sys.stderr)
                return False

        # Salva os cheats
        return save_cheats(system, rom_name, cheats)
    except Exception as e:
        print(f'Erro ao importar cheats: {e}', file=sys.stderr)
        return False

def print_cheat_info(cheat: Dict) -> None:
    """
    Imprime informações de um cheat.

    Args:
        cheat: Dicionário com informações do cheat.
    """
    print(f'\nCheat para {cheat["rom_name"]} ({cheat["system"]}):')
    print(f'  Código: {cheat["code"]} ({cheat["type"]})')
    print(f'  Descrição: {cheat["description"]}')
    print(f'  Categoria: {cheat["category"]}')
    print(f'  Ativo: {"Sim" if cheat["enabled"] else "Não"}')
    print(f'  Criado: {cheat["created"]}')
    print(f'  Modificado: {cheat["modified"]}')

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_cheats.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  create <sistema> <rom> <tipo> <código> <descrição> <categoria>')
        print('                        Cria um novo cheat', file=sys.stderr)
        print('  list [sistema] [rom] [categoria]')
        print('                        Lista cheats', file=sys.stderr)
        print('  update <sistema> <rom> <código> [descrição] [categoria] [ativo]')
        print('                        Atualiza um cheat', file=sys.stderr)
        print('  delete <sistema> <rom> <código>')
        print('                        Remove um cheat', file=sys.stderr)
        print('  export <sistema> <rom> <arquivo>')
        print('                        Exporta cheats', file=sys.stderr)
        print('  import <sistema> <rom> <arquivo>')
        print('                        Importa cheats', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_cheat_directories() else 1

    elif command == 'create' and len(sys.argv) > 7:
        return 0 if create_cheat(
            sys.argv[2],  # sistema
            sys.argv[3],  # rom
            sys.argv[4],  # tipo
            sys.argv[5],  # código
            sys.argv[6],  # descrição
            sys.argv[7],  # categoria
            True if len(sys.argv) > 8 and sys.argv[8].lower() == 'true' else False
        ) else 1

    elif command == 'list':
        system = sys.argv[2] if len(sys.argv) > 2 else None
        rom_name = sys.argv[3] if len(sys.argv) > 3 else None
        category = sys.argv[4] if len(sys.argv) > 4 else None
        cheats = list_cheats(system, rom_name, category)
        for cheat in cheats:
            print_cheat_info(cheat)
        return 0

    elif command == 'update' and len(sys.argv) > 4:
        description = sys.argv[5] if len(sys.argv) > 5 else None
        category = sys.argv[6] if len(sys.argv) > 6 else None
        enabled = (None if len(sys.argv) <= 7 else
                  True if sys.argv[7].lower() == 'true' else
                  False if sys.argv[7].lower() == 'false' else None)
        return 0 if update_cheat(
            sys.argv[2],  # sistema
            sys.argv[3],  # rom
            sys.argv[4],  # código
            description,
            category,
            enabled
        ) else 1

    elif command == 'delete' and len(sys.argv) > 4:
        return 0 if delete_cheat(sys.argv[2], sys.argv[3], sys.argv[4]) else 1

    elif command == 'export' and len(sys.argv) > 4:
        return 0 if export_cheats(sys.argv[2], sys.argv[3], sys.argv[4]) else 1

    elif command == 'import' and len(sys.argv) > 4:
        return 0 if import_cheats(sys.argv[2], sys.argv[3], sys.argv[4]) else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
