#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de configuração do emulador.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import platform

# Configurações do emulador
CONFIG = {
    'directories': {
        'config': 'config',
        'profiles': 'config/profiles',
        'presets': 'config/presets'
    },
    'emulator': {
        'name': 'Mega Emu',
        'version': '1.0.0',
        'description': 'Emulador de Mega Drive/Genesis',
        'author': os.getenv('USER', 'Your Name'),
        'website': 'https://github.com/yourusername/mega-emu',
        'license': 'MIT'
    },
    'system': {
        'region': {
            'auto': True,
            'preferred': 'ntsc',
            'options': ['ntsc', 'pal']
        },
        'cpu': {
            'm68k_frequency': 7670453,  # Hz (NTSC)
            'z80_frequency': 3579545,   # Hz (NTSC)
            'overclock': 100  # Porcentagem
        },
        'memory': {
            'rom_path': 'roms',
            'save_path': 'saves',
            'state_path': 'states',
            'bios_required': False
        }
    },
    'video': {
        'resolution': {
            'width': 1280,
            'height': 720
        },
        'fullscreen': False,
        'vsync': True,
        'fps_limit': 60,
        'aspect_ratio': '4:3',
        'scale_mode': 'nearest',
        'shader': 'none',
        'filters': ['none', 'bilinear', 'crt'],
        'shaders': ['none', 'crt-lottes', 'crt-easymode', 'lcd-grid']
    },
    'audio': {
        'enabled': True,
        'volume': 100,
        'rate': 44100,
        'channels': 2,
        'buffer_size': 2048,
        'resampling': 'sinc',
        'latency': 'auto'
    },
    'input': {
        'devices': {
            'keyboard': True,
            'gamepad': True,
            'mouse': False
        },
        'profile': 'default',
        'deadzone': 0.2,
        'turbo_speed': 10
    },
    'debug': {
        'enabled': False,
        'log_level': 'info',
        'trace_enabled': False,
        'profile_enabled': False
    },
    'ui': {
        'theme': 'dark',
        'language': 'pt_BR',
        'font_size': 14,
        'show_fps': True,
        'show_menu': True,
        'show_toolbar': True,
        'show_statusbar': True
    },
    'paths': {
        'roms': 'roms',
        'saves': 'saves',
        'states': 'states',
        'screenshots': 'screenshots',
        'recordings': 'recordings',
        'cheats': 'cheats'
    }
}

def create_directories() -> bool:
    """
    Cria a estrutura de diretórios necessária.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        # Cria diretórios de configuração
        for directory in CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)

        # Cria diretórios de dados
        for directory in CONFIG['paths'].values():
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
        config_dir = CONFIG['directories']['config']

        # Configuração principal
        main_config = {
            'emulator': CONFIG['emulator'],
            'system': CONFIG['system'],
            'video': CONFIG['video'],
            'audio': CONFIG['audio'],
            'input': CONFIG['input'],
            'debug': CONFIG['debug'],
            'ui': CONFIG['ui'],
            'paths': CONFIG['paths']
        }
        with open(f"{config_dir}/config.json", 'w') as f:
            json.dump(main_config, f, indent=4)

        # Perfis predefinidos
        presets = {
            'performance': {
                'name': 'Performance',
                'description': 'Configuração otimizada para desempenho',
                'video': {
                    'resolution': {'width': 640, 'height': 480},
                    'vsync': False,
                    'scale_mode': 'nearest',
                    'shader': 'none'
                },
                'audio': {
                    'buffer_size': 4096,
                    'resampling': 'linear'
                },
                'debug': {
                    'enabled': False,
                    'trace_enabled': False,
                    'profile_enabled': False
                }
            },
            'quality': {
                'name': 'Quality',
                'description': 'Configuração otimizada para qualidade',
                'video': {
                    'resolution': {'width': 1920, 'height': 1080},
                    'vsync': True,
                    'scale_mode': 'linear',
                    'shader': 'crt-lottes'
                },
                'audio': {
                    'buffer_size': 1024,
                    'resampling': 'sinc'
                },
                'debug': {
                    'enabled': False,
                    'trace_enabled': False,
                    'profile_enabled': False
                }
            },
            'debug': {
                'name': 'Debug',
                'description': 'Configuração para desenvolvimento',
                'video': {
                    'resolution': {'width': 1280, 'height': 720},
                    'vsync': True,
                    'scale_mode': 'nearest',
                    'shader': 'none'
                },
                'audio': {
                    'buffer_size': 2048,
                    'resampling': 'linear'
                },
                'debug': {
                    'enabled': True,
                    'log_level': 'debug',
                    'trace_enabled': True,
                    'profile_enabled': True
                }
            }
        }
        with open(f"{config_dir}/presets/presets.json", 'w') as f:
            json.dump(presets, f, indent=4)

        return True
    except Exception as e:
        print(f'Erro ao gerar arquivos de configuração: {e}', file=sys.stderr)
        return False

def generate_config_header() -> bool:
    """
    Gera arquivo de cabeçalho C++ para configuração.

    Returns:
        True se o arquivo foi gerado com sucesso, False caso contrário.
    """
    try:
        header = f"""
