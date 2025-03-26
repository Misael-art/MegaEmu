#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar as configurações do emulador.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

# Configurações padrão
DEFAULT_SETTINGS = {
    'video': {
        'resolution': {
            'width': 1280,
            'height': 720
        },
        'fullscreen': False,
        'vsync': True,
        'aspect_ratio': 'original',  # original, stretch, 4:3, 16:9
        'shader': 'none',
        'filter': 'nearest',  # nearest, linear, bicubic
        'integer_scaling': False,
        'show_fps': False,
        'frame_skip': 0
    },
    'audio': {
        'enabled': True,
        'volume': 100,
        'sample_rate': 44100,
        'buffer_size': 2048,
        'channels': 2,
        'sync_with_video': True,
        'resampling_quality': 'medium'  # low, medium, high
    },
    'input': {
        'default_mapping': 'Default Keyboard',
        'deadzone': 0.15,
        'turbo_speed': 12,  # frames
        'allow_simultaneous_opposite_directions': False,
        'swap_confirm_cancel': False
    },
    'system': {
        'region': 'auto',  # auto, jp, us, eu
        'language': 'pt_BR',
        'save_directory': 'saves',
        'screenshot_directory': 'screenshots',
        'shader_directory': 'shaders',
        'rom_directories': ['roms'],
        'auto_save': True,
        'auto_save_interval': 300,  # segundos
        'rewind_enabled': True,
        'rewind_buffer_size': 60,  # segundos
        'fast_forward_speed': 2.0
    },
    'ui': {
        'theme': 'default',
        'show_menu_bar': True,
        'show_status_bar': True,
        'show_game_info': True,
        'show_notifications': True,
        'notification_duration': 3,  # segundos
        'pause_when_unfocused': True,
        'confirm_exit': True,
        'recent_games_limit': 10
    },
    'debug': {
        'log_level': 'info',  # debug, info, warning, error
        'show_debug_info': False,
        'profile_performance': False,
        'save_crash_dumps': True
    }
}

