#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os estados salvos dos jogos.
"""

import json
import os
import shutil
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple

def create_save_directories() -> bool:
    """
    Cria a estrutura de diretórios para estados salvos.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        # Cria diretórios principais
        dirs = [
            'saves',
            'saves/states',  # Estados salvos manualmente
            'saves/auto',    # Estados salvos automaticamente
            'saves/backup',  # Backups de estados salvos
            'saves/sram'     # Dados de SRAM (saves internos dos jogos)
        ]
        for directory in dirs:
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def get_save_info(save_path: str) -> Optional[Dict]:
    """
    Obtém informações sobre um estado salvo.

    Args:
        save_path: Caminho do arquivo de estado salvo.

    Returns:
        Dicionário com informações do estado salvo ou None se inválido.
    """
    try:
        if not os.path.exists(save_path):
            return None

        # Obtém informações básicas do arquivo
        stat = os.stat(save_path)
        file_name = os.path.basename(save_path)
        save_type = ('auto' if '/auto/' in save_path
                    else 'backup' if '/backup/' in save_path
                    else 'sram' if '/sram/' in save_path
                    else 'manual')

        # Extrai informações do nome do arquivo
        # Formato: rom_name-YYYYMMDD-HHMMSS[-description].state
        parts = file_name.split('-')
        if len(parts) < 3:
            return None

        rom_name = parts[0]
        try:
            timestamp = datetime.strptime(f'{parts[1]}-{parts[2].split(".")[0]}',
                                        '%Y%m%d-%H%M%S')
        except ValueError:
            return None

        description = '-'.join(parts[3:-1]) if len(parts) > 3 else None

        return {
            'path': save_path,
            'rom_name': rom_name,
            'type': save_type,
            'timestamp': timestamp.isoformat(),
            'description': description,
            'size': stat.st_size,
            'modified': datetime.fromtimestamp(stat.st_mtime).isoformat()
        }
    except Exception as e:
        print(f'Erro ao obter informações do estado salvo: {e}', file=sys.stderr)
        return None

def list_saves(rom_name: Optional[str] = None,
              save_type: Optional[str] = None) -> List[Dict]:
    """
    Lista estados salvos.

    Args:
        rom_name: Nome da ROM para filtrar (opcional).
        save_type: Tipo de save para filtrar (opcional).

    Returns:
        Lista de dicionários com informações dos estados salvos.
    """
    saves = []
    try:
        # Define diretórios a serem pesquisados
        dirs = []
        if not save_type or save_type == 'manual':
            dirs.append('saves/states')
        if not save_type or save_type == 'auto':
            dirs.append('saves/auto')
        if not save_type or save_type == 'backup':
            dirs.append('saves/backup')
        if not save_type or save_type == 'sram':
            dirs.append('saves/sram')

        # Busca estados salvos
        for directory in dirs:
            if not os.path.exists(directory):
                continue

            for file_name in os.listdir(directory):
                if not file_name.endswith(('.state', '.srm')):
                    continue

                if rom_name and not file_name.startswith(f'{rom_name}-'):
                    continue

                save_path = os.path.join(directory, file_name)
                save_info = get_save_info(save_path)
                if save_info:
                    saves.append(save_info)

        # Ordena por timestamp decrescente
        saves.sort(key=lambda x: x['timestamp'], reverse=True)
        return saves
    except Exception as e:
        print(f'Erro ao listar estados salvos: {e}', file=sys.stderr)
        return []

def create_save(rom_name: str, source_path: str, save_type: str = 'manual',
               description: Optional[str] = None) -> Optional[str]:
    """
    Cria um novo estado salvo.

    Args:
        rom_name: Nome da ROM.
        source_path: Caminho do arquivo de origem.
        save_type: Tipo do save (manual, auto, backup, sram).
        description: Descrição opcional do estado salvo.

    Returns:
        Caminho do estado salvo criado ou None se falhou.
    """
    try:
        if not os.path.exists(source_path):
            print(f'Arquivo de origem não encontrado: {source_path}', file=sys.stderr)
            return None

        # Define diretório de destino
        if save_type == 'auto':
            save_dir = 'saves/auto'
        elif save_type == 'backup':
            save_dir = 'saves/backup'
        elif save_type == 'sram':
            save_dir = 'saves/sram'
        else:
            save_dir = 'saves/states'

        # Cria nome do arquivo
        timestamp = datetime.now().strftime('%Y%m%d-%H%M%S')
        extension = '.srm' if save_type == 'sram' else '.state'
        file_name = f'{rom_name}-{timestamp}'
        if description:
            file_name += f'-{description}'
        file_name += extension

        # Copia o arquivo
        save_path = os.path.join(save_dir, file_name)
        shutil.copy2(source_path, save_path)

        print(f'Estado salvo criado: {file_name}')
        return save_path
    except Exception as e:
        print(f'Erro ao criar estado salvo: {e}', file=sys.stderr)
        return None

def delete_save(save_path: str, backup: bool = True) -> bool:
    """
    Remove um estado salvo.

    Args:
        save_path: Caminho do estado salvo.
        backup: Se True, move o arquivo para o diretório de backup.

    Returns:
        True se o estado salvo foi removido com sucesso, False caso contrário.
    """
    try:
        if not os.path.exists(save_path):
            print(f'Estado salvo não encontrado: {save_path}', file=sys.stderr)
            return False

        if backup:
            # Move para o diretório de backup
            file_name = os.path.basename(save_path)
            backup_path = os.path.join('saves/backup', file_name)
            shutil.move(save_path, backup_path)
            print(f'Estado salvo movido para backup: {file_name}')
        else:
            # Remove o arquivo
            os.remove(save_path)
            print(f'Estado salvo removido: {os.path.basename(save_path)}')

        return True
    except Exception as e:
        print(f'Erro ao remover estado salvo: {e}', file=sys.stderr)
        return False

