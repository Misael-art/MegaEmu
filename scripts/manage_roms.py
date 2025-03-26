#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de ROM do emulador.
"""

import json
import os
import sys
import hashlib
import zlib
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import platform

# Configurações de ROM
ROM_CONFIG = {
    'directories': {
        'roms': 'roms',
        'database': 'database',
        'patches': 'patches',
        'saves': 'saves',
        'states': 'states'
    },
    'formats': {
        'extensions': ['.md', '.bin', '.gen', '.smd'],
        'patch_extensions': ['.ips', '.ups', '.bps']
    },
    'database': {
        'name': 'mega_emu.db',
        'schema': {
            'roms': '''
                CREATE TABLE IF NOT EXISTS roms (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    name TEXT NOT NULL,
                    path TEXT NOT NULL,
                    size INTEGER NOT NULL,
                    crc32 TEXT NOT NULL,
                    md5 TEXT NOT NULL,
                    sha1 TEXT NOT NULL,
                    region TEXT,
                    system TEXT,
                    company TEXT,
                    year TEXT,
                    genre TEXT,
                    description TEXT,
                    rating INTEGER,
                    favorite BOOLEAN DEFAULT 0,
                    last_played DATETIME,
                    play_count INTEGER DEFAULT 0,
                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
                )
            ''',
            'patches': '''
                CREATE TABLE IF NOT EXISTS patches (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    rom_id INTEGER,
                    name TEXT NOT NULL,
                    path TEXT NOT NULL,
                    type TEXT NOT NULL,
                    description TEXT,
                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                    FOREIGN KEY (rom_id) REFERENCES roms (id)
                )
            '''
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
        for directory in ROM_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def calculate_checksums(file_path: str) -> Tuple[str, str, str]:
    """
    Calcula os checksums de um arquivo.

    Args:
        file_path: Caminho do arquivo.

    Returns:
        Tupla com CRC32, MD5 e SHA1.
    """
    try:
        with open(file_path, 'rb') as f:
            data = f.read()
            crc32 = format(zlib.crc32(data) & 0xFFFFFFFF, '08x')
            md5 = hashlib.md5(data).hexdigest()
            sha1 = hashlib.sha1(data).hexdigest()
            return crc32, md5, sha1
    except Exception as e:
        print(f'Erro ao calcular checksums: {e}', file=sys.stderr)
        return ('', '', '')

def scan_roms() -> List[Dict[str, str]]:
    """
    Escaneia o diretório de ROMs.

    Returns:
        Lista de dicionários com informações das ROMs.
    """
    roms = []
    try:
        rom_dir = ROM_CONFIG['directories']['roms']
        extensions = ROM_CONFIG['formats']['extensions']

        for ext in extensions:
            for file in Path(rom_dir).glob(f'*{ext}'):
                crc32, md5, sha1 = calculate_checksums(str(file))
                rom_info = {
                    'name': file.stem,
                    'path': str(file),
                    'size': file.stat().st_size,
                    'crc32': crc32,
                    'md5': md5,
                    'sha1': sha1,
                    'extension': ext
                }
                roms.append(rom_info)

        return roms
    except Exception as e:
        print(f'Erro ao escanear ROMs: {e}', file=sys.stderr)
        return []

def generate_database() -> bool:
    """
    Gera o banco de dados de ROMs.

    Returns:
        True se o banco foi gerado com sucesso, False caso contrário.
    """
    try:
        import sqlite3

        db_path = os.path.join(
            ROM_CONFIG['directories']['database'],
            ROM_CONFIG['database']['name']
        )

        # Conecta ao banco
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()

        # Cria tabelas
        for table in ROM_CONFIG['database']['schema'].values():
            cursor.execute(table)

        # Escaneia ROMs
        roms = scan_roms()

        # Insere ROMs
        for rom in roms:
            cursor.execute('''
                INSERT INTO roms (
                    name, path, size, crc32, md5, sha1
                ) VALUES (?, ?, ?, ?, ?, ?)
            ''', (
                rom['name'],
                rom['path'],
                rom['size'],
                rom['crc32'],
                rom['md5'],
                rom['sha1']
            ))

        conn.commit()
        conn.close()
        return True
    except Exception as e:
        print(f'Erro ao gerar banco de dados: {e}', file=sys.stderr)
        return False

def generate_rom_header() -> bool:
    """
    Gera arquivo de cabeçalho C++ para ROMs.

    Returns:
        True se o arquivo foi gerado com sucesso, False caso contrário.
    """
    try:
        header = f"""
#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace rom {{

// Região da ROM
enum class Region {{
    Unknown,
    Japan,
    USA,
    Europe,
    World
}};

// Sistema
enum class System {{
    Unknown,
    MegaDrive,
    Genesis,
    MegaCD,
    SegaCD,
    ThirtyTwoX
}};

// Informações da ROM
struct ROMInfo {{
    std::string name;
    std::string path;
    uint64_t size;
    std::string crc32;
    std::string md5;
    std::string sha1;
    Region region;
    System system;
    std::string company;
    std::string year;
    std::string genre;
    std::string description;
    uint32_t rating;
    bool favorite;
    std::string last_played;
    uint32_t play_count;
}};

// Interface de gerenciamento de ROM
class IROMManager {{
public:
    virtual ~IROMManager() = default;

    // Carregamento
    virtual bool load(const std::string& path) = 0;
    virtual bool unload() = 0;

    // Informações
    virtual const ROMInfo& get_info() const = 0;
    virtual bool set_info(const ROMInfo& info) = 0;

    // Banco de dados
    virtual bool add_to_database() = 0;
    virtual bool remove_from_database() = 0;
    virtual bool update_in_database() = 0;

    // Patches
    virtual bool apply_patch(const std::string& patch_path) = 0;
    virtual bool create_patch(const std::string& patch_path) = 0;

    // Estados
    virtual bool save_state(const std::string& state_path) = 0;
    virtual bool load_state(const std::string& state_path) = 0;
}};

}} // namespace rom
""".strip()

        # Salva arquivo
        header_file = 'include/rom/rom.hpp'
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
        print('Uso: manage_roms.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  scan                  Escaneia ROMs', file=sys.stderr)
        print('  database              Gera banco de dados', file=sys.stderr)
        print('  header               Gera arquivo de cabeçalho C++', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_directories() else 1

    elif command == 'scan':
        roms = scan_roms()
        for rom in roms:
            print(f"{rom['name']} ({rom['size']} bytes)")
            print(f"  CRC32: {rom['crc32']}")
            print(f"  MD5: {rom['md5']}")
            print(f"  SHA1: {rom['sha1']}")
        return 0

    elif command == 'database':
        return 0 if generate_database() else 1

    elif command == 'header':
        return 0 if generate_rom_header() else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
