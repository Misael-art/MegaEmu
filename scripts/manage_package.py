#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de empacotamento do emulador.
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
import logging
import logging.handlers
import tempfile
import zipfile
import hashlib
import base64
import hmac

# Configurações de empacotamento
PACKAGE_CONFIG = {
    'directories': {
        'package': 'package',
        'temp': 'package/temp',
        'dist': 'package/dist',
        'logs': 'package/logs'
    },
    'metadata': {
        'name': 'Mega Emu',
        'version': '1.0.0',
        'description': 'Emulador de Mega Drive/Genesis',
        'author': 'Your Name',
        'license': 'MIT',
        'homepage': 'https://github.com/yourusername/mega-emu'
    },
    'formats': {
        'windows': {
            'installer': {
                'format': 'exe',
                'tool': 'innosetup',
                'template': 'templates/setup.iss',
                'icon': 'assets/icons/mega_emu.ico'
            },
            'portable': {
                'format': 'zip',
                'compression': 'deflate'
            }
        },
        'linux': {
            'deb': {
                'format': 'deb',
                'control': {
                    'Package': 'mega-emu',
                    'Version': '{version}',
                    'Architecture': '{arch}',
                    'Maintainer': '{author}',
                    'Description': '{description}',
                    'Depends': 'libsdl2-2.0-0, libopenal1, libglew2.2'
                }
            },
            'rpm': {
                'format': 'rpm',
                'spec': 'templates/mega-emu.spec'
            },
            'appimage': {
                'format': 'AppImage',
                'tool': 'appimagetool',
                'desktop': 'templates/mega-emu.desktop'
            }
        },
        'macos': {
            'dmg': {
                'format': 'dmg',
                'volume_name': 'Mega Emu',
                'icon': 'assets/icons/mega_emu.icns',
                'background': 'assets/images/dmg_background.png'
            },
            'pkg': {
                'format': 'pkg',
                'identifier': 'com.megaemu.app',
                'scripts': 'templates/scripts'
            }
        }
    },
    'files': {
        'bin': [
            'bin/mega_emu',
            'bin/*.dll',
            'bin/*.so',
            'bin/*.dylib'
        ],
        'lib': [
            'lib/*.dll',
            'lib/*.so',
            'lib/*.dylib'
        ],
        'share': [
            'share/assets/**/*',
            'share/config/**/*',
            'share/docs/**/*'
        ]
    },
    'verification': {
        'enabled': True,
        'algorithms': ['sha256', 'sha512'],
        'signature': True
    },
    'logging': {
        'enabled': True,
        'level': 'INFO',
        'format': '%(asctime)s [%(levelname)s] %(message)s',
        'rotation': {
            'when': 'D',
            'interval': 1,
            'backupCount': 7
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
        for directory in PACKAGE_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def collect_files() -> Dict[str, List[str]]:
    """
    Coleta arquivos para empacotamento.

    Returns:
        Dicionário com arquivos coletados por tipo.
    """
    try:
        print('\nColetando arquivos...')
        files = {}

        for dir_type, patterns in PACKAGE_CONFIG['files'].items():
            files[dir_type] = []
            for pattern in patterns:
                for path in Path('.').glob(pattern):
                    if path.is_file():
                        files[dir_type].append(str(path))

        return files
    except Exception as e:
        print(f'Erro ao coletar arquivos: {e}', file=sys.stderr)
        return {}

def create_windows_installer(files: Dict[str, List[str]]) -> Optional[str]:
    """
    Cria instalador para Windows.

    Args:
        files: Dicionário com arquivos por tipo.

    Returns:
        Caminho do instalador ou None se falhar.
    """
    try:
        config = PACKAGE_CONFIG['formats']['windows']['installer']
        print('\nCriando instalador Windows...')

        # Cria diretório temporário
        temp_dir = tempfile.mkdtemp(dir=PACKAGE_CONFIG['directories']['temp'])

        try:
            # Copia arquivos
            for dir_type, paths in files.items():
                for path in paths:
                    dest = os.path.join(temp_dir, dir_type,
                                    os.path.basename(path))
                    os.makedirs(os.path.dirname(dest), exist_ok=True)
                    shutil.copy2(path, dest)

            # Processa template
            with open(config['template'], 'r') as f:
                template = f.read()

            template = template.format(
                name=PACKAGE_CONFIG['metadata']['name'],
                version=PACKAGE_CONFIG['metadata']['version'],
                publisher=PACKAGE_CONFIG['metadata']['author'],
                website=PACKAGE_CONFIG['metadata']['homepage'],
                source_dir=temp_dir,
                output_dir=PACKAGE_CONFIG['directories']['dist'],
                icon=config['icon']
            )

            # Salva script
            script_path = os.path.join(temp_dir, 'setup.iss')
            with open(script_path, 'w') as f:
                f.write(template)

            # Compila instalador
            subprocess.run([
                'iscc',
                script_path
            ], check=True)

            # Encontra instalador
            installer = next(
                Path(PACKAGE_CONFIG['directories']['dist']).glob('*.exe')
            )

            print(f'Instalador criado: {installer}')
            return str(installer)
        finally:
            shutil.rmtree(temp_dir)
    except Exception as e:
        print(f'Erro ao criar instalador Windows: {e}', file=sys.stderr)
        return None

def create_linux_package(files: Dict[str, List[str]], format: str) -> Optional[str]:
    """
    Cria pacote para Linux.

    Args:
        files: Dicionário com arquivos por tipo.
        format: Formato do pacote ('deb', 'rpm' ou 'appimage').

    Returns:
        Caminho do pacote ou None se falhar.
    """
    try:
        config = PACKAGE_CONFIG['formats']['linux'][format]
        print(f'\nCriando pacote Linux {format}...')

        # Cria diretório temporário
        temp_dir = tempfile.mkdtemp(dir=PACKAGE_CONFIG['directories']['temp'])

        try:
            if format == 'deb':
                # Cria estrutura
                for dir_type, paths in files.items():
                    for path in paths:
                        if dir_type == 'bin':
                            dest = os.path.join(temp_dir, 'usr/bin',
                                           os.path.basename(path))
                        elif dir_type == 'lib':
                            dest = os.path.join(temp_dir, 'usr/lib',
                                           os.path.basename(path))
                        elif dir_type == 'share':
                            dest = os.path.join(temp_dir, 'usr/share/mega-emu',
                                           os.path.relpath(path, 'share'))
                        os.makedirs(os.path.dirname(dest), exist_ok=True)
                        shutil.copy2(path, dest)

                # Cria DEBIAN/control
                control_dir = os.path.join(temp_dir, 'DEBIAN')
                os.makedirs(control_dir)

                with open(os.path.join(control_dir, 'control'), 'w') as f:
                    for key, value in config['control'].items():
                        f.write(f'{key}: {value.format(**PACKAGE_CONFIG["metadata"])}\n')

                # Cria pacote
                package = os.path.join(
                    PACKAGE_CONFIG['directories']['dist'],
                    f'mega-emu_{PACKAGE_CONFIG["metadata"]["version"]}_{platform.machine()}.deb'
                )
                subprocess.run([
                    'dpkg-deb',
                    '--build',
                    temp_dir,
                    package
                ], check=True)

            elif format == 'rpm':
                # Copia arquivos para SOURCES
                rpmbuild_dir = os.path.expanduser('~/rpmbuild')
                sources_dir = os.path.join(rpmbuild_dir, 'SOURCES')
                os.makedirs(sources_dir, exist_ok=True)

                archive = os.path.join(
                    sources_dir,
                    f'mega-emu-{PACKAGE_CONFIG["metadata"]["version"]}.tar.gz'
                )

                with tarfile.open(archive, 'w:gz') as tar:
                    for dir_type, paths in files.items():
                        for path in paths:
                            tar.add(path)

                # Processa spec
                with open(config['spec'], 'r') as f:
                    spec = f.read()

                spec = spec.format(
                    name=PACKAGE_CONFIG['metadata']['name'],
                    version=PACKAGE_CONFIG['metadata']['version'],
                    release='1',
                    summary=PACKAGE_CONFIG['metadata']['description'],
                    license=PACKAGE_CONFIG['metadata']['license'],
                    url=PACKAGE_CONFIG['metadata']['homepage']
                )

                # Salva spec
                specs_dir = os.path.join(rpmbuild_dir, 'SPECS')
                os.makedirs(specs_dir, exist_ok=True)
                spec_path = os.path.join(specs_dir, 'mega-emu.spec')
                with open(spec_path, 'w') as f:
                    f.write(spec)

                # Cria pacote
                subprocess.run([
                    'rpmbuild',
                    '-ba',
                    spec_path
                ], check=True)

                # Move pacote
                rpm_path = os.path.join(
                    rpmbuild_dir,
                    'RPMS',
                    platform.machine(),
                    f'mega-emu-{PACKAGE_CONFIG["metadata"]["version"]}-1.{platform.machine()}.rpm'
                )
                package = os.path.join(
                    PACKAGE_CONFIG['directories']['dist'],
                    os.path.basename(rpm_path)
                )
                shutil.move(rpm_path, package)

            elif format == 'appimage':
                # Cria estrutura
                app_dir = os.path.join(temp_dir, 'MegaEmu.AppDir')
                os.makedirs(app_dir)

                for dir_type, paths in files.items():
                    for path in paths:
                        if dir_type == 'bin':
                            dest = os.path.join(app_dir, 'usr/bin',
                                           os.path.basename(path))
                        elif dir_type == 'lib':
                            dest = os.path.join(app_dir, 'usr/lib',
                                           os.path.basename(path))
                        elif dir_type == 'share':
                            dest = os.path.join(app_dir, 'usr/share/mega-emu',
                                           os.path.relpath(path, 'share'))
                        os.makedirs(os.path.dirname(dest), exist_ok=True)
                        shutil.copy2(path, dest)

                # Copia desktop e ícone
                shutil.copy2(config['desktop'],
                         os.path.join(app_dir, 'mega-emu.desktop'))
                shutil.copy2('assets/icons/mega_emu.png',
                         os.path.join(app_dir, 'mega-emu.png'))

                # Cria AppRun
                apprun = os.path.join(app_dir, 'AppRun')
                with open(apprun, 'w') as f:
                    f.write('#!/bin/sh\n')
                    f.write('SELF=$(readlink -f "$0")\n')
                    f.write('HERE=${SELF%/*}\n')
                    f.write('export PATH="${HERE}/usr/bin:${PATH}"\n')
                    f.write('export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"\n')
                    f.write('exec "${HERE}/usr/bin/mega_emu" "$@"\n')
                os.chmod(apprun, 0o755)

                # Cria AppImage
                package = os.path.join(
                    PACKAGE_CONFIG['directories']['dist'],
                    f'MegaEmu-{PACKAGE_CONFIG["metadata"]["version"]}-{platform.machine()}.AppImage'
                )
                subprocess.run([
                    'appimagetool',
                    app_dir,
                    package
                ], check=True)

            print(f'Pacote criado: {package}')
            return package
        finally:
            shutil.rmtree(temp_dir)
    except Exception as e:
        print(f'Erro ao criar pacote Linux: {e}', file=sys.stderr)
        return None

def create_macos_package(files: Dict[str, List[str]], format: str) -> Optional[str]:
    """
    Cria pacote para macOS.

    Args:
        files: Dicionário com arquivos por tipo.
        format: Formato do pacote ('dmg' ou 'pkg').

    Returns:
        Caminho do pacote ou None se falhar.
    """
    try:
        config = PACKAGE_CONFIG['formats']['macos'][format]
        print(f'\nCriando pacote macOS {format}...')

        # Cria diretório temporário
        temp_dir = tempfile.mkdtemp(dir=PACKAGE_CONFIG['directories']['temp'])

        try:
            # Cria estrutura do app
            app_path = os.path.join(temp_dir, 'MegaEmu.app')
            contents = os.path.join(app_path, 'Contents')
            macos = os.path.join(contents, 'MacOS')
            resources = os.path.join(contents, 'Resources')
            frameworks = os.path.join(contents, 'Frameworks')

            os.makedirs(macos)
            os.makedirs(resources)
            os.makedirs(frameworks)

            # Copia arquivos
            for dir_type, paths in files.items():
                for path in paths:
                    if dir_type == 'bin':
                        dest = os.path.join(macos, os.path.basename(path))
                    elif dir_type == 'lib':
                        dest = os.path.join(frameworks, os.path.basename(path))
                    elif dir_type == 'share':
                        dest = os.path.join(resources,
                                       os.path.relpath(path, 'share'))
                    os.makedirs(os.path.dirname(dest), exist_ok=True)
                    shutil.copy2(path, dest)

            # Copia ícones
            shutil.copy2(config['icon'],
                      os.path.join(resources, 'MegaEmu.icns'))

            if format == 'dmg':
                # Cria DMG
                package = os.path.join(
                    PACKAGE_CONFIG['directories']['dist'],
                    f'MegaEmu-{PACKAGE_CONFIG["metadata"]["version"]}-{platform.machine()}.dmg'
                )

                subprocess.run([
                    'hdiutil',
                    'create',
                    '-volname', config['volume_name'],
                    '-srcfolder', temp_dir,
                    '-ov',
                    '-format', 'UDZO',
                    package
                ], check=True)

            elif format == 'pkg':
                # Cria PKG
                package = os.path.join(
                    PACKAGE_CONFIG['directories']['dist'],
                    f'MegaEmu-{PACKAGE_CONFIG["metadata"]["version"]}-{platform.machine()}.pkg'
                )

                subprocess.run([
                    'pkgbuild',
                    '--root', temp_dir,
                    '--identifier', config['identifier'],
                    '--version', PACKAGE_CONFIG['metadata']['version'],
                    '--install-location', '/Applications',
                    '--scripts', config['scripts'],
                    package
                ], check=True)

            print(f'Pacote criado: {package}')
            return package
        finally:
            shutil.rmtree(temp_dir)
    except Exception as e:
        print(f'Erro ao criar pacote macOS: {e}', file=sys.stderr)
        return None

def verify_package(package: str) -> bool:
    """
    Verifica integridade de um pacote.

    Args:
        package: Caminho do pacote.

    Returns:
        True se o pacote é válido, False caso contrário.
    """
    try:
        if not PACKAGE_CONFIG['verification']['enabled']:
            return True

        print(f'\nVerificando pacote {package}...')

        # Calcula hashes
        hashes = {}
        for algorithm in PACKAGE_CONFIG['verification']['algorithms']:
            hash_obj = hashlib.new(algorithm)
            with open(package, 'rb') as f:
                for chunk in iter(lambda: f.read(8192), b''):
                    hash_obj.update(chunk)
            hashes[algorithm] = hash_obj.hexdigest()

            # Salva hash
            hash_path = f'{package}.{algorithm}'
            with open(hash_path, 'w') as f:
                f.write(f'{hashes[algorithm]} {os.path.basename(package)}\n')

        # Gera assinatura
        if PACKAGE_CONFIG['verification']['signature']:
            from cryptography.hazmat.primitives import serialization
            from cryptography.hazmat.primitives.asymmetric import padding
            from cryptography.hazmat.primitives import hashes

            # Carrega chave privada
            with open('security/keys/private.pem', 'rb') as f:
                private_key = serialization.load_pem_private_key(
                    f.read(),
                    password=None
                )

            # Assina hash SHA-256
            signature = private_key.sign(
                hashes['sha256'].encode(),
                padding.PSS(
                    mgf=padding.MGF1(hashes.SHA256()),
                    salt_length=padding.PSS.MAX_LENGTH
                ),
                hashes.SHA256()
            )

            # Salva assinatura
            sig_path = f'{package}.sig'
            with open(sig_path, 'w') as f:
                f.write(base64.b64encode(signature).decode())

        print('Pacote verificado com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao verificar pacote: {e}', file=sys.stderr)
        return False

def setup_logging() -> bool:
    """
    Configura sistema de logging.

    Returns:
        True se a configuração foi bem sucedida, False caso contrário.
    """
    try:
        if not PACKAGE_CONFIG['logging']['enabled']:
            return True

        # Configura logger
        logger = logging.getLogger('package')
        logger.setLevel(PACKAGE_CONFIG['logging']['level'])

        # Handler para arquivo
        log_path = os.path.join(PACKAGE_CONFIG['directories']['logs'],
                             'package.log')
        handler = logging.handlers.TimedRotatingFileHandler(
            log_path,
            when=PACKAGE_CONFIG['logging']['rotation']['when'],
            interval=PACKAGE_CONFIG['logging']['rotation']['interval'],
            backupCount=PACKAGE_CONFIG['logging']['rotation']['backupCount']
        )
        handler.setFormatter(logging.Formatter(
            PACKAGE_CONFIG['logging']['format']
        ))
        logger.addHandler(handler)

        return True
    except Exception as e:
        print(f'Erro ao configurar logging: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_package.py <comando> [formato]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  create <formato>      Cria pacote', file=sys.stderr)
        print('\nFormatos disponíveis:', file=sys.stderr)
        system = platform.system().lower()
        if system in PACKAGE_CONFIG['formats']:
            for format in PACKAGE_CONFIG['formats'][system]:
                print(f'  {format}', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        if not create_directories():
            return 1
        if not setup_logging():
            return 1
        return 0

    elif command == 'create':
        if len(sys.argv) < 3:
            print('Erro: formato não especificado.', file=sys.stderr)
            return 1

        format = sys.argv[2]
        system = platform.system().lower()

        if system not in PACKAGE_CONFIG['formats']:
            print(f'Sistema não suportado: {system}', file=sys.stderr)
            return 1

        if format not in PACKAGE_CONFIG['formats'][system]:
            print(f'Formato não suportado: {format}', file=sys.stderr)
            return 1

        # Coleta arquivos
        files = collect_files()
        if not files:
            return 1

        # Cria pacote
        package = None
        if system == 'windows':
            if format == 'installer':
                package = create_windows_installer(files)
            elif format == 'portable':
                package = create_windows_portable(files)
        elif system == 'linux':
            package = create_linux_package(files, format)
        elif system == 'macos':
            package = create_macos_package(files, format)

        if not package:
            return 1

        # Verifica pacote
        if not verify_package(package):
            return 1

        return 0

    else:
        print('Comando inválido.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
