#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os shaders do emulador.
"""

import json
import os
import shutil
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple

# Tipos de shaders suportados
SHADER_TYPES = {
    'vertex': ['.vert', '.vs'],
    'fragment': ['.frag', '.fs'],
    'compute': ['.comp', '.cs'],
    'geometry': ['.geom', '.gs']
}

# Presets de shaders
DEFAULT_PRESETS = {
    'crt': {
        'name': 'CRT',
        'description': 'Simula uma tela CRT clássica',
        'shaders': ['crt.vert', 'crt.frag'],
        'parameters': {
            'curvature': 0.1,
            'scanline_strength': 0.5,
            'bloom': 0.2,
            'mask_strength': 0.3
        }
    },
    'lcd': {
        'name': 'LCD',
        'description': 'Simula uma tela LCD',
        'shaders': ['lcd.vert', 'lcd.frag'],
        'parameters': {
            'grid_strength': 0.2,
            'subpixel_layout': 'rgb',
            'response_time': 0.016
        }
    },
    'scanlines': {
        'name': 'Scanlines',
        'description': 'Adiciona scanlines simples',
        'shaders': ['basic.vert', 'scanlines.frag'],
        'parameters': {
            'line_strength': 0.3,
            'line_width': 1.0
        }
    },
    'hqx': {
        'name': 'HQx',
        'description': 'Filtro de escala HQx',
        'shaders': ['hqx.vert', 'hqx.frag'],
        'parameters': {
            'scale': 2
        }
    },
    'xbrz': {
        'name': 'xBRZ',
        'description': 'Filtro de escala xBRZ',
        'shaders': ['xbrz.vert', 'xbrz.frag'],
        'parameters': {
            'scale': 3
        }
    }
}

def create_shader_directories() -> bool:
    """
    Cria a estrutura de diretórios para shaders.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        # Cria diretórios principais
        dirs = ['shaders', 'shaders/presets', 'shaders/cache']
        for shader_type in SHADER_TYPES:
            dirs.append(f'shaders/{shader_type}')

        for directory in dirs:
            os.makedirs(directory, exist_ok=True)

        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def validate_shader(shader_path: str) -> Tuple[bool, Optional[str]]:
    """
    Valida um arquivo de shader.

    Args:
        shader_path: O caminho do arquivo de shader.

    Returns:
        Uma tupla (válido, tipo) onde válido é um booleano e tipo é o tipo do shader
        ou None se inválido.
    """
    # Verifica se o arquivo existe
    if not os.path.exists(shader_path):
        return False, None

    # Verifica a extensão
    ext = os.path.splitext(shader_path)[1].lower()
    for shader_type, extensions in SHADER_TYPES.items():
        if ext in extensions:
            return True, shader_type

    return False, None

def install_shader(shader_path: str) -> bool:
    """
    Instala um shader no diretório apropriado.

    Args:
        shader_path: O caminho do arquivo de shader.

    Returns:
        True se o shader foi instalado com sucesso, False caso contrário.
    """
    try:
        # Valida o shader
        valid, shader_type = validate_shader(shader_path)
        if not valid:
            print(f'Shader inválido: {shader_path}', file=sys.stderr)
            return False

        # Cria o diretório de destino se necessário
        dest_dir = f'shaders/{shader_type}'
        os.makedirs(dest_dir, exist_ok=True)

        # Copia o shader
        shader_name = os.path.basename(shader_path)
        dest_path = os.path.join(dest_dir, shader_name)
        shutil.copy2(shader_path, dest_path)

        print(f'Shader instalado: {shader_name} ({shader_type})')
        return True
    except Exception as e:
        print(f'Erro ao instalar shader: {e}', file=sys.stderr)
        return False

def create_preset(name: str, description: str, shaders: List[str],
                 parameters: Dict) -> bool:
    """
    Cria um preset de shader.

    Args:
        name: Nome do preset.
        description: Descrição do preset.
        shaders: Lista de shaders utilizados.
        parameters: Parâmetros do preset.

    Returns:
        True se o preset foi criado com sucesso, False caso contrário.
    """
    try:
        # Verifica se todos os shaders existem
        for shader in shaders:
            shader_path = None
            for shader_type in SHADER_TYPES:
                test_path = f'shaders/{shader_type}/{shader}'
                if os.path.exists(test_path):
                    shader_path = test_path
                    break
            if not shader_path:
                print(f'Shader não encontrado: {shader}', file=sys.stderr)
                return False

        # Cria o preset
        preset = {
            'name': name,
            'description': description,
            'shaders': shaders,
            'parameters': parameters,
            'created': datetime.now().isoformat()
        }

        # Salva o preset
        preset_path = f'shaders/presets/{name.lower()}.json'
        with open(preset_path, 'w', encoding='utf-8') as f:
            json.dump(preset, f, indent=2, ensure_ascii=False)

        print(f'Preset criado: {name}')
        return True
    except Exception as e:
        print(f'Erro ao criar preset: {e}', file=sys.stderr)
        return False

