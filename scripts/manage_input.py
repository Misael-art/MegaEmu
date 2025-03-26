#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de entrada do emulador.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import platform

# Configurações de entrada
INPUT_CONFIG = {
    'directories': {
        'config': 'config/input',
        'profiles': 'config/input/profiles',
        'macros': 'config/input/macros'
    },
    'controllers': {
        'genesis_3': {
            'name': 'Mega Drive 3-Button',
            'buttons': [
                {'name': 'Up', 'type': 'digital'},
                {'name': 'Down', 'type': 'digital'},
                {'name': 'Left', 'type': 'digital'},
                {'name': 'Right', 'type': 'digital'},
                {'name': 'A', 'type': 'digital'},
                {'name': 'B', 'type': 'digital'},
                {'name': 'C', 'type': 'digital'},
                {'name': 'Start', 'type': 'digital'}
            ]
        },
        'genesis_6': {
            'name': 'Mega Drive 6-Button',
            'buttons': [
                {'name': 'Up', 'type': 'digital'},
                {'name': 'Down', 'type': 'digital'},
                {'name': 'Left', 'type': 'digital'},
                {'name': 'Right', 'type': 'digital'},
                {'name': 'A', 'type': 'digital'},
                {'name': 'B', 'type': 'digital'},
                {'name': 'C', 'type': 'digital'},
                {'name': 'X', 'type': 'digital'},
                {'name': 'Y', 'type': 'digital'},
                {'name': 'Z', 'type': 'digital'},
                {'name': 'Start', 'type': 'digital'},
                {'name': 'Mode', 'type': 'digital'}
            ]
        }
    },
    'keyboard_default': {
        'Up': 'Up',
        'Down': 'Down',
        'Left': 'Left',
        'Right': 'Right',
        'A': 'A',
        'B': 'S',
        'C': 'D',
        'X': 'Q',
        'Y': 'W',
        'Z': 'E',
        'Start': 'Enter',
        'Mode': 'Tab'
    },
    'gamepad_default': {
        'Up': 'DPadUp',
        'Down': 'DPadDown',
        'Left': 'DPadLeft',
        'Right': 'DPadRight',
        'A': 'ButtonA',
        'B': 'ButtonB',
        'C': 'ButtonX',
        'X': 'ButtonY',
        'Y': 'ButtonLeftShoulder',
        'Z': 'ButtonRightShoulder',
        'Start': 'ButtonStart',
        'Mode': 'ButtonSelect'
    },
    'profiles': {
        'default': {
            'name': 'Default Profile',
            'description': 'Configuração padrão para teclado e gamepad',
            'keyboard': 'keyboard_default',
            'gamepad': 'gamepad_default'
        },
        'arcade': {
            'name': 'Arcade Profile',
            'description': 'Configuração para controles de arcade',
            'keyboard': {
                'Up': '5',
                'Down': '2',
                'Left': '1',
                'Right': '3',
                'A': 'L',
                'B': 'K',
                'C': 'J',
                'X': 'I',
                'Y': 'U',
                'Z': 'Y',
                'Start': '1',
                'Mode': '5'
            },
            'gamepad': 'gamepad_default'
        }
    },
    'macros': {
        'hadouken': {
            'name': 'Hadouken',
            'description': 'Down, Down-Forward, Forward + Punch',
            'sequence': [
                {'input': 'Down', 'duration': 0.1},
                {'input': ['Down', 'Right'], 'duration': 0.1},
                {'input': 'Right', 'duration': 0.1},
                {'input': 'A', 'duration': 0.1}
            ]
        },
        'shoryuken': {
            'name': 'Shoryuken',
            'description': 'Forward, Down, Down-Forward + Punch',
            'sequence': [
                {'input': 'Right', 'duration': 0.1},
                {'input': 'Down', 'duration': 0.1},
                {'input': ['Down', 'Right'], 'duration': 0.1},
                {'input': 'A', 'duration': 0.1}
            ]
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
        for directory in INPUT_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def generate_config_files() -> bool:
    """
    Gera arquivos de configuração.

    Returns:
        True se os arquivos foram gerados com sucesso, False caso contrário.
    """
    try:
        config_dir = INPUT_CONFIG['directories']['config']

        # Salva configuração de controles
        config_file = f"{config_dir}/controllers.json"
        with open(config_file, 'w') as f:
            json.dump(INPUT_CONFIG['controllers'], f, indent=4)

        # Salva configurações padrão
        defaults_file = f"{config_dir}/defaults.json"
        defaults = {
            'keyboard': INPUT_CONFIG['keyboard_default'],
            'gamepad': INPUT_CONFIG['gamepad_default']
        }
        with open(defaults_file, 'w') as f:
            json.dump(defaults, f, indent=4)

        return True
    except Exception as e:
        print(f'Erro ao gerar arquivos de configuração: {e}', file=sys.stderr)
        return False

def generate_profile_files() -> bool:
    """
    Gera arquivos de perfil.

    Returns:
        True se os arquivos foram gerados com sucesso, False caso contrário.
    """
    try:
        profiles_dir = INPUT_CONFIG['directories']['profiles']

        for profile_name, profile_data in INPUT_CONFIG['profiles'].items():
            profile_file = f"{profiles_dir}/{profile_name}.json"
            with open(profile_file, 'w') as f:
                json.dump(profile_data, f, indent=4)

        return True
    except Exception as e:
        print(f'Erro ao gerar arquivos de perfil: {e}', file=sys.stderr)
        return False

def generate_macro_files() -> bool:
    """
    Gera arquivos de macro.

    Returns:
        True se os arquivos foram gerados com sucesso, False caso contrário.
    """
    try:
        macros_dir = INPUT_CONFIG['directories']['macros']

        for macro_name, macro_data in INPUT_CONFIG['macros'].items():
            macro_file = f"{macros_dir}/{macro_name}.json"
            with open(macro_file, 'w') as f:
                json.dump(macro_data, f, indent=4)

        return True
    except Exception as e:
        print(f'Erro ao gerar arquivos de macro: {e}', file=sys.stderr)
        return False

def generate_cpp_header() -> bool:
    """
    Gera arquivo de cabeçalho C++.

    Returns:
        True se o arquivo foi gerado com sucesso, False caso contrário.
    """
    try:
        header = f"""
#pragma once

#include <string>
#include <vector>
#include <map>

namespace input {{

// Tipos de controle
enum class ControllerType {{
    Genesis3Button,
    Genesis6Button
}};

// Botões
enum class Button {{
    Up,
    Down,
    Left,
    Right,
    A,
    B,
    C,
    X,
    Y,
    Z,
    Start,
    Mode
}};

// Estado dos botões
struct ButtonState {{
    bool up;
    bool down;
    bool left;
    bool right;
    bool a;
    bool b;
    bool c;
    bool x;
    bool y;
    bool z;
    bool start;
    bool mode;
}};

// Configuração de controle
struct ControllerConfig {{
    ControllerType type;
    std::map<Button, int> keyboardMap;
    std::map<Button, int> gamepadMap;
}};

// Macro de entrada
struct MacroInput {{
    std::vector<Button> buttons;
    float duration;
}};

struct Macro {{
    std::string name;
    std::string description;
    std::vector<MacroInput> sequence;
}};

}} // namespace input
""".strip()

        # Salva arquivo
        header_file = 'include/input/input.hpp'
        os.makedirs(os.path.dirname(header_file), exist_ok=True)
        with open(header_file, 'w') as f:
            f.write(header)

        return True
    except Exception as e:
        print(f'Erro ao gerar arquivo de cabeçalho: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_input.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  config               Gera arquivos de configuração', file=sys.stderr)
        print('  profiles             Gera arquivos de perfil', file=sys.stderr)
        print('  macros               Gera arquivos de macro', file=sys.stderr)
        print('  header               Gera arquivo de cabeçalho C++', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_directories() else 1

    elif command == 'config':
        return 0 if generate_config_files() else 1

    elif command == 'profiles':
        return 0 if generate_profile_files() else 1

    elif command == 'macros':
        return 0 if generate_macro_files() else 1

    elif command == 'header':
        return 0 if generate_cpp_header() else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
