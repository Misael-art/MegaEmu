#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os temas da interface do emulador.
"""

import json
import os
import shutil
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple

# Estrutura padrão de um tema
DEFAULT_THEME = {
    'name': 'Default',
    'description': 'Tema padrão do emulador',
    'version': '1.0.0',
    'author': 'Mega Emu Team',
    'colors': {
        'primary': '#2196F3',
        'secondary': '#FFC107',
        'background': '#121212',
        'surface': '#1E1E1E',
        'error': '#CF6679',
        'text': {
            'primary': '#FFFFFF',
            'secondary': '#B3FFFFFF',
            'disabled': '#666666'
        },
        'button': {
            'background': '#2196F3',
            'text': '#FFFFFF',
            'hover': '#1976D2',
            'disabled': '#666666'
        },
        'input': {
            'background': '#1E1E1E',
            'text': '#FFFFFF',
            'border': '#666666',
            'focus': '#2196F3'
        },
        'menu': {
            'background': '#1E1E1E',
            'text': '#FFFFFF',
            'hover': '#2C2C2C',
            'selected': '#2196F3'
        }
    },
    'fonts': {
        'primary': 'Roboto',
        'secondary': 'Open Sans',
        'monospace': 'Roboto Mono'
    },
    'sizes': {
        'text': {
            'small': '12px',
            'medium': '14px',
            'large': '16px',
            'xlarge': '20px'
        },
        'spacing': {
            'small': '4px',
            'medium': '8px',
            'large': '16px',
            'xlarge': '24px'
        },
        'border-radius': {
            'small': '4px',
            'medium': '8px',
            'large': '12px',
            'circle': '50%'
        }
    },
    'icons': {
        'menu': 'menu.svg',
        'close': 'close.svg',
        'settings': 'settings.svg',
        'play': 'play.svg',
        'pause': 'pause.svg',
        'stop': 'stop.svg',
        'save': 'save.svg',
        'load': 'load.svg',
        'fullscreen': 'fullscreen.svg'
    }
}

# Temas padrão adicionais
DEFAULT_THEMES = {
    'dark': {
        'name': 'Dark',
        'description': 'Tema escuro moderno',
        'version': '1.0.0',
        'author': 'Mega Emu Team',
        'colors': {
            'primary': '#BB86FC',
            'secondary': '#03DAC6',
            'background': '#121212',
            'surface': '#1E1E1E',
            'error': '#CF6679',
            'text': {
                'primary': '#FFFFFF',
                'secondary': '#B3FFFFFF',
                'disabled': '#666666'
            },
            'button': {
                'background': '#BB86FC',
                'text': '#000000',
                'hover': '#9F75D7',
                'disabled': '#666666'
            },
            'input': {
                'background': '#1E1E1E',
                'text': '#FFFFFF',
                'border': '#666666',
                'focus': '#BB86FC'
            },
            'menu': {
                'background': '#1E1E1E',
                'text': '#FFFFFF',
                'hover': '#2C2C2C',
                'selected': '#BB86FC'
            }
        }
    },
    'light': {
        'name': 'Light',
        'description': 'Tema claro e minimalista',
        'version': '1.0.0',
        'author': 'Mega Emu Team',
        'colors': {
            'primary': '#1976D2',
            'secondary': '#FFA000',
            'background': '#FFFFFF',
            'surface': '#F5F5F5',
            'error': '#B00020',
            'text': {
                'primary': '#000000',
                'secondary': '#666666',
                'disabled': '#999999'
            },
            'button': {
                'background': '#1976D2',
                'text': '#FFFFFF',
                'hover': '#1565C0',
                'disabled': '#CCCCCC'
            },
            'input': {
                'background': '#FFFFFF',
                'text': '#000000',
                'border': '#CCCCCC',
                'focus': '#1976D2'
            },
            'menu': {
                'background': '#F5F5F5',
                'text': '#000000',
                'hover': '#E0E0E0',
                'selected': '#1976D2'
            }
        }
    },
    'retro': {
        'name': 'Retro',
        'description': 'Tema inspirado em consoles clássicos',
        'version': '1.0.0',
        'author': 'Mega Emu Team',
        'colors': {
            'primary': '#E60012',
            'secondary': '#FFD700',
            'background': '#1A1B1E',
            'surface': '#2A2B2E',
            'error': '#FF0000',
            'text': {
                'primary': '#FFFFFF',
                'secondary': '#B3FFFFFF',
                'disabled': '#666666'
            },
            'button': {
                'background': '#E60012',
                'text': '#FFFFFF',
                'hover': '#CC0000',
                'disabled': '#666666'
            },
            'input': {
                'background': '#2A2B2E',
                'text': '#FFFFFF',
                'border': '#666666',
                'focus': '#E60012'
            },
            'menu': {
                'background': '#2A2B2E',
                'text': '#FFFFFF',
                'hover': '#3A3B3E',
                'selected': '#E60012'
            }
        }
    }
}

def create_theme_directories() -> bool:
    """
    Cria a estrutura de diretórios para temas.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        # Cria diretórios principais
        dirs = ['themes', 'themes/icons']
        for directory in dirs:
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def validate_theme(theme: Dict) -> Tuple[bool, Optional[str]]:
    """
    Valida um tema.

    Args:
        theme: O dicionário contendo o tema.

    Returns:
        Uma tupla (válido, mensagem) onde válido é um booleano e mensagem é None
        se válido ou uma mensagem de erro caso contrário.
    """
    required_fields = ['name', 'description', 'version', 'author', 'colors']
    for field in required_fields:
        if field not in theme:
            return False, f'Campo obrigatório ausente: {field}'

    required_colors = ['primary', 'secondary', 'background', 'surface', 'error',
                      'text', 'button', 'input', 'menu']
    for color in required_colors:
        if color not in theme['colors']:
            return False, f'Cor obrigatória ausente: {color}'

    return True, None