def cleanup_auto_saves(rom_name: str, max_saves: int = 10) -> bool:
    """
    Limpa estados salvos automáticos antigos.

    Args:
        rom_name: Nome da ROM.
        max_saves: Número máximo de saves automáticos a manter.

    Returns:
        True se a limpeza foi bem sucedida, False caso contrário.
    """
    try:
        # Lista saves automáticos da ROM
        auto_saves = list_saves(rom_name, 'auto')
        if len(auto_saves) <= max_saves:
            return True

        # Remove saves excedentes (mais antigos)
        for save in auto_saves[max_saves:]:
            delete_save(save['path'], backup=False)

        return True
    except Exception as e:
        print(f'Erro ao limpar estados salvos automáticos: {e}', file=sys.stderr)
        return False

def export_save(save_path: str, output_path: str) -> bool:
    """
    Exporta um estado salvo.

    Args:
        save_path: Caminho do estado salvo.
        output_path: Caminho de destino.

    Returns:
        True se o estado salvo foi exportado com sucesso, False caso contrário.
    """
    try:
        if not os.path.exists(save_path):
            print(f'Estado salvo não encontrado: {save_path}', file=sys.stderr)
            return False

        # Cria diretório de destino se necessário
        os.makedirs(os.path.dirname(output_path), exist_ok=True)

        # Copia o arquivo
        shutil.copy2(save_path, output_path)
        print(f'Estado salvo exportado para: {output_path}')
        return True
    except Exception as e:
        print(f'Erro ao exportar estado salvo: {e}', file=sys.stderr)
        return False

def import_save(input_path: str, rom_name: str, save_type: str = 'manual',
               description: Optional[str] = None) -> Optional[str]:
    """
    Importa um estado salvo.

    Args:
        input_path: Caminho do arquivo a ser importado.
        rom_name: Nome da ROM.
        save_type: Tipo do save (manual, auto, backup, sram).
        description: Descrição opcional do estado salvo.

    Returns:
        Caminho do estado salvo importado ou None se falhou.
    """
    return create_save(rom_name, input_path, save_type, description)

def print_save_info(save_info: Dict) -> None:
    """
    Imprime informações de um estado salvo.

    Args:
        save_info: Dicionário com informações do estado salvo.
    """
    print(f'\nEstado Salvo: {os.path.basename(save_info["path"])}')
    print(f'  ROM: {save_info["rom_name"]}')
    print(f'  Tipo: {save_info["type"]}')
    print(f'  Data: {save_info["timestamp"]}')
    if save_info["description"]:
        print(f'  Descrição: {save_info["description"]}')
    print(f'  Tamanho: {save_info["size"]} bytes')
    print(f'  Modificado: {save_info["modified"]}')

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_saves.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  list [rom] [tipo]     Lista estados salvos', file=sys.stderr)
        print('  create <rom> <arquivo> [tipo] [descrição]')
        print('                        Cria um novo estado salvo', file=sys.stderr)
        print('  delete <arquivo> [--no-backup]')
        print('                        Remove um estado salvo', file=sys.stderr)
        print('  cleanup <rom> [max]   Limpa estados salvos automáticos', file=sys.stderr)
        print('  export <arquivo> <destino>')
        print('                        Exporta um estado salvo', file=sys.stderr)
        print('  import <arquivo> <rom> [tipo] [descrição]')
        print('                        Importa um estado salvo', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_save_directories() else 1

    elif command == 'list':
        rom_name = sys.argv[2] if len(sys.argv) > 2 else None
        save_type = sys.argv[3] if len(sys.argv) > 3 else None
        saves = list_saves(rom_name, save_type)
        for save in saves:
            print_save_info(save)
        return 0

    elif command == 'create' and len(sys.argv) > 3:
        rom_name = sys.argv[2]
        source_path = sys.argv[3]
        save_type = sys.argv[4] if len(sys.argv) > 4 else 'manual'
        description = sys.argv[5] if len(sys.argv) > 5 else None
        return 0 if create_save(rom_name, source_path, save_type, description) else 1

    elif command == 'delete' and len(sys.argv) > 2:
        save_path = sys.argv[2]
        backup = '--no-backup' not in sys.argv
        return 0 if delete_save(save_path, backup) else 1

    elif command == 'cleanup' and len(sys.argv) > 2:
        rom_name = sys.argv[2]
        max_saves = int(sys.argv[3]) if len(sys.argv) > 3 else 10
        return 0 if cleanup_auto_saves(rom_name, max_saves) else 1

    elif command == 'export' and len(sys.argv) > 3:
        save_path = sys.argv[2]
        output_path = sys.argv[3]
        return 0 if export_save(save_path, output_path) else 1

    elif command == 'import' and len(sys.argv) > 3:
        input_path = sys.argv[2]
        rom_name = sys.argv[3]
        save_type = sys.argv[4] if len(sys.argv) > 4 else 'manual'
        description = sys.argv[5] if len(sys.argv) > 5 else None
        return 0 if import_save(input_path, rom_name, save_type, description) else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
