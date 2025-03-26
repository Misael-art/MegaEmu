#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de áudio do emulador.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import platform
import wave
import struct
import numpy as np

# Configurações de áudio
AUDIO_CONFIG = {
    'directories': {
        'samples': 'assets/audio/samples',
        'music': 'assets/audio/music',
        'sfx': 'assets/audio/sfx'
    },
    'formats': {
        'sample_rate': 44100,
        'channels': 2,
        'sample_width': 2,  # bytes (16-bit)
        'max_amplitude': 32767  # 2^15 - 1
    },
    'psg': {
        'clock': 3579545,  # Hz (NTSC)
        'channels': 4,  # 3 square + 1 noise
        'volume_levels': 16,  # 4-bit
        'frequency_min': 20,  # Hz
        'frequency_max': 20000  # Hz
    },
    'fm': {
        'clock': 7670453,  # Hz (NTSC)
        'channels': 6,
        'operators': 4,
        'algorithms': 8,
        'volume_levels': 128,  # 7-bit
        'frequency_min': 20,  # Hz
        'frequency_max': 20000  # Hz
    },
    'samples': {
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
        for directory in AUDIO_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def generate_sine_wave(frequency: float, duration: float) -> np.ndarray:
    """
    Gera uma onda senoidal.

    Args:
        frequency: Frequência em Hz.
        duration: Duração em segundos.

    Returns:
        Array NumPy com as amostras.
    """
    sample_rate = AUDIO_CONFIG['formats']['sample_rate']
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    samples = np.sin(2 * np.pi * frequency * t)
    return samples

def generate_square_wave(frequency: float, duration: float, duty_cycle: float = 0.5) -> np.ndarray:
    """
    Gera uma onda quadrada.

    Args:
        frequency: Frequência em Hz.
        duration: Duração em segundos.
        duty_cycle: Ciclo de trabalho (0.0 a 1.0).

    Returns:
        Array NumPy com as amostras.
    """
    sample_rate = AUDIO_CONFIG['formats']['sample_rate']
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    samples = np.where((t * frequency) % 1.0 < duty_cycle, 1.0, -1.0)
    return samples

def generate_noise(duration: float, white: bool = True) -> np.ndarray:
    """
    Gera ruído branco ou rosa.

    Args:
        duration: Duração em segundos.
        white: True para ruído branco, False para ruído rosa.

    Returns:
        Array NumPy com as amostras.
    """
    sample_rate = AUDIO_CONFIG['formats']['sample_rate']
    num_samples = int(sample_rate * duration)

    if white:
        samples = np.random.uniform(-1.0, 1.0, num_samples)
    else:
        # Ruído rosa (aproximação)
        samples = np.random.uniform(-1.0, 1.0, num_samples)
        b = [0.049922035, -0.095993537, 0.050612699, -0.004408786]
        a = [1, -2.494956002, 2.017265875, -0.522189400]
        samples = np.array(samples)
        from scipy import signal
        samples = signal.lfilter(b, a, samples)

    return samples

def save_wav(samples: np.ndarray, filename: str) -> bool:
    """
    Salva amostras em arquivo WAV.

    Args:
        samples: Array NumPy com as amostras.
        filename: Nome do arquivo.

    Returns:
        True se o arquivo foi salvo com sucesso, False caso contrário.
    """
    try:
        # Normaliza amostras
        max_amplitude = AUDIO_CONFIG['formats']['max_amplitude']
        samples = np.int16(samples * max_amplitude)

        # Cria diretório se não existir
        os.makedirs(os.path.dirname(filename), exist_ok=True)

        # Salva arquivo WAV
        with wave.open(filename, 'w') as wav:
            wav.setnchannels(AUDIO_CONFIG['formats']['channels'])
            wav.setsampwidth(AUDIO_CONFIG['formats']['sample_width'])
            wav.setframerate(AUDIO_CONFIG['formats']['sample_rate'])
            wav.writeframes(samples.tobytes())

        return True
    except Exception as e:
        print(f'Erro ao salvar arquivo WAV: {e}', file=sys.stderr)
        return False

def generate_test_samples() -> bool:
    """
    Gera amostras de teste.

    Returns:
        True se as amostras foram geradas com sucesso, False caso contrário.
    """
    try:
        samples_dir = AUDIO_CONFIG['directories']['samples']

        # Gera tons de teste
        frequencies = [440, 880, 1760]  # A4, A5, A6
        for freq in frequencies:
            # Onda senoidal
            samples = generate_sine_wave(freq, 1.0)
            filename = f"{samples_dir}/sine_{freq}hz.wav"
            save_wav(samples, filename)

            # Onda quadrada
            samples = generate_square_wave(freq, 1.0)
            filename = f"{samples_dir}/square_{freq}hz.wav"
            save_wav(samples, filename)

        # Gera ruídos
        samples = generate_noise(1.0, white=True)
        filename = f"{samples_dir}/white_noise.wav"
        save_wav(samples, filename)

        samples = generate_noise(1.0, white=False)
        filename = f"{samples_dir}/pink_noise.wav"
        save_wav(samples, filename)

        return True
    except Exception as e:
        print(f'Erro ao gerar amostras: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_audio.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  samples              Gera amostras de teste', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_directories() else 1

    elif command == 'samples':
        return 0 if generate_test_samples() else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