def install_theme(theme_path: str) -> bool:
    """
    Instala um tema.

    Args:
        theme_path: O caminho do arquivo de tema.

    Returns:
        True se o tema foi instalado com sucesso, False caso contrário.
    """
    try:
        # Verifica se o arquivo existe
        if not os.path.exists(theme_path):
            print(f'Arquivo não encontrado: {theme_path}', file=sys.stderr)
            return False

        # Lê o tema
        with open(theme_path, 'r', encoding='utf-8') as f:
            theme = json.load(f)

        # Valida o tema
        valid, message = validate_theme(theme)
        if not valid:
            print(f'Tema inválido: {message}', file=sys.stderr)
            return False

        # Copia ícones se existirem
        if 'icons' in theme:
            icons_dir = os.path.join(os.path.dirname(theme_path), 'icons')
            if os.path.exists(icons_dir):
                for icon_name in theme['icons'].values():
                    icon_path = os.path.join(icons_dir, icon_name)
                    if os.path.exists(icon_path):
                        shutil.copy2(icon_path, 'themes/icons')

        # Salva o tema
        theme_name = theme['name'].lower().replace(' ', '_')
        output_path = f'themes/{theme_name}.json'
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(theme, f, indent=2, ensure_ascii=False)

        print(f'Tema instalado: {theme["name"]}')
        return True
    except Exception as e:
        print(f'Erro ao instalar tema: {e}', file=sys.stderr)
        return False

def install_default_themes() -> bool:
    """
    Instala os temas padrão.

    Returns:
        True se os temas foram instalados com sucesso, False caso contrário.
    """
    try:
        # Instala o tema padrão
        with open('themes/default.json', 'w', encoding='utf-8') as f:
            json.dump(DEFAULT_THEME, f, indent=2, ensure_ascii=False)

        # Instala os temas adicionais
        for theme_id, theme_data in DEFAULT_THEMES.items():
            # Mescla com o tema padrão para garantir todos os campos
            theme = DEFAULT_THEME.copy()
            theme.update(theme_data)

            with open(f'themes/{theme_id}.json', 'w', encoding='utf-8') as f:
                json.dump(theme, f, indent=2, ensure_ascii=False)

        print('Temas padrão instalados com sucesso')
        return True
    except Exception as e:
        print(f'Erro ao instalar temas padrão: {e}', file=sys.stderr)
        return False

