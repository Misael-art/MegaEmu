#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de release do emulador.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import platform
import shutil
import hashlib

# Configurações de release
RELEASE_CONFIG = {
    'directories': {
        'release': 'release',
        'packages': 'release/packages',
        'installers': 'release/installers',
        'artifacts': 'release/artifacts'
    },
    'version': {
        'major': 1,
        'minor': 0,
        'patch': 0,
        'pre': None,  # alpha, beta, rc
        'build': None  # número do build
    },
    'platforms': {
        'windows': {
            'name': 'Windows',
            'architectures': ['x64'],
            'formats': ['zip', 'exe'],
            'files': [
                {'source': 'build/release/bin/mega_emu.exe', 'target': 'bin/mega_emu.exe'},
                {'source': 'build/release/bin/*.dll', 'target': 'bin'},
                {'source': 'assets', 'target': 'assets'},
                {'source': 'docs', 'target': 'docs'},
                {'source': 'LICENSE', 'target': 'LICENSE'},
                {'source': 'README.md', 'target': 'README.md'}
            ]
        },
        'linux': {
            'name': 'Linux',
            'architectures': ['x64'],
            'formats': ['tar.gz', 'deb', 'rpm'],
            'files': [
                {'source': 'build/release/bin/mega_emu', 'target': 'bin/mega_emu'},
                {'source': 'build/release/lib/*.so', 'target': 'lib'},
                {'source': 'assets', 'target': 'assets'},
                {'source': 'docs', 'target': 'docs'},
                {'source': 'LICENSE', 'target': 'LICENSE'},
                {'source': 'README.md', 'target': 'README.md'}
            ]
        },
        'macos': {
            'name': 'macOS',
            'architectures': ['x64'],
            'formats': ['dmg', 'pkg'],
            'files': [
                {'source': 'build/release/bin/mega_emu', 'target': 'bin/mega_emu'},
                {'source': 'build/release/lib/*.dylib', 'target': 'lib'},
                {'source': 'assets', 'target': 'assets'},
                {'source': 'docs', 'target': 'docs'},
                {'source': 'LICENSE', 'target': 'LICENSE'},
                {'source': 'README.md', 'target': 'README.md'}
            ]
        }
    },
    'metadata': {
        'name': 'Mega Emu',
        'description': 'Emulador de Mega Drive/Genesis',
        'author': os.getenv('USER', 'Your Name'),
        'website': 'https://github.com/yourusername/mega-emu',
        'license': 'MIT'
    }
}