def install_default_presets() -> bool:
    """
    Instala os presets padrão.

    Returns:
        True se os presets foram instalados com sucesso, False caso contrário.
    """
    try:
        for preset_id, preset_data in DEFAULT_PRESETS.items():
            create_preset(
                preset_data['name'],
                preset_data['description'],
                preset_data['shaders'],
                preset_data['parameters']
            )
        return True
    except Exception as e:
        print(f'Erro ao instalar presets padrão: {e}', file=sys.stderr)
        return False

def list_shaders() -> bool:
    """
    Lista todos os shaders instalados.

    Returns:
        True se a listagem foi bem sucedida, False caso contrário.
    """
    try:
        print('\nShaders instalados:')
        for shader_type in SHADER_TYPES:
            shader_dir = f'shaders/{shader_type}'
            if os.path.exists(shader_dir):
                shaders = [f for f in os.listdir(shader_dir)
                          if os.path.isfile(os.path.join(shader_dir, f))]
                if shaders:
                    print(f'\n{shader_type.capitalize()}:')
                    for shader in sorted(shaders):
                        print(f'  - {shader}')
        return True
    except Exception as e:
        print(f'Erro ao listar shaders: {e}', file=sys.stderr)
        return False

def list_presets() -> bool:
    """
    Lista todos os presets disponíveis.

    Returns:
        True se a listagem foi bem sucedida, False caso contrário.
    """
    try:
        presets_dir = 'shaders/presets'
        if not os.path.exists(presets_dir):
            print('Nenhum preset encontrado.')
            return True

        print('\nPresets disponíveis:')
        for preset_file in sorted(os.listdir(presets_dir)):
            if preset_file.endswith('.json'):
                preset_path = os.path.join(presets_dir, preset_file)
                with open(preset_path, 'r', encoding='utf-8') as f:
                    preset = json.load(f)
                print(f'\n{preset["name"]}:')
                print(f'  Descrição: {preset["description"]}')
                print(f'  Shaders: {", ".join(preset["shaders"])}')
                print('  Parâmetros:')
                for param, value in preset['parameters'].items():
                    print(f'    - {param}: {value}')
        return True
    except Exception as e:
        print(f'Erro ao listar presets: {e}', file=sys.stderr)
        return False

def remove_shader(shader_name: str) -> bool:
    """
    Remove um shader instalado.

    Args:
        shader_name: Nome do arquivo de shader.

    Returns:
        True se o shader foi removido com sucesso, False caso contrário.
    """
    try:
        # Procura o shader em todos os diretórios
        for shader_type in SHADER_TYPES:
            shader_path = f'shaders/{shader_type}/{shader_name}'
            if os.path.exists(shader_path):
                os.remove(shader_path)
                print(f'Shader removido: {shader_name}')
                return True

        print(f'Shader não encontrado: {shader_name}', file=sys.stderr)
        return False
    except Exception as e:
        print(f'Erro ao remover shader: {e}', file=sys.stderr)
        return False

def remove_preset(preset_name: str) -> bool:
    """
    Remove um preset.

    Args:
        preset_name: Nome do preset.

    Returns:
        True se o preset foi removido com sucesso, False caso contrário.
    """
    try:
        preset_path = f'shaders/presets/{preset_name.lower()}.json'
        if os.path.exists(preset_path):
            os.remove(preset_path)
            print(f'Preset removido: {preset_name}')
            return True

        print(f'Preset não encontrado: {preset_name}', file=sys.stderr)
        return False
    except Exception as e:
        print(f'Erro ao remover preset: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_shaders.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  install <arquivo>     Instala um shader', file=sys.stderr)
        print('  create-preset <nome> <descrição> <shaders> <parâmetros>')
        print('                        Cria um preset de shader', file=sys.stderr)
        print('  install-defaults      Instala presets padrão', file=sys.stderr)
        print('  list                  Lista shaders instalados', file=sys.stderr)
        print('  list-presets         Lista presets disponíveis', file=sys.stderr)
        print('  remove <shader>       Remove um shader', file=sys.stderr)
        print('  remove-preset <nome>  Remove um preset', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_shader_directories() else 1

    elif command == 'install' and len(sys.argv) > 2:
        return 0 if install_shader(sys.argv[2]) else 1

    elif command == 'create-preset' and len(sys.argv) > 5:
        try:
            name = sys.argv[2]
            description = sys.argv[3]
            shaders = json.loads(sys.argv[4])
            parameters = json.loads(sys.argv[5])
            return 0 if create_preset(name, description, shaders, parameters) else 1
        except json.JSONDecodeError:
            print('Formato inválido para shaders ou parâmetros. Use JSON.',
                  file=sys.stderr)
            return 1

    elif command == 'install-defaults':
        return 0 if install_default_presets() else 1

    elif command == 'list':
        return 0 if list_shaders() else 1

    elif command == 'list-presets':
        return 0 if list_presets() else 1

    elif command == 'remove' and len(sys.argv) > 2:
        return 0 if remove_shader(sys.argv[2]) else 1

    elif command == 'remove-preset' and len(sys.argv) > 2:
        return 0 if remove_preset(sys.argv[2]) else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