def list_themes() -> bool:
    """
    Lista todos os temas instalados.

    Returns:
        True se a listagem foi bem sucedida, False caso contrário.
    """
    try:
        themes_dir = 'themes'
        if not os.path.exists(themes_dir):
            print('Nenhum tema encontrado.')
            return True

        print('\nTemas instalados:')
        for theme_file in sorted(os.listdir(themes_dir)):
            if theme_file.endswith('.json'):
                theme_path = os.path.join(themes_dir, theme_file)
                with open(theme_path, 'r', encoding='utf-8') as f:
                    theme = json.load(f)
                print(f'\n{theme["name"]}:')
                print(f'  Descrição: {theme["description"]}')
                print(f'  Versão: {theme["version"]}')
                print(f'  Autor: {theme["author"]}')
        return True
    except Exception as e:
        print(f'Erro ao listar temas: {e}', file=sys.stderr)
        return False

def remove_theme(theme_name: str) -> bool:
    """
    Remove um tema.

    Args:
        theme_name: Nome do tema.

    Returns:
        True se o tema foi removido com sucesso, False caso contrário.
    """
    try:
        # Não permite remover o tema padrão
        if theme_name.lower() == 'default':
            print('Não é possível remover o tema padrão.', file=sys.stderr)
            return False

        theme_path = f'themes/{theme_name.lower()}.json'
        if os.path.exists(theme_path):
            # Remove o arquivo do tema
            os.remove(theme_path)

            # Remove ícones não utilizados por outros temas
            if os.path.exists('themes/icons'):
                used_icons = set()
                # Coleta ícones usados por outros temas
                for theme_file in os.listdir('themes'):
                    if theme_file.endswith('.json') and theme_file != f'{theme_name.lower()}.json':
                        with open(os.path.join('themes', theme_file), 'r', encoding='utf-8') as f:
                            theme = json.load(f)
                            if 'icons' in theme:
                                used_icons.update(theme['icons'].values())

                # Remove ícones não utilizados
                for icon in os.listdir('themes/icons'):
                    if icon not in used_icons:
                        os.remove(os.path.join('themes/icons', icon))

            print(f'Tema removido: {theme_name}')
            return True

        print(f'Tema não encontrado: {theme_name}', file=sys.stderr)
        return False
    except Exception as e:
        print(f'Erro ao remover tema: {e}', file=sys.stderr)
        return False

def export_theme(theme_name: str, output_path: str) -> bool:
    """
    Exporta um tema com seus ícones.

    Args:
        theme_name: Nome do tema.
        output_path: Caminho de saída.

    Returns:
        True se o tema foi exportado com sucesso, False caso contrário.
    """
    try:
        theme_file = f'themes/{theme_name.lower()}.json'
        if not os.path.exists(theme_file):
            print(f'Tema não encontrado: {theme_name}', file=sys.stderr)
            return False

        # Cria diretório de saída
        os.makedirs(output_path, exist_ok=True)
        icons_dir = os.path.join(output_path, 'icons')
        os.makedirs(icons_dir, exist_ok=True)

        # Copia o tema
        with open(theme_file, 'r', encoding='utf-8') as f:
            theme = json.load(f)

        # Copia os ícones
        if 'icons' in theme:
            for icon_name in theme['icons'].values():
                icon_path = os.path.join('themes/icons', icon_name)
                if os.path.exists(icon_path):
                    shutil.copy2(icon_path, icons_dir)

        # Salva o tema
        output_file = os.path.join(output_path, f'{theme_name.lower()}.json')
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(theme, f, indent=2, ensure_ascii=False)

        print(f'Tema exportado para: {output_path}')
        return True
    except Exception as e:
        print(f'Erro ao exportar tema: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_themes.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  install <arquivo>     Instala um tema', file=sys.stderr)
        print('  install-defaults      Instala temas padrão', file=sys.stderr)
        print('  list                  Lista temas instalados', file=sys.stderr)
        print('  remove <tema>         Remove um tema', file=sys.stderr)
        print('  export <tema> <dir>   Exporta um tema', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_theme_directories() else 1

    elif command == 'install' and len(sys.argv) > 2:
        return 0 if install_theme(sys.argv[2]) else 1

    elif command == 'install-defaults':
        return 0 if install_default_themes() else 1

    elif command == 'list':
        return 0 if list_themes() else 1

    elif command == 'remove' and len(sys.argv) > 2:
        return 0 if remove_theme(sys.argv[2]) else 1

    elif command == 'export' and len(sys.argv) > 3:
        return 0 if export_theme(sys.argv[2], sys.argv[3]) else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