#pragma once

#include <string>
#include <vector>
#include <map>

namespace config {{

// Região do sistema
enum class Region {{
    Auto,
    NTSC,
    PAL
}};

// Modo de escala
enum class ScaleMode {{
    Nearest,
    Linear,
    CRT
}};

// Modo de reamostragem
enum class ResamplingMode {{
    Linear,
    Sinc
}};

// Nível de log
enum class LogLevel {{
    Error,
    Warning,
    Info,
    Debug,
    Trace
}};

// Configuração do sistema
struct SystemConfig {{
    Region region;
    bool region_auto;
    uint32_t m68k_frequency;
    uint32_t z80_frequency;
    uint32_t overclock;
    std::string rom_path;
    std::string save_path;
    std::string state_path;
    bool bios_required;
}};

// Configuração de vídeo
struct VideoConfig {{
    uint32_t width;
    uint32_t height;
    bool fullscreen;
    bool vsync;
    uint32_t fps_limit;
    std::string aspect_ratio;
    ScaleMode scale_mode;
    std::string shader;
}};

// Configuração de áudio
struct AudioConfig {{
    bool enabled;
    uint32_t volume;
    uint32_t rate;
    uint32_t channels;
    uint32_t buffer_size;
    ResamplingMode resampling;
    std::string latency;
}};

// Configuração de entrada
struct InputConfig {{
    bool keyboard_enabled;
    bool gamepad_enabled;
    bool mouse_enabled;
    std::string profile;
    float deadzone;
    uint32_t turbo_speed;
}};

// Configuração de debug
struct DebugConfig {{
    bool enabled;
    LogLevel log_level;
    bool trace_enabled;
    bool profile_enabled;
}};

// Configuração da interface
struct UIConfig {{
    std::string theme;
    std::string language;
    uint32_t font_size;
    bool show_fps;
    bool show_menu;
    bool show_toolbar;
    bool show_statusbar;
}};

// Configuração completa
struct Config {{
    SystemConfig system;
    VideoConfig video;
    AudioConfig audio;
    InputConfig input;
    DebugConfig debug;
    UIConfig ui;
    std::map<std::string, std::string> paths;
}};

// Interface de configuração
class IConfig {{
public:
    virtual ~IConfig() = default;

    // Carregamento/salvamento
    virtual bool load(const std::string& filename) = 0;
    virtual bool save(const std::string& filename) = 0;

    // Acesso
    virtual const Config& get() const = 0;
    virtual void set(const Config& config) = 0;

    // Perfis
    virtual bool load_preset(const std::string& name) = 0;
    virtual bool save_preset(const std::string& name) = 0;
}};

}} // namespace config
""".strip()

        # Salva arquivo
        header_file = 'include/config/config.hpp'
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
        print('Uso: manage_config.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  config               Gera arquivos de configuração', file=sys.stderr)
        print('  header               Gera arquivo de cabeçalho C++', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_directories() else 1

    elif command == 'config':
        return 0 if generate_config_files() else 1

    elif command == 'header':
        return 0 if generate_config_header() else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
