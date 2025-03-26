#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de instalação do emulador.
"""

import os
import sys
import json
import shutil
import logging
import logging.handlers
import subprocess
import platform
import winreg
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

# Configurações de instalação
INSTALL_CONFIG = {
    'directories': {
        'install': 'install',
        'temp': 'install/temp',
        'logs': 'install/logs'
    },
    'application': {
        'name': 'Mega Emu',
        'version': '1.0.0',
        'publisher': 'Mega Emu Team',
        'website': 'https://mega-emu.org',
        'description': 'Mega Drive/Genesis Emulator',
        'license': 'MIT',
        'icon': 'share/icons/mega-emu.ico'
    },
    'files': {
        'windows': {
            'bin': [
                'bin/mega-emu.exe',
                'bin/SDL2.dll',
                'bin/OpenAL32.dll'
            ],
            'share': [
                'share/shaders/*',
                'share/themes/*',
                'share/icons/*'
            ],
            'doc': [
                'README.md',
                'LICENSE'
            ]
        },
        'linux': {
            'bin': [
                'bin/mega-emu'
            ],
            'lib': [
                'lib/libSDL2-2.0.so.0',
                'lib/libopenal.so.1'
            ],
            'share': [
                'share/shaders/*',
                'share/themes/*',
                'share/icons/*'
            ],
            'doc': [
                'README.md',
                'LICENSE'
            ]
        },
        'macos': {
            'app': [
                'Mega Emu.app/**/*'
            ]
        }
    },
    'shortcuts': {
        'windows': {
            'desktop': True,
            'start_menu': True,
            'quick_launch': True
        },
        'linux': {
            'desktop': True,
            'applications': True
        },
        'macos': {
            'applications': True,
            'dock': True
        }
    },
    'registry': {
        'windows': {
            'uninstall': {
                'DisplayName': '{name}',
                'DisplayVersion': '{version}',
                'Publisher': '{publisher}',
                'URLInfoAbout': '{website}',
                'DisplayIcon': '{icon}',
                'UninstallString': '"{uninstaller}"',
                'InstallLocation': '{install_dir}',
                'NoModify': 1,
                'NoRepair': 1
            }
        }
    },
    'logging': {
        'enabled': True,
        'level': 'INFO',
        'format': '%(asctime)s [%(levelname)s] %(message)s',
        'rotation': {
            'when': 'D',
            'interval': 1,
            'backupCount': 30
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
        for directory in INSTALL_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def get_install_directory() -> Optional[str]:
    """
    Obtém diretório de instalação padrão.

    Returns:
        Caminho do diretório ou None em caso de erro.
    """
    try:
        if sys.platform.startswith('win'):
            # Windows: Program Files
            return os.path.join(
                os.environ.get('ProgramFiles', 'C:\\Program Files'),
                INSTALL_CONFIG['application']['name']
            )
        elif sys.platform.startswith('linux'):
            # Linux: /opt
            return os.path.join(
                '/opt',
                INSTALL_CONFIG['application']['name'].lower().replace(' ', '-')
            )
        elif sys.platform.startswith('darwin'):
            # macOS: Applications
            return '/Applications'
        else:
            print(f'Sistema não suportado: {sys.platform}', file=sys.stderr)
            return None
    except Exception as e:
        print(f'Erro ao obter diretório de instalação: {e}', file=sys.stderr)
        return None

def copy_files(install_dir: str) -> bool:
    """
    Copia arquivos para o diretório de instalação.

    Args:
        install_dir: Diretório de instalação.

    Returns:
        True se os arquivos foram copiados com sucesso, False caso contrário.
    """
    try:
        print('\nCopiando arquivos...')

        # Obtém configuração da plataforma
        if sys.platform.startswith('win'):
            platform_config = INSTALL_CONFIG['files']['windows']
        elif sys.platform.startswith('linux'):
            platform_config = INSTALL_CONFIG['files']['linux']
        elif sys.platform.startswith('darwin'):
            platform_config = INSTALL_CONFIG['files']['macos']
        else:
            print(f'Sistema não suportado: {sys.platform}', file=sys.stderr)
            return False

        # Copia arquivos
        for category, patterns in platform_config.items():
            for pattern in patterns:
                for src in Path('.').glob(pattern):
                    if src.is_file():
                        # Cria diretório de destino
                        dst = os.path.join(install_dir, category, src.name)
                        os.makedirs(os.path.dirname(dst), exist_ok=True)

                        # Copia arquivo
                        shutil.copy2(src, dst)
                        print(f'Copiado: {src} -> {dst}')

        return True
    except Exception as e:
        print(f'Erro ao copiar arquivos: {e}', file=sys.stderr)
        return False

def create_shortcuts(install_dir: str) -> bool:
    """
    Cria atalhos para o aplicativo.

    Args:
        install_dir: Diretório de instalação.

    Returns:
        True se os atalhos foram criados com sucesso, False caso contrário.
    """
    try:
        print('\nCriando atalhos...')

        if sys.platform.startswith('win'):
            # Windows
            config = INSTALL_CONFIG['shortcuts']['windows']
            import win32com.client

            shell = win32com.client.Dispatch('WScript.Shell')
            exe_path = os.path.join(install_dir, 'bin', 'mega-emu.exe')
            icon_path = os.path.join(install_dir, 'share', 'icons', 'mega-emu.ico')

            if config['desktop']:
                # Atalho na área de trabalho
                desktop = shell.SpecialFolders('Desktop')
                shortcut = shell.CreateShortCut(
                    os.path.join(desktop, 'Mega Emu.lnk')
                )
                shortcut.TargetPath = exe_path
                shortcut.IconLocation = icon_path
                shortcut.Save()

            if config['start_menu']:
                # Atalho no menu iniciar
                start_menu = shell.SpecialFolders('StartMenu')
                shortcut = shell.CreateShortCut(
                    os.path.join(start_menu, 'Programs', 'Mega Emu.lnk')
                )
                shortcut.TargetPath = exe_path
                shortcut.IconLocation = icon_path
                shortcut.Save()

            if config['quick_launch']:
                # Atalho na barra de tarefas
                quick_launch = os.path.join(
                    os.environ['APPDATA'],
                    'Microsoft\\Internet Explorer\\Quick Launch'
                )
                shortcut = shell.CreateShortCut(
                    os.path.join(quick_launch, 'Mega Emu.lnk')
                )
                shortcut.TargetPath = exe_path
                shortcut.IconLocation = icon_path
                shortcut.Save()

        elif sys.platform.startswith('linux'):
            # Linux
            config = INSTALL_CONFIG['shortcuts']['linux']

            if config['desktop']:
                # Atalho na área de trabalho
                desktop_path = os.path.expanduser('~/Desktop/mega-emu.desktop')
                with open(desktop_path, 'w') as f:
                    f.write('[Desktop Entry]\n')
                    f.write('Type=Application\n')
                    f.write('Name=Mega Emu\n')
                    f.write('Comment=Mega Drive/Genesis Emulator\n')
                    f.write(f'Exec={os.path.join(install_dir, "bin/mega-emu")}\n')
                    f.write(f'Icon={os.path.join(install_dir, "share/icons/mega-emu.png")}\n')
                    f.write('Terminal=false\n')
                    f.write('Categories=Game;Emulator;\n')
                os.chmod(desktop_path, 0o755)

            if config['applications']:
                # Atalho no menu de aplicativos
                applications_path = os.path.expanduser(
                    '~/.local/share/applications/mega-emu.desktop'
                )
                os.makedirs(os.path.dirname(applications_path), exist_ok=True)
                with open(applications_path, 'w') as f:
                    f.write('[Desktop Entry]\n')
                    f.write('Type=Application\n')
                    f.write('Name=Mega Emu\n')
                    f.write('Comment=Mega Drive/Genesis Emulator\n')
                    f.write(f'Exec={os.path.join(install_dir, "bin/mega-emu")}\n')
                    f.write(f'Icon={os.path.join(install_dir, "share/icons/mega-emu.png")}\n')
                    f.write('Terminal=false\n')
                    f.write('Categories=Game;Emulator;\n')
                os.chmod(applications_path, 0o755)

        elif sys.platform.startswith('darwin'):
            # macOS
            config = INSTALL_CONFIG['shortcuts']['macos']

            if config['applications']:
                # Copia para Applications
                app_path = os.path.join(install_dir, 'Mega Emu.app')
                if os.path.exists(app_path):
                    shutil.copytree(
                        app_path,
                        '/Applications/Mega Emu.app',
                        dirs_exist_ok=True
                    )

            if config['dock']:
                # Adiciona ao Dock
                subprocess.run([
                    'defaults', 'write',
                    'com.apple.dock', 'persistent-apps',
                    '-array-add',
                    f'<dict><key>tile-data</key><dict><key>file-data</key><dict><key>_CFURLString</key><string>/Applications/Mega Emu.app</string><key>_CFURLStringType</key><integer>0</integer></dict></dict></dict>'
                ], check=True)
                subprocess.run(['killall', 'Dock'], check=True)

        return True
    except Exception as e:
        print(f'Erro ao criar atalhos: {e}', file=sys.stderr)
        return False

def update_registry(install_dir: str) -> bool:
    """
    Atualiza registro do Windows.

    Args:
        install_dir: Diretório de instalação.

    Returns:
        True se o registro foi atualizado com sucesso, False caso contrário.
    """
    try:
        if not sys.platform.startswith('win'):
            return True

        print('\nAtualizando registro...')

        # Obtém configuração
        config = INSTALL_CONFIG['registry']['windows']['uninstall']
        app_config = INSTALL_CONFIG['application']

        # Formata valores
        values = {}
        for key, value in config.items():
            if isinstance(value, str):
                values[key] = value.format(
                    name=app_config['name'],
                    version=app_config['version'],
                    publisher=app_config['publisher'],
                    website=app_config['website'],
                    icon=os.path.join(install_dir, app_config['icon']),
                    uninstaller=os.path.join(install_dir, 'uninstall.exe'),
                    install_dir=install_dir
                )
            else:
                values[key] = value

        # Cria chave de desinstalação
        key_path = f'Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{app_config["name"]}'
        with winreg.CreateKey(winreg.HKEY_LOCAL_MACHINE, key_path) as key:
            for name, value in values.items():
                if isinstance(value, str):
                    winreg.SetValueEx(key, name, 0, winreg.REG_SZ, value)
                elif isinstance(value, int):
                    winreg.SetValueEx(key, name, 0, winreg.REG_DWORD, value)

        return True
    except Exception as e:
        print(f'Erro ao atualizar registro: {e}', file=sys.stderr)
        return False

def setup_logging() -> bool:
    """
    Configura sistema de logging.

    Returns:
        True se a configuração foi bem sucedida, False caso contrário.
    """
    try:
        if not INSTALL_CONFIG['logging']['enabled']:
            return True

        # Configura logger
        logger = logging.getLogger('install')
        logger.setLevel(INSTALL_CONFIG['logging']['level'])

        # Handler para arquivo
        log_path = os.path.join(
            INSTALL_CONFIG['directories']['logs'],
            'install.log'
        )
        handler = logging.handlers.TimedRotatingFileHandler(
            log_path,
            when=INSTALL_CONFIG['logging']['rotation']['when'],
            interval=INSTALL_CONFIG['logging']['rotation']['interval'],
            backupCount=INSTALL_CONFIG['logging']['rotation']['backupCount']
        )
        handler.setFormatter(logging.Formatter(
            INSTALL_CONFIG['logging']['format']
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
        print('Uso: manage_install.py <comando> [diretório]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  install [diretório]   Instala aplicativo', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        if not create_directories():
            return 1
        if not setup_logging():
            return 1
        return 0

    elif command == 'install':
        # Obtém diretório de instalação
        if len(sys.argv) > 2:
            install_dir = sys.argv[2]
        else:
            install_dir = get_install_directory()
            if not install_dir:
                return 1

        # Cria diretório de instalação
        try:
            os.makedirs(install_dir, exist_ok=True)
        except Exception as e:
            print(f'Erro ao criar diretório de instalação: {e}', file=sys.stderr)
            return 1

        # Copia arquivos
        if not copy_files(install_dir):
            return 1

        # Cria atalhos
        if not create_shortcuts(install_dir):
            return 1

        # Atualiza registro
        if not update_registry(install_dir):
            return 1

        print(f'\nInstalação concluída em: {install_dir}')
        return 0

    else:
        print('Comando inválido.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
