#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os mapeamentos de controle do emulador.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple

# Sistemas suportados e seus controles
SYSTEMS = {
    'mega-drive': {
        'name': 'Mega Drive',
        'buttons': {
            'up': 'D-Pad Up',
            'down': 'D-Pad Down',
            'left': 'D-Pad Left',
            'right': 'D-Pad Right',
            'a': 'A Button',
            'b': 'B Button',
            'c': 'C Button',
            'x': 'X Button',
            'y': 'Y Button',
            'z': 'Z Button',
            'start': 'Start Button',
            'mode': 'Mode Button'
        }
    },
    'master-system': {
        'name': 'Master System',
        'buttons': {
            'up': 'D-Pad Up',
            'down': 'D-Pad Down',
            'left': 'D-Pad Left',
            'right': 'D-Pad Right',
            '1': 'Button 1',
            '2': 'Button 2',
            'pause': 'Pause Button'
        }
    },
    'game-gear': {
        'name': 'Game Gear',
        'buttons': {
            'up': 'D-Pad Up',
            'down': 'D-Pad Down',
            'left': 'D-Pad Left',
            'right': 'D-Pad Right',
            '1': 'Button 1',
            '2': 'Button 2',
            'start': 'Start Button'
        }
    },
    'nes': {
        'name': 'NES',
        'buttons': {
            'up': 'D-Pad Up',
            'down': 'D-Pad Down',
            'left': 'D-Pad Left',
            'right': 'D-Pad Right',
            'a': 'A Button',
            'b': 'B Button',
            'select': 'Select Button',
            'start': 'Start Button'
        }
    }
}

# Tipos de entrada suportados
INPUT_TYPES = {
    'keyboard': {
        'name': 'Teclado',
        'keys': [
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
            'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'f1', 'f2', 'f3', 'f4', 'f5', 'f6', 'f7', 'f8', 'f9', 'f10', 'f11', 'f12',
            'up', 'down', 'left', 'right',
            'space', 'tab', 'enter', 'backspace', 'delete', 'escape',
            'left_shift', 'right_shift', 'left_ctrl', 'right_ctrl',
            'left_alt', 'right_alt'
        ]
    },
    'gamepad': {
        'name': 'Controle',
        'buttons': [
            'a', 'b', 'x', 'y',
            'left_bumper', 'right_bumper',
            'left_trigger', 'right_trigger',
            'select', 'start', 'guide',
            'left_stick', 'right_stick',
            'dpad_up', 'dpad_down', 'dpad_left', 'dpad_right'
        ],
        'axes': [
            'left_x', 'left_y',
            'right_x', 'right_y',
            'left_trigger', 'right_trigger'
        ]
    }
}

# Mapeamentos padrão
DEFAULT_MAPPINGS = {
    'mega-drive': {
        'keyboard': {
            'up': 'up',
            'down': 'down',
            'left': 'left',
            'right': 'right',
            'a': 'a',
            'b': 's',
            'c': 'd',
            'x': 'q',
            'y': 'w',
            'z': 'e',
            'start': 'enter',
            'mode': 'tab'
        },
        'gamepad': {
            'up': 'dpad_up',
            'down': 'dpad_down',
            'left': 'dpad_left',
            'right': 'dpad_right',
            'a': 'b',
            'b': 'a',
            'c': 'right_bumper',
            'x': 'y',
            'y': 'x',
            'z': 'left_bumper',
            'start': 'start',
            'mode': 'select'
        }
    },
    'master-system': {
        'keyboard': {
            'up': 'up',
            'down': 'down',
            'left': 'left',
            'right': 'right',
            '1': 'a',
            '2': 's',
            'pause': 'enter'
        },
        'gamepad': {
            'up': 'dpad_up',
            'down': 'dpad_down',
            'left': 'dpad_left',
            'right': 'dpad_right',
            '1': 'b',
            '2': 'a',
            'pause': 'start'
        }
    },
    'game-gear': {
        'keyboard': {
            'up': 'up',
            'down': 'down',
            'left': 'left',
            'right': 'right',
            '1': 'a',
            '2': 's',
            'start': 'enter'
        },
        'gamepad': {
            'up': 'dpad_up',
            'down': 'dpad_down',
            'left': 'dpad_left',
            'right': 'dpad_right',
            '1': 'b',
            '2': 'a',
            'start': 'start'
        }
    },
    'nes': {
        'keyboard': {
            'up': 'up',
            'down': 'down',
            'left': 'left',
            'right': 'right',
            'a': 'a',
            'b': 's',
            'select': 'tab',
            'start': 'enter'
        },
        'gamepad': {
            'up': 'dpad_up',
            'down': 'dpad_down',
            'left': 'dpad_left',
            'right': 'dpad_right',
            'a': 'b',
            'b': 'a',
            'select': 'select',
            'start': 'start'
        }
    }
}

