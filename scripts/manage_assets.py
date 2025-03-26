#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos do emulador.
"""

import json
import os
import sys
import shutil
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import platform

# Configurações dos recursos
ASSETS_CONFIG = {
    'directories': {
        'shaders': 'assets/shaders',
        'textures': 'assets/textures',
        'fonts': 'assets/fonts',
        'icons': 'assets/icons',
        'themes': 'assets/themes',
        'sounds': 'assets/sounds'
    },
    'shaders': {
        'vertex': [
            'basic.vert',
            'crt.vert',
            'lcd.vert'
        ],
        'fragment': [
            'basic.frag',
            'crt.frag',
            'lcd.frag'
        ],
        'compute': [
            'blur.comp',
            'bloom.comp'
        ]
    },
    'textures': {
        'ui': [
            'logo.png',
            'background.png',
            'button.png',
            'checkbox.png',
            'radio.png',
            'slider.png',
            'scrollbar.png'
        ],
        'icons': [
            'play.png',
            'pause.png',
            'stop.png',
            'reset.png',
            'save.png',
            'load.png',
            'settings.png',
            'fullscreen.png',
            'debug.png'
        ]
    },
    'fonts': {
        'ui': [
            'Roboto-Regular.ttf',
            'Roboto-Bold.ttf',
            'RobotoMono-Regular.ttf'
        ]
    },
    'themes': {
        'dark': {
            'name': 'Dark Theme',
            'colors': {
                'text': '#FFFFFF',
                'background': '#1A1A1A',
                'primary': '#2A2A2A',
                'secondary': '#3A3A3A',
                'accent': '#4A4A4A',
                'border': '#333333'
            }
        },
        'light': {
            'name': 'Light Theme',
            'colors': {
                'text': '#000000',
                'background': '#F0F0F0',
                'primary': '#E0E0E0',
                'secondary': '#D0D0D0',
                'accent': '#C0C0C0',
                'border': '#B0B0B0'
            }
        }
    },
    'sounds': {
        'ui': [
            'click.wav',
            'hover.wav',
            'error.wav',
            'success.wav'
        ]
    }
}

def create_directories() -> bool:
    """
    Cria a estrutura de diretórios necessária.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        for directory in ASSETS_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def generate_basic_shaders() -> bool:
    """
    Gera shaders básicos.

    Returns:
        True se os shaders foram gerados com sucesso, False caso contrário.
    """
    try:
        # Shader básico
        basic_vert = """
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
""".strip()

        basic_frag = """
#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
}
""".strip()

        # Shader CRT
        crt_vert = """
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
""".strip()

        crt_frag = """
#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 resolution;
uniform float time;

void main()
{
    vec2 uv = TexCoord;

    // Curvatura
    vec2 curve = (uv * 2.0 - 1.0);
    uv = uv + curve * curve * curve * 0.1;

    // Scanlines
    float scanline = sin(uv.y * resolution.y * 2.0) * 0.02;

    // Vignette
    float vignette = uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y);
    vignette = pow(vignette * 15.0, 0.25);

    // Cor
    vec4 color = texture(screenTexture, uv);
    color.rgb += scanline;
    color.rgb *= vignette;

    FragColor = color;
}
""".strip()

        # Salva shaders
        shader_dir = ASSETS_CONFIG['directories']['shaders']

        with open(f"{shader_dir}/basic.vert", 'w') as f:
            f.write(basic_vert)
        with open(f"{shader_dir}/basic.frag", 'w') as f:
            f.write(basic_frag)
        with open(f"{shader_dir}/crt.vert", 'w') as f:
            f.write(crt_vert)
        with open(f"{shader_dir}/crt.frag", 'w') as f:
            f.write(crt_frag)

        return True
    except Exception as e:
        print(f'Erro ao gerar shaders: {e}', file=sys.stderr)
        return False

def generate_theme_files() -> bool:
    """
    Gera arquivos de tema.

    Returns:
        True se os temas foram gerados com sucesso, False caso contrário.
    """
    try:
        theme_dir = ASSETS_CONFIG['directories']['themes']

        for theme_name, theme_data in ASSETS_CONFIG['themes'].items():
            theme_file = f"{theme_dir}/{theme_name}.json"
            with open(theme_file, 'w') as f:
                json.dump(theme_data, f, indent=4)

        return True
    except Exception as e:
        print(f'Erro ao gerar temas: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_assets.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  shaders              Gera shaders básicos', file=sys.stderr)
        print('  themes               Gera arquivos de tema', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_directories() else 1

    elif command == 'shaders':
        return 0 if generate_basic_shaders() else 1

    elif command == 'themes':
        return 0 if generate_theme_files() else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