def create_directories() -> bool:
    """
    Cria a estrutura de diretórios necessária.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        for directory in RELEASE_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def get_version_string(include_build: bool = False) -> str:
    """
    Obtém string de versão.

    Args:
        include_build: Se deve incluir número do build.

    Returns:
        String de versão.
    """
    version = RELEASE_CONFIG['version']
    version_str = f"{version['major']}.{version['minor']}.{version['patch']}"

    if version['pre']:
        version_str += f"-{version['pre']}"
        if version['build'] and include_build:
            version_str += f".{version['build']}"
    elif version['build'] and include_build:
        version_str += f"+{version['build']}"

    return version_str

def get_platform() -> str:
    """
    Detecta a plataforma atual.

    Returns:
        Nome da plataforma ('windows', 'linux' ou 'macos').
    """
    if sys.platform.startswith('win'):
        return 'windows'
    elif sys.platform.startswith('linux'):
        return 'linux'
    elif sys.platform.startswith('darwin'):
        return 'macos'
    else:
        raise RuntimeError(f'Plataforma não suportada: {sys.platform}')

def copy_release_files(platform: str, arch: str, dest_dir: str) -> bool:
    """
    Copia arquivos para o diretório de release.

    Args:
        platform: Nome da plataforma.
        arch: Arquitetura.
        dest_dir: Diretório de destino.

    Returns:
        True se os arquivos foram copiados com sucesso, False caso contrário.
    """
    try:
        # Obtém lista de arquivos
        files = RELEASE_CONFIG['platforms'][platform]['files']

        # Copia cada arquivo/diretório
        for file in files:
            source = file['source']
            target = os.path.join(dest_dir, file['target'])

            # Cria diretório de destino
            os.makedirs(os.path.dirname(target), exist_ok=True)

            # Copia arquivo/diretório
            if '*' in source:
                # Copia múltiplos arquivos
                for src in Path().glob(source):
                    if src.is_file():
                        shutil.copy2(src, target)
            elif os.path.isdir(source):
                # Copia diretório
                shutil.copytree(source, target, dirs_exist_ok=True)
            else:
                # Copia arquivo
                shutil.copy2(source, target)

        return True
    except Exception as e:
        print(f'Erro ao copiar arquivos: {e}', file=sys.stderr)
        return False

def create_package(platform: str, arch: str, format: str) -> bool:
    """
    Cria pacote de release.

    Args:
        platform: Nome da plataforma.
        arch: Arquitetura.
        format: Formato do pacote.

    Returns:
        True se o pacote foi criado com sucesso, False caso contrário.
    """
    try:
        # Define nomes
        version = get_version_string()
        base_name = f"mega_emu-{version}-{platform}-{arch}"
        stage_dir = os.path.join(RELEASE_CONFIG['directories']['release'], 'stage')
        output_dir = RELEASE_CONFIG['directories']['packages']

        # Limpa diretório de staging
        if os.path.exists(stage_dir):
            shutil.rmtree(stage_dir)
        os.makedirs(stage_dir)

        # Copia arquivos
        if not copy_release_files(platform, arch, stage_dir):
            return False

        # Cria pacote
        if format == 'zip':
            # Arquivo ZIP
            output_file = os.path.join(output_dir, f"{base_name}.zip")
            shutil.make_archive(
                output_file[:-4],  # remove .zip
                'zip',
                stage_dir
            )

        elif format == 'tar.gz':
            # Arquivo TAR.GZ
            output_file = os.path.join(output_dir, f"{base_name}.tar.gz")
            shutil.make_archive(
                output_file[:-7],  # remove .tar.gz
                'gztar',
                stage_dir
            )

        elif format == 'exe':
            # Instalador Windows
            output_file = os.path.join(output_dir, f"{base_name}.exe")
            subprocess.run([
                'makensis',
                '/DVERSION=' + version,
                '/DARCH=' + arch,
                '/DSTAGE_DIR=' + stage_dir,
                '/DOUTPUT_FILE=' + output_file,
                'installer/windows/installer.nsi'
            ], check=True)

        elif format == 'deb':
            # Pacote Debian
            output_file = os.path.join(output_dir, f"{base_name}.deb")
            subprocess.run([
                'dpkg-deb',
                '--build',
                '--root-owner-group',
                stage_dir,
                output_file
            ], check=True)

        elif format == 'rpm':
            # Pacote RPM
            output_file = os.path.join(output_dir, f"{base_name}.rpm")
            subprocess.run([
                'rpmbuild',
                '-bb',
                '--define', f'_version {version}',
                '--define', f'_arch {arch}',
                '--define', f'_stage {stage_dir}',
                '--define', f'_output {output_file}',
                'installer/linux/package.spec'
            ], check=True)

        elif format == 'dmg':
            # Imagem DMG
            output_file = os.path.join(output_dir, f"{base_name}.dmg")
            subprocess.run([
                'hdiutil',
                'create',
                '-volname', 'Mega Emu',
                '-srcfolder', stage_dir,
                '-ov',
                output_file
            ], check=True)

        elif format == 'pkg':
            # Pacote PKG
            output_file = os.path.join(output_dir, f"{base_name}.pkg")
            subprocess.run([
                'pkgbuild',
                '--root', stage_dir,
                '--identifier', 'com.megaemu.app',
                '--version', version,
                output_file
            ], check=True)

        # Calcula checksums
        checksums = {}
        for hash_type in ['md5', 'sha1', 'sha256']:
            hasher = getattr(hashlib, hash_type)()
            with open(output_file, 'rb') as f:
                for chunk in iter(lambda: f.read(4096), b''):
                    hasher.update(chunk)
            checksums[hash_type] = hasher.hexdigest()

        # Salva checksums
        checksum_file = output_file + '.checksums'
        with open(checksum_file, 'w') as f:
            for hash_type, hash_value in checksums.items():
                f.write(f'{hash_type}: {hash_value}\n')

        return True
    except Exception as e:
        print(f'Erro ao criar pacote: {e}', file=sys.stderr)
        return False

def create_release_notes() -> bool:
    """
    Cria notas de release.

    Returns:
        True se as notas foram criadas com sucesso, False caso contrário.
    """
    try:
        version = get_version_string()
        notes_file = os.path.join(RELEASE_CONFIG['directories']['release'],
                                 f'release_notes_{version}.md')

        with open(notes_file, 'w', encoding='utf-8') as f:
            # Cabeçalho
            f.write(f'# {RELEASE_CONFIG["metadata"]["name"]} {version}\n\n')
            f.write(f'Data: {datetime.now().strftime("%Y-%m-%d")}\n\n')

            # Descrição
            f.write('## Sobre\n\n')
            f.write(f'{RELEASE_CONFIG["metadata"]["description"]}\n\n')

            # Mudanças
            f.write('## Mudanças\n\n')
            f.write('TODO: Listar mudanças\n\n')

            # Instalação
            f.write('## Instalação\n\n')
            f.write('### Downloads\n\n')
            for platform in RELEASE_CONFIG['platforms']:
                f.write(f'#### {RELEASE_CONFIG["platforms"][platform]["name"]}\n\n')
                for arch in RELEASE_CONFIG['platforms'][platform]['architectures']:
                    for format in RELEASE_CONFIG['platforms'][platform]['formats']:
                        filename = f'mega_emu-{version}-{platform}-{arch}.{format}'
                        f.write(f'- [{filename}](packages/{filename})\n')
                        f.write(f'  - Checksums: [checksums](packages/{filename}.checksums)\n')
                f.write('\n')

            # Requisitos
            f.write('## Requisitos\n\n')
            f.write('TODO: Listar requisitos\n\n')

            # Problemas Conhecidos
            f.write('## Problemas Conhecidos\n\n')
            f.write('TODO: Listar problemas\n\n')

        return True
    except Exception as e:
        print(f'Erro ao criar notas de release: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_release.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  package [plataforma] [arquitetura] [formato]')
        print('                        Cria pacote de release', file=sys.stderr)
        print('  notes                 Cria notas de release', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_directories() else 1

    elif command == 'package':
        platform = sys.argv[2] if len(sys.argv) > 2 else get_platform()
        if platform not in RELEASE_CONFIG['platforms']:
            print(f'Plataforma inválida: {platform}', file=sys.stderr)
            print('Plataformas válidas:', file=sys.stderr)
            for p in RELEASE_CONFIG['platforms']:
                print(f'  {p}', file=sys.stderr)
            return 1

        arch = sys.argv[3] if len(sys.argv) > 3 else 'x64'
        if arch not in RELEASE_CONFIG['platforms'][platform]['architectures']:
            print(f'Arquitetura inválida: {arch}', file=sys.stderr)
            print('Arquiteturas válidas:', file=sys.stderr)
            for a in RELEASE_CONFIG['platforms'][platform]['architectures']:
                print(f'  {a}', file=sys.stderr)
            return 1

        format = sys.argv[4] if len(sys.argv) > 4 else RELEASE_CONFIG['platforms'][platform]['formats'][0]
        if format not in RELEASE_CONFIG['platforms'][platform]['formats']:
            print(f'Formato inválido: {format}', file=sys.stderr)
            print('Formatos válidos:', file=sys.stderr)
            for f in RELEASE_CONFIG['platforms'][platform]['formats']:
                print(f'  {f}', file=sys.stderr)
            return 1

        return 0 if create_package(platform, arch, format) else 1

    elif command == 'notes':
        return 0 if create_release_notes() else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
