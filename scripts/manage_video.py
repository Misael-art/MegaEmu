#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de vídeo do emulador.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import platform
import numpy as np
from PIL import Image

# Configurações de vídeo
VIDEO_CONFIG = {
    'directories': {
        'textures': 'assets/video/textures',
        'palettes': 'assets/video/palettes',
        'patterns': 'assets/video/patterns',
        'sprites': 'assets/video/sprites',
        'screenshots': 'assets/video/screenshots'
    },
    'formats': {
        'width': 320,
        'height': 224,
        'aspect_ratio': 4/3,
        'refresh_rate': 60,  # Hz (NTSC)
        'color_depth': 9,  # bits (RGB333)
        'max_colors': 512,  # 2^9
        'max_sprites': 80,
        'max_pixels_line': 320,
        'max_pixels_column': 224
    },
    'vdp': {
        'clock': 13423294,  # Hz (NTSC)
        'vram': 65536,  # bytes
        'cram': 128,  # bytes (64 colors)
        'vsram': 80,  # bytes (40 words)
        'registers': 24,
        'planes': 2,
        'plane_width': 64,  # cells
        'plane_height': 32,  # cells
        'cell_width': 8,  # pixels
        'cell_height': 8,  # pixels
        'sprite_width': 32,  # pixels
        'sprite_height': 32  # pixels
    },
    'palettes': {
        'system': {
            'black': (0, 0, 0),
            'white': (255, 255, 255),
            'red': (255, 0, 0),
            'green': (0, 255, 0),
            'blue': (0, 0, 255),
            'yellow': (255, 255, 0),
            'magenta': (255, 0, 255),
            'cyan': (0, 255, 255)
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
        for directory in VIDEO_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def generate_test_pattern(pattern_type: str = 'color_bars') -> np.ndarray:
    """
    Gera um padrão de teste.

    Args:
        pattern_type: Tipo de padrão ('color_bars', 'grid', 'gradient').

    Returns:
        Array NumPy com a imagem.
    """
    width = VIDEO_CONFIG['formats']['width']
    height = VIDEO_CONFIG['formats']['height']
    image = np.zeros((height, width, 3), dtype=np.uint8)

    if pattern_type == 'color_bars':
        # Barras de cores verticais
        colors = list(VIDEO_CONFIG['palettes']['system'].values())
        bar_width = width // len(colors)
        for i, color in enumerate(colors):
            x1 = i * bar_width
            x2 = (i + 1) * bar_width
            image[:, x1:x2] = color

    elif pattern_type == 'grid':
        # Grade 8x8
        image.fill(255)
        for y in range(0, height, 8):
            image[y, :] = 0
        for x in range(0, width, 8):
            image[:, x] = 0

    elif pattern_type == 'gradient':
        # Gradiente RGB
        for y in range(height):
            for x in range(width):
                r = int(255 * x / width)
                g = int(255 * y / height)
                b = int(255 * (x + y) / (width + height))
                image[y, x] = [r, g, b]

    return image

def save_image(image: np.ndarray, filename: str) -> bool:
    """
    Salva uma imagem em arquivo.

    Args:
        image: Array NumPy com a imagem.
        filename: Nome do arquivo.

    Returns:
        True se o arquivo foi salvo com sucesso, False caso contrário.
    """
    try:
        # Converte para PIL Image
        pil_image = Image.fromarray(image)

        # Cria diretório se não existir
        os.makedirs(os.path.dirname(filename), exist_ok=True)

        # Salva arquivo
        pil_image.save(filename)

        return True
    except Exception as e:
        print(f'Erro ao salvar imagem: {e}', file=sys.stderr)
        return False

def generate_test_patterns() -> bool:
    """
    Gera padrões de teste.

    Returns:
        True se os padrões foram gerados com sucesso, False caso contrário.
    """
    try:
        patterns_dir = VIDEO_CONFIG['directories']['patterns']

        # Gera padrões
        pattern_types = ['color_bars', 'grid', 'gradient']
        for pattern_type in pattern_types:
            image = generate_test_pattern(pattern_type)
            filename = f"{patterns_dir}/{pattern_type}.png"
            save_image(image, filename)

        return True
    except Exception as e:
        print(f'Erro ao gerar padrões: {e}', file=sys.stderr)
        return False

def generate_sprite_sheet() -> bool:
    """
    Gera uma sprite sheet de teste.

    Returns:
        True se a sprite sheet foi gerada com sucesso, False caso contrário.
    """
    try:
        sprites_dir = VIDEO_CONFIG['directories']['sprites']

        # Dimensões da sprite sheet
        sprite_width = VIDEO_CONFIG['vdp']['sprite_width']
        sprite_height = VIDEO_CONFIG['vdp']['sprite_height']
        sheet_width = sprite_width * 4
        sheet_height = sprite_height * 4

        # Cria imagem
        image = np.zeros((sheet_height, sheet_width, 3), dtype=np.uint8)

        # Gera sprites de teste
        for y in range(4):
            for x in range(4):
                # Posição do sprite
                x1 = x * sprite_width
                y1 = y * sprite_height
                x2 = x1 + sprite_width
                y2 = y1 + sprite_height

                # Cor do sprite
                color = list(VIDEO_CONFIG['palettes']['system'].values())[y * 4 + x]

                # Desenha sprite
                image[y1:y2, x1:x2] = color

                # Adiciona borda
                image[y1:y2, x1] = [255, 255, 255]
                image[y1:y2, x2-1] = [255, 255, 255]
                image[y1, x1:x2] = [255, 255, 255]
                image[y2-1, x1:x2] = [255, 255, 255]

        # Salva sprite sheet
        filename = f"{sprites_dir}/test_sprites.png"
        save_image(image, filename)

        return True
    except Exception as e:
        print(f'Erro ao gerar sprite sheet: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_video.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  patterns             Gera padrões de teste', file=sys.stderr)
        print('  sprites              Gera sprite sheet de teste', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_directories() else 1

    elif command == 'patterns':
        return 0 if generate_test_patterns() else 1

    elif command == 'sprites':
        return 0 if generate_sprite_sheet() else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
