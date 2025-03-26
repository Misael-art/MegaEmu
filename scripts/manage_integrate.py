#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de integração do emulador.
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
import requests
import hashlib
import base64
import hmac
import logging
import logging.handlers
import winreg
import plistlib

# Configurações de integração
INTEGRATE_CONFIG = {
    'directories': {
        'integrate': 'integrate',
        'plugins': 'integrate/plugins',
        'scripts': 'integrate/scripts',
        'hooks': 'integrate/hooks',
        'logs': 'integrate/logs',
        'temp': 'integrate/temp'
    },
    'plugins': {
        'video': {
            'opengl': {
                'enabled': True,
                'version': '4.6',
                'extensions': [
                    'GL_ARB_compute_shader',
                    'GL_ARB_shader_storage_buffer_object'
                ]
            },
            'vulkan': {
                'enabled': False,
                'version': '1.3',
                'extensions': [
                    'VK_KHR_swapchain',
                    'VK_KHR_dynamic_rendering'
                ]
            },
            'directx': {
                'enabled': False,
                'version': '12',
                'features': [
                    'D3D12_FEATURE_D3D12_OPTIONS7',
                    'D3D12_FEATURE_SHADER_MODEL'
                ]
            }
        },
        'audio': {
            'openal': {
                'enabled': True,
                'version': '1.1',
                'extensions': [
                    'ALC_ENUMERATE_ALL_EXT',
                    'ALC_SOFT_HRTF'
                ]
            },
            'wasapi': {
                'enabled': False,
                'version': '1.0',
                'features': [
                    'IAudioClient3',
                    'IAudioRenderClient'
                ]
            }
        },
        'input': {
            'sdl': {
                'enabled': True,
                'version': '2.30.0',
                'features': [
                    'SDL_GAMECONTROLLER',
                    'SDL_HAPTIC'
                ]
            },
            'xinput': {
                'enabled': False,
                'version': '1.4',
                'features': [
                    'XINPUT_GAMEPAD',
                    'XINPUT_VIBRATION'
                ]
            }
        }
    },
    'scripts': {
        'pre_build': {
            'enabled': True,
            'scripts': [
                'check_deps.sh',
                'gen_version.sh'
            ]
        },
        'post_build': {
            'enabled': True,
            'scripts': [
                'copy_deps.sh',
                'gen_docs.sh'
            ]
        }
    },
    'hooks': {
        'git': {
            'enabled': True,
            'hooks': [
                'pre-commit',
                'pre-push'
            ]
        },
        'cmake': {
            'enabled': True,
            'hooks': [
                'configure',
                'build'
            ]
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
    },
    'file_associations': {
        'windows': {
            'extensions': ['.md', '.smd', '.gen', '.bin', '.sms', '.gg'],
            'icon': 'share/icons/mega-emu.ico',
            'description': 'Mega Emu ROM',
            'command': '"{app}" "%1"'
        },
        'linux': {
            'extensions': ['.md', '.smd', '.gen', '.bin', '.sms', '.gg'],
            'icon': 'share/icons/mega-emu.png',
            'mime_type': 'application/x-mega-emu-rom',
            'desktop_entry': {
                'name': 'Mega Emu',
                'comment': 'Mega Drive/Genesis Emulator',
                'exec': 'mega-emu %f',
                'icon': 'mega-emu',
                'type': 'Application',
                'categories': ['Game', 'Emulator']
            }
        },
        'macos': {
            'extensions': ['.md', '.smd', '.gen', '.bin', '.sms', '.gg'],
            'icon': 'share/icons/mega-emu.icns',
            'bundle_id': 'org.mega-emu.app',
            'role': 'Editor'
        }
    },
    'shell_integration': {
        'windows': {
            'context_menu': {
                'enabled': True,
                'items': [
                    {
                        'name': 'Abrir com Mega Emu',
                        'command': '"{app}" "%1"'
                    },
                    {
                        'name': 'Iniciar com Depuração',
                        'command': '"{app}" --debug "%1"'
                    }
                ]
            },
            'jump_list': {
                'enabled': True,
                'tasks': [
                    {
                        'name': 'Abrir ROM',
                        'command': '"{app}" --open'
                    },
                    {
                        'name': 'Configurações',
                        'command': '"{app}" --config'
                    }
                ]
            }
        },
        'linux': {
            'context_menu': {
                'enabled': True,
                'items': [
                    {
                        'name': 'Abrir com Mega Emu',
                        'command': 'mega-emu %f'
                    },
                    {
                        'name': 'Iniciar com Depuração',
                        'command': 'mega-emu --debug %f'
                    }
                ]
            }
        },
        'macos': {
            'services': {
                'enabled': True,
                'items': [
                    {
                        'name': 'Abrir com Mega Emu',
                        'command': 'open -a "Mega Emu" "$1"'
                    },
                    {
                        'name': 'Iniciar com Depuração',
                        'command': 'open -a "Mega Emu" --args --debug "$1"'
                    }
                ]
            }
        }
    },
    'protocol_handlers': {
        'enabled': True,
        'schemes': {
            'mega-emu': {
                'name': 'Mega Emu Protocol',
                'description': 'Protocolo para integração com o Mega Emu',
                'command': '"{app}" --url "%1"'
            }
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
        for directory in INTEGRATE_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def check_plugin(plugin: str, category: str) -> bool:
    """
    Verifica disponibilidade de um plugin.

    Args:
        plugin: Nome do plugin.
        category: Categoria do plugin.

    Returns:
        True se o plugin está disponível, False caso contrário.
    """
    try:
        config = INTEGRATE_CONFIG['plugins'][category][plugin]
        if not config['enabled']:
            print(f'\nPlugin {plugin} desabilitado.')
            return False

        print(f'\nVerificando plugin {plugin}...')

        # Verifica versão
        if plugin == 'opengl':
            # Verifica OpenGL
            import OpenGL.GL as gl
            version = gl.glGetString(gl.GL_VERSION).decode()
            if version < config['version']:
                print(f'Versão OpenGL insuficiente: {version}')
                return False

            # Verifica extensões
            for ext in config['extensions']:
                if not gl.glGetExtensionString(ext):
                    print(f'Extensão não suportada: {ext}')
                    return False

        elif plugin == 'vulkan':
            # Verifica Vulkan
            import vulkan as vk
            version = vk.vkEnumerateInstanceVersion()
            if version < vk.VK_MAKE_VERSION(*map(int, config['version'].split('.'))):
                print(f'Versão Vulkan insuficiente: {version}')
                return False

            # Verifica extensões
            extensions = vk.vkEnumerateInstanceExtensionProperties(None)
            supported = [ext.extensionName for ext in extensions]
            for ext in config['extensions']:
                if ext not in supported:
                    print(f'Extensão não suportada: {ext}')
                    return False

        elif plugin == 'openal':
            # Verifica OpenAL
            import openal
            device = openal.Device()
            context = openal.Context(device)
            version = context.get_version()
            if version < config['version']:
                print(f'Versão OpenAL insuficiente: {version}')
                return False

            # Verifica extensões
            for ext in config['extensions']:
                if not context.has_extension(ext):
                    print(f'Extensão não suportada: {ext}')
                    return False

        elif plugin == 'sdl':
            # Verifica SDL
            import sdl2
            version = f'{sdl2.SDL_MAJOR_VERSION}.{sdl2.SDL_MINOR_VERSION}.{sdl2.SDL_PATCHLEVEL}'
            if version < config['version']:
                print(f'Versão SDL insuficiente: {version}')
                return False

            # Verifica features
            for feature in config['features']:
                if not hasattr(sdl2, feature):
                    print(f'Feature não suportada: {feature}')
                    return False

        print('Plugin disponível.')
        return True
    except Exception as e:
        print(f'Erro ao verificar plugin: {e}', file=sys.stderr)
        return False

def run_script(script: str, phase: str) -> bool:
    """
    Executa um script de integração.

    Args:
        script: Nome do script.
        phase: Fase de execução.

    Returns:
        True se o script foi executado com sucesso, False caso contrário.
    """
    try:
        config = INTEGRATE_CONFIG['scripts'][phase]
        if not config['enabled']:
            print(f'\nScripts {phase} desabilitados.')
            return False

        script_path = os.path.join(INTEGRATE_CONFIG['directories']['scripts'],
                                script)
        if not os.path.exists(script_path):
            print(f'Script não encontrado: {script}')
            return False

        print(f'\nExecutando script {script}...')

        # Executa script
        result = subprocess.run(['bash', script_path],
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             text=True)

        if result.returncode != 0:
            print(f'Erro ao executar script: {result.stderr}')
            return False

        print('Script executado com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao executar script: {e}', file=sys.stderr)
        return False

def install_hook(hook: str, system: str) -> bool:
    """
    Instala um hook de integração.

    Args:
        hook: Nome do hook.
        system: Sistema de hooks.

    Returns:
        True se o hook foi instalado com sucesso, False caso contrário.
    """
    try:
        config = INTEGRATE_CONFIG['hooks'][system]
        if not config['enabled']:
            print(f'\nHooks {system} desabilitados.')
            return False

        hook_path = os.path.join(INTEGRATE_CONFIG['directories']['hooks'],
                              f'{system}/{hook}')
        if not os.path.exists(hook_path):
            print(f'Hook não encontrado: {hook}')
            return False

        print(f'\nInstalando hook {hook}...')

        if system == 'git':
            # Instala hook Git
            git_dir = subprocess.run(['git', 'rev-parse', '--git-dir'],
                                  stdout=subprocess.PIPE,
                                  text=True).stdout.strip()
            dest_path = os.path.join(git_dir, 'hooks', hook)
            shutil.copy2(hook_path, dest_path)
            os.chmod(dest_path, 0o755)

        elif system == 'cmake':
            # Instala hook CMake
            cmake_dir = '.cmake/hooks'
            os.makedirs(cmake_dir, exist_ok=True)
            dest_path = os.path.join(cmake_dir, hook)
            shutil.copy2(hook_path, dest_path)

        print('Hook instalado com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao instalar hook: {e}', file=sys.stderr)
        return False

def get_app_path() -> Optional[str]:
    """
    Obtém caminho do executável do emulador.

    Returns:
        Caminho do executável ou None em caso de erro.
    """
    try:
        if sys.platform.startswith('win'):
            # Procura no registro
            with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,
                             r'Software\Mega Emu') as key:
                return winreg.QueryValueEx(key, 'Path')[0]
        elif sys.platform.startswith('linux'):
            # Procura no PATH
            result = subprocess.run(
                ['which', 'mega-emu'],
                capture_output=True,
                text=True
            )
            if result.returncode == 0:
                return result.stdout.strip()
        elif sys.platform.startswith('darwin'):
            # Procura no Applications
            app_path = '/Applications/Mega Emu.app'
            if os.path.exists(app_path):
                return app_path

        # Procura no diretório atual
        if sys.platform.startswith('win'):
            exe_name = 'mega-emu.exe'
        else:
            exe_name = 'mega-emu'

        exe_path = os.path.join('bin', exe_name)
        if os.path.exists(exe_path):
            return os.path.abspath(exe_path)

        return None
    except Exception as e:
        print(f'Erro ao obter caminho do aplicativo: {e}', file=sys.stderr)
        return None

def setup_file_associations() -> bool:
    """
    Configura associações de arquivos.

    Returns:
        True se as associações foram configuradas com sucesso, False caso contrário.
    """
    try:
        print('\nConfigurando associações de arquivos...')

        # Obtém caminho do aplicativo
        app_path = get_app_path()
        if not app_path:
            print('Aplicativo não encontrado.')
            return False

        # Configura associações
        if sys.platform.startswith('win'):
            # Windows Registry
            config = INTEGRATE_CONFIG['file_associations']['windows']

            for ext in config['extensions']:
                # Registra extensão
                with winreg.CreateKey(winreg.HKEY_CLASSES_ROOT, ext) as key:
                    winreg.SetValue(key, '', winreg.REG_SZ, 'MegaEmu.ROM')

                # Registra tipo de arquivo
                with winreg.CreateKey(winreg.HKEY_CLASSES_ROOT,
                                   'MegaEmu.ROM') as key:
                    winreg.SetValue(key, '', winreg.REG_SZ, config['description'])
                    with winreg.CreateKey(key, 'DefaultIcon') as icon_key:
                        icon_path = os.path.join(
                            os.path.dirname(app_path),
                            config['icon']
                        )
                        winreg.SetValue(icon_key, '', winreg.REG_SZ, icon_path)
                    with winreg.CreateKey(key, r'shell\open\command') as cmd_key:
                        command = config['command'].format(app=app_path)
                        winreg.SetValue(cmd_key, '', winreg.REG_SZ, command)

        elif sys.platform.startswith('linux'):
            # Freedesktop.org
            config = INTEGRATE_CONFIG['file_associations']['linux']

            # Cria arquivo .desktop
            desktop_path = os.path.expanduser(
                '~/.local/share/applications/mega-emu.desktop'
            )
            os.makedirs(os.path.dirname(desktop_path), exist_ok=True)

            with open(desktop_path, 'w') as f:
                f.write('[Desktop Entry]\n')
                for key, value in config['desktop_entry'].items():
                    f.write(f'{key}={value}\n')
                f.write(f'MimeType={config["mime_type"]}\n')

            # Cria arquivo MIME type
            mime_path = os.path.expanduser(
                '~/.local/share/mime/packages/mega-emu.xml'
            )
            os.makedirs(os.path.dirname(mime_path), exist_ok=True)

            with open(mime_path, 'w') as f:
                f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
                f.write('<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">\n')
                f.write(f'  <mime-type type="{config["mime_type"]}">\n')
                f.write('    <comment>Mega Drive/Genesis ROM</comment>\n')
                for ext in config['extensions']:
                    f.write(f'    <glob pattern="*{ext}"/>\n')
                f.write('  </mime-type>\n')
                f.write('</mime-info>\n')

            # Atualiza banco de dados MIME
            subprocess.run(['update-mime-database',
                         os.path.expanduser('~/.local/share/mime')],
                        check=True)

            # Copia ícone
            icon_path = os.path.expanduser('~/.local/share/icons/hicolor')
            for size in ['16x16', '32x32', '48x48', '64x64', '128x128']:
                size_path = os.path.join(icon_path, size, 'apps')
                os.makedirs(size_path, exist_ok=True)
                shutil.copy2(
                    os.path.join(os.path.dirname(app_path), config['icon']),
                    os.path.join(size_path, 'mega-emu.png')
                )

            # Atualiza cache de ícones
            subprocess.run(['gtk-update-icon-cache', icon_path], check=True)

        elif sys.platform.startswith('darwin'):
            # macOS
            config = INTEGRATE_CONFIG['file_associations']['macos']

            # Atualiza Info.plist
            plist_path = os.path.join(app_path, 'Contents/Info.plist')
            if not os.path.exists(plist_path):
                print('Info.plist não encontrado.')
                return False

            with open(plist_path, 'rb') as f:
                plist = plistlib.load(f)

            # Adiciona tipos de documento
            if 'CFBundleDocumentTypes' not in plist:
                plist['CFBundleDocumentTypes'] = []

            doc_type = {
                'CFBundleTypeExtensions': config['extensions'],
                'CFBundleTypeIconFile': os.path.basename(config['icon']),
                'CFBundleTypeName': 'Mega Drive/Genesis ROM',
                'CFBundleTypeRole': config['role'],
                'LSHandlerRank': 'Owner'
            }

            if doc_type not in plist['CFBundleDocumentTypes']:
                plist['CFBundleDocumentTypes'].append(doc_type)

            with open(plist_path, 'wb') as f:
                plistlib.dump(plist, f)

        print('Associações de arquivos configuradas com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao configurar associações de arquivos: {e}', file=sys.stderr)
        return False

def setup_shell_integration() -> bool:
    """
    Configura integração com o shell.

    Returns:
        True se a integração foi configurada com sucesso, False caso contrário.
    """
    try:
        print('\nConfigurando integração com o shell...')

        # Obtém caminho do aplicativo
        app_path = get_app_path()
        if not app_path:
            print('Aplicativo não encontrado.')
            return False

        # Configura integração
        if sys.platform.startswith('win'):
            # Windows
            config = INTEGRATE_CONFIG['shell_integration']['windows']

            if config['context_menu']['enabled']:
                # Menu de contexto
                for item in config['context_menu']['items']:
                    with winreg.CreateKey(winreg.HKEY_CLASSES_ROOT,
                                      r'MegaEmu.ROM\shell') as key:
                        with winreg.CreateKey(key, item['name']) as item_key:
                            command = item['command'].format(app=app_path)
                            with winreg.CreateKey(item_key, 'command') as cmd_key:
                                winreg.SetValue(cmd_key, '', winreg.REG_SZ,
                                            command)

            if config['jump_list']['enabled']:
                # Jump List
                import win32com.client
                shell = win32com.client.Dispatch('WScript.Shell')
                startup = shell.SpecialFolders('Startup')

                for task in config['jump_list']['tasks']:
                    shortcut = shell.CreateShortCut(
                        os.path.join(startup, f'{task["name"]}.lnk')
                    )
                    shortcut.TargetPath = app_path
                    shortcut.Arguments = task['command'].format(app=app_path)
                    shortcut.Save()

        elif sys.platform.startswith('linux'):
            # Linux
            config = INTEGRATE_CONFIG['shell_integration']['linux']

            if config['context_menu']['enabled']:
                # Menu de contexto
                actions_path = os.path.expanduser(
                    '~/.local/share/file-manager/actions'
                )
                os.makedirs(actions_path, exist_ok=True)

                for item in config['context_menu']['items']:
                    action_path = os.path.join(
                        actions_path,
                        f'mega-emu-{item["name"].lower().replace(" ", "-")}.desktop'
                    )
                    with open(action_path, 'w') as f:
                        f.write('[Desktop Entry]\n')
                        f.write('Type=Action\n')
                        f.write(f'Name={item["name"]}\n')
                        f.write('Profiles=all;\n')
                        f.write('\n[X-Action-Profile all]\n')
                        f.write('MimeTypes=application/x-mega-emu-rom\n')
                        f.write(f'Exec={item["command"]}\n')

        elif sys.platform.startswith('darwin'):
            # macOS
            config = INTEGRATE_CONFIG['shell_integration']['macos']

            if config['services']['enabled']:
                # Serviços
                services_path = os.path.expanduser(
                    '~/Library/Services'
                )
                os.makedirs(services_path, exist_ok=True)

                for item in config['services']['items']:
                    service_path = os.path.join(
                        services_path,
                        f'{item["name"]}.workflow'
                    )
                    os.makedirs(service_path, exist_ok=True)

                    # Info.plist
                    info_plist = {
                        'NSServices': [{
                            'NSMenuItem': {'default': item['name']},
                            'NSMessage': 'runWorkflowAsService',
                            'NSRequiredFileTypes': ['public.data'],
                            'NSSendFileTypes': ['public.data']
                        }]
                    }

                    with open(os.path.join(service_path, 'Info.plist'), 'wb') as f:
                        plistlib.dump(info_plist, f)

                    # Script
                    script_path = os.path.join(service_path, 'script.sh')
                    with open(script_path, 'w') as f:
                        f.write('#!/bin/bash\n')
                        f.write(item['command'])
                    os.chmod(script_path, 0o755)

        print('Integração com o shell configurada com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao configurar integração com o shell: {e}', file=sys.stderr)
        return False

def setup_protocol_handlers() -> bool:
    """
    Configura handlers de protocolo.

    Returns:
        True se os handlers foram configurados com sucesso, False caso contrário.
    """
    try:
        if not INTEGRATE_CONFIG['protocol_handlers']['enabled']:
            return True

        print('\nConfigurando handlers de protocolo...')

        # Obtém caminho do aplicativo
        app_path = get_app_path()
        if not app_path:
            print('Aplicativo não encontrado.')
            return False

        # Configura handlers
        if sys.platform.startswith('win'):
            # Windows Registry
            for scheme, config in INTEGRATE_CONFIG['protocol_handlers']['schemes'].items():
                with winreg.CreateKey(winreg.HKEY_CLASSES_ROOT,
                                   scheme) as key:
                    winreg.SetValue(key, '', winreg.REG_SZ, config['name'])
                    winreg.SetValueEx(key, 'URL Protocol', 0, winreg.REG_SZ, '')
                    with winreg.CreateKey(key, r'shell\open\command') as cmd_key:
                        command = config['command'].format(app=app_path)
                        winreg.SetValue(cmd_key, '', winreg.REG_SZ, command)

        elif sys.platform.startswith('linux'):
            # Freedesktop.org
            desktop_path = os.path.expanduser(
                '~/.local/share/applications/mega-emu-url.desktop'
            )
            os.makedirs(os.path.dirname(desktop_path), exist_ok=True)

            with open(desktop_path, 'w') as f:
                f.write('[Desktop Entry]\n')
                f.write('Type=Application\n')
                f.write('Name=Mega Emu URL Handler\n')
                f.write('NoDisplay=true\n')
                for scheme in INTEGRATE_CONFIG['protocol_handlers']['schemes']:
                    f.write(f'MimeType=x-scheme-handler/{scheme};\n')
                f.write('Exec=mega-emu --url %u\n')

            # Atualiza banco de dados MIME
            subprocess.run([
                'xdg-mime', 'default', 'mega-emu-url.desktop',
                'x-scheme-handler/mega-emu'
            ], check=True)

        elif sys.platform.startswith('darwin'):
            # macOS
            plist_path = os.path.join(app_path, 'Contents/Info.plist')
            if not os.path.exists(plist_path):
                print('Info.plist não encontrado.')
                return False

            with open(plist_path, 'rb') as f:
                plist = plistlib.load(f)

            # Adiciona schemes
            if 'CFBundleURLTypes' not in plist:
                plist['CFBundleURLTypes'] = []

            for scheme, config in INTEGRATE_CONFIG['protocol_handlers']['schemes'].items():
                url_type = {
                    'CFBundleURLName': scheme,
                    'CFBundleURLSchemes': [scheme],
                    'CFBundleTypeRole': 'Editor'
                }

                if url_type not in plist['CFBundleURLTypes']:
                    plist['CFBundleURLTypes'].append(url_type)

            with open(plist_path, 'wb') as f:
                plistlib.dump(plist, f)

        print('Handlers de protocolo configurados com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao configurar handlers de protocolo: {e}', file=sys.stderr)
        return False

def setup_logging() -> bool:
    """
    Configura sistema de logging.

    Returns:
        True se a configuração foi bem sucedida, False caso contrário.
    """
    try:
        if not INTEGRATE_CONFIG['logging']['enabled']:
            return True

        # Configura logger
        logger = logging.getLogger('integrate')
        logger.setLevel(INTEGRATE_CONFIG['logging']['level'])

        # Handler para arquivo
        log_path = os.path.join(
            INTEGRATE_CONFIG['directories']['logs'],
            'integrate.log'
        )
        handler = logging.handlers.TimedRotatingFileHandler(
            log_path,
            when=INTEGRATE_CONFIG['logging']['rotation']['when'],
            interval=INTEGRATE_CONFIG['logging']['rotation']['interval'],
            backupCount=INTEGRATE_CONFIG['logging']['rotation']['backupCount']
        )
        handler.setFormatter(logging.Formatter(
            INTEGRATE_CONFIG['logging']['format']
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
        print('Uso: manage_integrate.py <comando>', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init      Cria estrutura de diretórios', file=sys.stderr)
        print('  files     Configura associações de arquivos', file=sys.stderr)
        print('  shell     Configura integração com o shell', file=sys.stderr)
        print('  protocol  Configura handlers de protocolo', file=sys.stderr)
        print('  all       Configura todas as integrações', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        if not create_directories():
            return 1
        if not setup_logging():
            return 1
        return 0

    elif command == 'files':
        return 0 if setup_file_associations() else 1

    elif command == 'shell':
        return 0 if setup_shell_integration() else 1

    elif command == 'protocol':
        return 0 if setup_protocol_handlers() else 1

    elif command == 'all':
        if not setup_file_associations():
            return 1
        if not setup_shell_integration():
            return 1
        if not setup_protocol_handlers():
            return 1
        return 0

    else:
        print('Comando inválido.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