def create_mapping_directories() -> bool:
    """
    Cria a estrutura de diretórios para mapeamentos.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        # Cria diretórios principais
        dirs = ['mappings']
        for system in SYSTEMS:
            dirs.append(f'mappings/{system}')

        for directory in dirs:
            os.makedirs(directory, exist_ok=True)

        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def validate_mapping(system: str, input_type: str, mapping: Dict) -> Tuple[bool, Optional[str]]:
    """
    Valida um mapeamento de controle.

    Args:
        system: O sistema alvo.
        input_type: O tipo de entrada.
        mapping: O dicionário com o mapeamento.

    Returns:
        Uma tupla (válido, mensagem) onde válido é um booleano e mensagem é None
        se válido ou uma mensagem de erro caso contrário.
    """
    # Verifica se o sistema é suportado
    if system not in SYSTEMS:
        return False, f'Sistema não suportado: {system}'

    # Verifica se o tipo de entrada é suportado
    if input_type not in INPUT_TYPES:
        return False, f'Tipo de entrada não suportado: {input_type}'

    # Verifica se todos os botões necessários estão mapeados
    required_buttons = SYSTEMS[system]['buttons'].keys()
    for button in required_buttons:
        if button not in mapping:
            return False, f'Botão obrigatório não mapeado: {button}'

    # Verifica se os valores mapeados são válidos
    valid_inputs = (INPUT_TYPES[input_type]['keys'] if input_type == 'keyboard'
                   else INPUT_TYPES[input_type]['buttons'] + INPUT_TYPES[input_type]['axes'])
    for button, value in mapping.items():
        if value not in valid_inputs:
            return False, f'Valor inválido para {button}: {value}'

    return True, None

def create_mapping(system: str, input_type: str, name: str,
                  mapping: Dict, description: Optional[str] = None) -> bool:
    """
    Cria um novo mapeamento de controle.

    Args:
        system: O sistema alvo.
        input_type: O tipo de entrada.
        name: Nome do mapeamento.
        mapping: O dicionário com o mapeamento.
        description: Descrição opcional do mapeamento.

    Returns:
        True se o mapeamento foi criado com sucesso, False caso contrário.
    """
    try:
        # Valida o mapeamento
        valid, message = validate_mapping(system, input_type, mapping)
        if not valid:
            print(f'Mapeamento inválido: {message}', file=sys.stderr)
            return False

        # Cria o arquivo de mapeamento
        mapping_data = {
            'name': name,
            'description': description or f'Mapeamento {input_type} para {system}',
            'system': system,
            'input_type': input_type,
            'mapping': mapping,
            'created': datetime.now().isoformat(),
            'modified': datetime.now().isoformat()
        }

        # Salva o mapeamento
        file_name = name.lower().replace(' ', '_')
        mapping_path = f'mappings/{system}/{file_name}.json'
        with open(mapping_path, 'w', encoding='utf-8') as f:
            json.dump(mapping_data, f, indent=2, ensure_ascii=False)

        print(f'Mapeamento criado: {name}')
        return True
    except Exception as e:
        print(f'Erro ao criar mapeamento: {e}', file=sys.stderr)
        return False

def install_default_mappings() -> bool:
    """
    Instala os mapeamentos padrão.

    Returns:
        True se os mapeamentos foram instalados com sucesso, False caso contrário.
    """
    try:
        for system, mappings in DEFAULT_MAPPINGS.items():
            for input_type, mapping in mappings.items():
                create_mapping(
                    system,
                    input_type,
                    f'Default {input_type.capitalize()}',
                    mapping,
                    f'Mapeamento padrão de {input_type} para {SYSTEMS[system]["name"]}'
                )
        return True
    except Exception as e:
        print(f'Erro ao instalar mapeamentos padrão: {e}', file=sys.stderr)
        return False

def list_mappings(system: Optional[str] = None) -> bool:
    """
    Lista os mapeamentos disponíveis.

    Args:
        system: Sistema opcional para filtrar os mapeamentos.

    Returns:
        True se a listagem foi bem sucedida, False caso contrário.
    """
    try:
        if system and system not in SYSTEMS:
            print(f'Sistema não suportado: {system}', file=sys.stderr)
            return False

        systems = [system] if system else SYSTEMS.keys()
        for sys in systems:
            mappings_dir = f'mappings/{sys}'
            if os.path.exists(mappings_dir):
                print(f'\nMapeamentos para {SYSTEMS[sys]["name"]}:')
                for mapping_file in sorted(os.listdir(mappings_dir)):
                    if mapping_file.endswith('.json'):
                        mapping_path = os.path.join(mappings_dir, mapping_file)
                        with open(mapping_path, 'r', encoding='utf-8') as f:
                            mapping = json.load(f)
                        print(f'\n{mapping["name"]}:')
                        print(f'  Descrição: {mapping["description"]}')
                        print(f'  Tipo: {mapping["input_type"]}')
                        print('  Mapeamento:')
                        for button, value in mapping['mapping'].items():
                            print(f'    {SYSTEMS[sys]["buttons"][button]}: {value}')
        return True
    except Exception as e:
        print(f'Erro ao listar mapeamentos: {e}', file=sys.stderr)
        return False

def remove_mapping(system: str, name: str) -> bool:
    """
    Remove um mapeamento.

    Args:
        system: O sistema do mapeamento.
        name: Nome do mapeamento.

    Returns:
        True se o mapeamento foi removido com sucesso, False caso contrário.
    """
    try:
        if system not in SYSTEMS:
            print(f'Sistema não suportado: {system}', file=sys.stderr)
            return False

        file_name = name.lower().replace(' ', '_')
        mapping_path = f'mappings/{system}/{file_name}.json'
        if os.path.exists(mapping_path):
            os.remove(mapping_path)
            print(f'Mapeamento removido: {name}')
            return True

        print(f'Mapeamento não encontrado: {name}', file=sys.stderr)
        return False
    except Exception as e:
        print(f'Erro ao remover mapeamento: {e}', file=sys.stderr)
        return False

def export_mapping(system: str, name: str, output_path: str) -> bool:
    """
    Exporta um mapeamento.

    Args:
        system: O sistema do mapeamento.
        name: Nome do mapeamento.
        output_path: Caminho de saída.

    Returns:
        True se o mapeamento foi exportado com sucesso, False caso contrário.
    """
    try:
        if system not in SYSTEMS:
            print(f'Sistema não suportado: {system}', file=sys.stderr)
            return False

        file_name = name.lower().replace(' ', '_')
        mapping_path = f'mappings/{system}/{file_name}.json'
        if not os.path.exists(mapping_path):
            print(f'Mapeamento não encontrado: {name}', file=sys.stderr)
            return False

        # Cria diretório de saída se necessário
        os.makedirs(os.path.dirname(output_path), exist_ok=True)

        # Copia o mapeamento
        with open(mapping_path, 'r', encoding='utf-8') as f:
            mapping = json.load(f)

        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(mapping, f, indent=2, ensure_ascii=False)

        print(f'Mapeamento exportado para: {output_path}')
        return True
    except Exception as e:
        print(f'Erro ao exportar mapeamento: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_controls.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  create <sistema> <tipo> <nome> <mapeamento> [descrição]')
        print('                        Cria um novo mapeamento', file=sys.stderr)
        print('  install-defaults      Instala mapeamentos padrão', file=sys.stderr)
        print('  list [sistema]        Lista mapeamentos disponíveis', file=sys.stderr)
        print('  remove <sistema> <nome>')
        print('                        Remove um mapeamento', file=sys.stderr)
        print('  export <sistema> <nome> <arquivo>')
        print('                        Exporta um mapeamento', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_mapping_directories() else 1

    elif command == 'create' and len(sys.argv) >= 5:
        try:
            system = sys.argv[2]
            input_type = sys.argv[3]
            name = sys.argv[4]
            mapping = json.loads(sys.argv[5])
            description = sys.argv[6] if len(sys.argv) > 6 else None
            return 0 if create_mapping(system, input_type, name, mapping, description) else 1
        except json.JSONDecodeError:
            print('Formato inválido para mapeamento. Use JSON.', file=sys.stderr)
            return 1

    elif command == 'install-defaults':
        return 0 if install_default_mappings() else 1

    elif command == 'list':
        system = sys.argv[2] if len(sys.argv) > 2 else None
        return 0 if list_mappings(system) else 1

    elif command == 'remove' and len(sys.argv) > 3:
        return 0 if remove_mapping(sys.argv[2], sys.argv[3]) else 1

    elif command == 'export' and len(sys.argv) > 4:
        return 0 if export_mapping(sys.argv[2], sys.argv[3], sys.argv[4]) else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