def create_settings_directory() -> bool:
    """
    Cria o diretório de configurações.

    Returns:
        True se o diretório foi criado com sucesso, False caso contrário.
    """
    try:
        os.makedirs('config', exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretório de configurações: {e}', file=sys.stderr)
        return False

def get_settings_path() -> str:
    """
    Retorna o caminho do arquivo de configurações.

    Returns:
        Caminho do arquivo de configurações.
    """
    return 'config/settings.json'

def load_settings() -> Dict:
    """
    Carrega as configurações do arquivo.

    Returns:
        Dicionário com as configurações.
    """
    settings_path = get_settings_path()
    if os.path.exists(settings_path):
        try:
            with open(settings_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        except Exception as e:
            print(f'Erro ao carregar configurações: {e}', file=sys.stderr)
            return DEFAULT_SETTINGS.copy()
    return DEFAULT_SETTINGS.copy()

def save_settings(settings: Dict) -> bool:
    """
    Salva as configurações no arquivo.

    Args:
        settings: Dicionário com as configurações.

    Returns:
        True se as configurações foram salvas com sucesso, False caso contrário.
    """
    try:
        settings_path = get_settings_path()
        with open(settings_path, 'w', encoding='utf-8') as f:
            json.dump(settings, f, indent=2, ensure_ascii=False)
        return True
    except Exception as e:
        print(f'Erro ao salvar configurações: {e}', file=sys.stderr)
        return False

def validate_settings(settings: Dict) -> Tuple[bool, Optional[str]]:
    """
    Valida as configurações.

    Args:
        settings: Dicionário com as configurações.

    Returns:
        Uma tupla (válido, mensagem) onde válido é um booleano e mensagem é None
        se válido ou uma mensagem de erro caso contrário.
    """
    try:
        # Verifica se todas as seções necessárias existem
        required_sections = ['video', 'audio', 'input', 'system', 'ui', 'debug']
        for section in required_sections:
            if section not in settings:
                return False, f'Seção obrigatória ausente: {section}'

        # Valida configurações de vídeo
        video = settings['video']
        if not isinstance(video['resolution'], dict):
            return False, 'Resolução inválida'
        if not isinstance(video['resolution']['width'], int):
            return False, 'Largura da resolução deve ser um número inteiro'
        if not isinstance(video['resolution']['height'], int):
            return False, 'Altura da resolução deve ser um número inteiro'
        if not isinstance(video['fullscreen'], bool):
            return False, 'Fullscreen deve ser um booleano'
        if video['aspect_ratio'] not in ['original', 'stretch', '4:3', '16:9']:
            return False, 'Proporção de tela inválida'
        if video['filter'] not in ['nearest', 'linear', 'bicubic']:
            return False, 'Filtro inválido'

        # Valida configurações de áudio
        audio = settings['audio']
        if not isinstance(audio['enabled'], bool):
            return False, 'Áudio habilitado deve ser um booleano'
        if not isinstance(audio['volume'], (int, float)) or audio['volume'] < 0 or audio['volume'] > 100:
            return False, 'Volume deve ser um número entre 0 e 100'
        if not isinstance(audio['sample_rate'], int):
            return False, 'Taxa de amostragem deve ser um número inteiro'
        if audio['resampling_quality'] not in ['low', 'medium', 'high']:
            return False, 'Qualidade de reamostragem inválida'

        # Valida configurações de entrada
        input_settings = settings['input']
        if not isinstance(input_settings['deadzone'], (int, float)):
            return False, 'Zona morta deve ser um número'
        if not isinstance(input_settings['turbo_speed'], int):
            return False, 'Velocidade do turbo deve ser um número inteiro'

        # Valida configurações do sistema
        system = settings['system']
        if system['region'] not in ['auto', 'jp', 'us', 'eu']:
            return False, 'Região inválida'
        if not isinstance(system['auto_save'], bool):
            return False, 'Auto-save deve ser um booleano'
        if not isinstance(system['auto_save_interval'], int):
            return False, 'Intervalo de auto-save deve ser um número inteiro'

        # Valida configurações da interface
        ui = settings['ui']
        if not isinstance(ui['show_menu_bar'], bool):
            return False, 'Exibir barra de menu deve ser um booleano'
        if not isinstance(ui['notification_duration'], (int, float)):
            return False, 'Duração das notificações deve ser um número'

        # Valida configurações de debug
        debug = settings['debug']
        if debug['log_level'] not in ['debug', 'info', 'warning', 'error']:
            return False, 'Nível de log inválido'
        if not isinstance(debug['show_debug_info'], bool):
            return False, 'Exibir informações de debug deve ser um booleano'

        return True, None
    except Exception as e:
        return False, f'Erro ao validar configurações: {e}'

def get_setting(section: str, key: str) -> Optional[Union[str, int, float, bool, Dict, List]]:
    """
    Obtém o valor de uma configuração específica.

    Args:
        section: Seção da configuração.
        key: Chave da configuração.

    Returns:
        Valor da configuração ou None se não encontrada.
    """
    try:
        settings = load_settings()
        return settings[section][key]
    except Exception:
        return None

def set_setting(section: str, key: str, value: Union[str, int, float, bool, Dict, List]) -> bool:
    """
    Define o valor de uma configuração específica.

    Args:
        section: Seção da configuração.
        key: Chave da configuração.
        value: Novo valor.

    Returns:
        True se a configuração foi definida com sucesso, False caso contrário.
    """
    try:
        settings = load_settings()
        if section not in settings:
            settings[section] = {}
        settings[section][key] = value
        valid, message = validate_settings(settings)
        if not valid:
            print(f'Configuração inválida: {message}', file=sys.stderr)
            return False
        return save_settings(settings)
    except Exception as e:
        print(f'Erro ao definir configuração: {e}', file=sys.stderr)
        return False

def reset_settings() -> bool:
    """
    Restaura as configurações padrão.

    Returns:
        True se as configurações foram restauradas com sucesso, False caso contrário.
    """
    return save_settings(DEFAULT_SETTINGS.copy())

def export_settings(output_path: str) -> bool:
    """
    Exporta as configurações para um arquivo.

    Args:
        output_path: Caminho do arquivo de saída.

    Returns:
        True se as configurações foram exportadas com sucesso, False caso contrário.
    """
    try:
        settings = load_settings()
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(settings, f, indent=2, ensure_ascii=False)
        return True
    except Exception as e:
        print(f'Erro ao exportar configurações: {e}', file=sys.stderr)
        return False

def import_settings(input_path: str) -> bool:
    """
    Importa configurações de um arquivo.

    Args:
        input_path: Caminho do arquivo de entrada.

    Returns:
        True se as configurações foram importadas com sucesso, False caso contrário.
    """
    try:
        if not os.path.exists(input_path):
            print(f'Arquivo não encontrado: {input_path}', file=sys.stderr)
            return False

        with open(input_path, 'r', encoding='utf-8') as f:
            settings = json.load(f)

        valid, message = validate_settings(settings)
        if not valid:
            print(f'Configurações inválidas: {message}', file=sys.stderr)
            return False

        return save_settings(settings)
    except Exception as e:
        print(f'Erro ao importar configurações: {e}', file=sys.stderr)
        return False

def print_settings(settings: Dict, indent: int = 0) -> None:
    """
    Imprime as configurações de forma hierárquica.

    Args:
        settings: Dicionário com as configurações.
        indent: Nível de indentação.
    """
    for key, value in settings.items():
        if isinstance(value, dict):
            print('  ' * indent + f'{key}:')
            print_settings(value, indent + 1)
        else:
            print('  ' * indent + f'{key}: {value}')

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_settings.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria diretório de configurações', file=sys.stderr)
        print('  get <seção> <chave>   Obtém valor de uma configuração', file=sys.stderr)
        print('  set <seção> <chave> <valor>')
        print('                        Define valor de uma configuração', file=sys.stderr)
        print('  list                  Lista todas as configurações', file=sys.stderr)
        print('  reset                 Restaura configurações padrão', file=sys.stderr)
        print('  export <arquivo>      Exporta configurações', file=sys.stderr)
        print('  import <arquivo>      Importa configurações', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_settings_directory() else 1

    elif command == 'get' and len(sys.argv) > 3:
        value = get_setting(sys.argv[2], sys.argv[3])
        if value is not None:
            print(value)
            return 0
        return 1

    elif command == 'set' and len(sys.argv) > 4:
        try:
            # Tenta converter o valor para o tipo apropriado
            value = sys.argv[4]
            if value.lower() == 'true':
                value = True
            elif value.lower() == 'false':
                value = False
            elif value.replace('.', '').isdigit():
                value = int(value) if value.isdigit() else float(value)
            elif value.startswith('{') or value.startswith('['):
                value = json.loads(value)
            return 0 if set_setting(sys.argv[2], sys.argv[3], value) else 1
        except json.JSONDecodeError:
            print('Formato inválido para o valor. Use JSON para objetos e arrays.',
                  file=sys.stderr)
            return 1

    elif command == 'list':
        settings = load_settings()
        print_settings(settings)
        return 0

    elif command == 'reset':
        return 0 if reset_settings() else 1

    elif command == 'export' and len(sys.argv) > 2:
        return 0 if export_settings(sys.argv[2]) else 1

    elif command == 'import' and len(sys.argv) > 2:
        return 0 if import_settings(sys.argv[2]) else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
