#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar os recursos de segurança do emulador.
"""

import os
import sys
import json
import shutil
import logging
import logging.handlers
import subprocess
import hashlib
import hmac
import secrets
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

# Configurações de segurança
SECURITY_CONFIG = {
    'directories': {
        'security': 'security',
        'keys': 'security/keys',
        'hashes': 'security/hashes',
        'signatures': 'security/signatures',
        'logs': 'security/logs'
    },
    'keys': {
        'rsa': {
            'size': 4096,
            'format': 'PEM',
            'public': 'rsa_public.pem',
            'private': 'rsa_private.pem'
        },
        'aes': {
            'size': 256,
            'mode': 'GCM',
            'file': 'aes.key'
        }
    },
    'hashes': {
        'algorithms': [
            'sha256',
            'sha512'
        ],
        'extensions': [
            '.exe',
            '.dll',
            '.so',
            '.dylib',
            '.rom'
        ]
    },
    'signatures': {
        'algorithm': 'rsa-pss-sha512',
        'salt_length': 64,
        'extensions': [
            '.exe',
            '.dll',
            '.so',
            '.dylib',
            '.rom'
        ]
    },
    'verification': {
        'check_hashes': True,
        'check_signatures': True,
        'enforce_secure_boot': True,
        'allow_unknown_roms': False
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
        for directory in SECURITY_CONFIG['directories'].values():
            os.makedirs(directory, exist_ok=True)
        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def generate_keys() -> bool:
    """
    Gera chaves criptográficas.

    Returns:
        True se as chaves foram geradas com sucesso, False caso contrário.
    """
    try:
        print('\nGerando chaves...')

        # Gera par de chaves RSA
        from cryptography.hazmat.primitives import serialization
        from cryptography.hazmat.primitives.asymmetric import rsa
        from cryptography.hazmat.backends import default_backend

        # Gera chave privada RSA
        private_key = rsa.generate_private_key(
            public_exponent=65537,
            key_size=SECURITY_CONFIG['keys']['rsa']['size'],
            backend=default_backend()
        )

        # Serializa chave privada
        private_pem = private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption()
        )

        # Obtém e serializa chave pública
        public_key = private_key.public_key()
        public_pem = public_key.public_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PublicFormat.SubjectPublicKeyInfo
        )

        # Salva chaves RSA
        private_path = os.path.join(
            SECURITY_CONFIG['directories']['keys'],
            SECURITY_CONFIG['keys']['rsa']['private']
        )
        public_path = os.path.join(
            SECURITY_CONFIG['directories']['keys'],
            SECURITY_CONFIG['keys']['rsa']['public']
        )

        with open(private_path, 'wb') as f:
            f.write(private_pem)
        os.chmod(private_path, 0o600)

        with open(public_path, 'wb') as f:
            f.write(public_pem)
        os.chmod(public_path, 0o644)

        # Gera chave AES
        aes_key = secrets.token_bytes(
            SECURITY_CONFIG['keys']['aes']['size'] // 8
        )

        # Salva chave AES
        aes_path = os.path.join(
            SECURITY_CONFIG['directories']['keys'],
            SECURITY_CONFIG['keys']['aes']['file']
        )
        with open(aes_path, 'wb') as f:
            f.write(aes_key)
        os.chmod(aes_path, 0o600)

        print('Chaves geradas com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao gerar chaves: {e}', file=sys.stderr)
        return False

def calculate_hashes(path: str) -> Dict[str, str]:
    """
    Calcula hashes de um arquivo.

    Args:
        path: Caminho do arquivo.

    Returns:
        Dicionário com algoritmos e hashes calculados.
    """
    try:
        hashes = {}
        for algorithm in SECURITY_CONFIG['hashes']['algorithms']:
            hash_obj = hashlib.new(algorithm)
            with open(path, 'rb') as f:
                for chunk in iter(lambda: f.read(8192), b''):
                    hash_obj.update(chunk)
            hashes[algorithm] = hash_obj.hexdigest()
        return hashes
    except Exception as e:
        print(f'Erro ao calcular hashes: {e}', file=sys.stderr)
        return {}

def sign_file(path: str) -> Optional[bytes]:
    """
    Assina um arquivo.

    Args:
        path: Caminho do arquivo.

    Returns:
        Assinatura do arquivo ou None em caso de erro.
    """
    try:
        from cryptography.hazmat.primitives import hashes, serialization
        from cryptography.hazmat.primitives.asymmetric import padding
        from cryptography.hazmat.backends import default_backend

        # Carrega chave privada
        private_path = os.path.join(
            SECURITY_CONFIG['directories']['keys'],
            SECURITY_CONFIG['keys']['rsa']['private']
        )
        with open(private_path, 'rb') as f:
            private_key = serialization.load_pem_private_key(
                f.read(),
                password=None,
                backend=default_backend()
            )

        # Calcula hash do arquivo
        hasher = hashes.Hash(hashes.SHA512(), backend=default_backend())
        with open(path, 'rb') as f:
            for chunk in iter(lambda: f.read(8192), b''):
                hasher.update(chunk)
        digest = hasher.finalize()

        # Assina hash
        signature = private_key.sign(
            digest,
            padding.PSS(
                mgf=padding.MGF1(hashes.SHA512()),
                salt_length=SECURITY_CONFIG['signatures']['salt_length']
            ),
            hashes.SHA512()
        )

        return signature
    except Exception as e:
        print(f'Erro ao assinar arquivo: {e}', file=sys.stderr)
        return None

def verify_file(path: str) -> bool:
    """
    Verifica integridade e autenticidade de um arquivo.

    Args:
        path: Caminho do arquivo.

    Returns:
        True se o arquivo é válido, False caso contrário.
    """
    try:
        # Verifica extensão
        if not any(path.endswith(ext) for ext in SECURITY_CONFIG['hashes']['extensions']):
            return True

        print(f'\nVerificando arquivo: {path}')

        # Verifica hashes
        if SECURITY_CONFIG['verification']['check_hashes']:
            # Carrega hashes salvos
            hash_path = os.path.join(
                SECURITY_CONFIG['directories']['hashes'],
                os.path.basename(path) + '.json'
            )
            if os.path.exists(hash_path):
                with open(hash_path) as f:
                    saved_hashes = json.load(f)

                # Calcula hashes atuais
                current_hashes = calculate_hashes(path)

                # Compara hashes
                for algorithm in SECURITY_CONFIG['hashes']['algorithms']:
                    if algorithm not in saved_hashes:
                        print(f'Hash {algorithm} não encontrado.')
                        return False
                    if saved_hashes[algorithm] != current_hashes[algorithm]:
                        print(f'Hash {algorithm} inválido.')
                        return False
            else:
                print('Arquivo de hashes não encontrado.')
                return False

        # Verifica assinatura
        if SECURITY_CONFIG['verification']['check_signatures']:
            from cryptography.hazmat.primitives import hashes, serialization
            from cryptography.hazmat.primitives.asymmetric import padding
            from cryptography.hazmat.backends import default_backend

            # Carrega chave pública
            public_path = os.path.join(
                SECURITY_CONFIG['directories']['keys'],
                SECURITY_CONFIG['keys']['rsa']['public']
            )
            with open(public_path, 'rb') as f:
                public_key = serialization.load_pem_public_key(
                    f.read(),
                    backend=default_backend()
                )

            # Carrega assinatura
            sig_path = os.path.join(
                SECURITY_CONFIG['directories']['signatures'],
                os.path.basename(path) + '.sig'
            )
            if os.path.exists(sig_path):
                with open(sig_path, 'rb') as f:
                    signature = f.read()

                # Calcula hash do arquivo
                hasher = hashes.Hash(hashes.SHA512(), backend=default_backend())
                with open(path, 'rb') as f:
                    for chunk in iter(lambda: f.read(8192), b''):
                        hasher.update(chunk)
                digest = hasher.finalize()

                # Verifica assinatura
                try:
                    public_key.verify(
                        signature,
                        digest,
                        padding.PSS(
                            mgf=padding.MGF1(hashes.SHA512()),
                            salt_length=SECURITY_CONFIG['signatures']['salt_length']
                        ),
                        hashes.SHA512()
                    )
                except:
                    print('Assinatura inválida.')
                    return False
            else:
                print('Arquivo de assinatura não encontrado.')
                return False

        print('Arquivo verificado com sucesso.')
        return True
    except Exception as e:
        print(f'Erro ao verificar arquivo: {e}', file=sys.stderr)
        return False

def setup_logging() -> bool:
    """
    Configura sistema de logging.

    Returns:
        True se a configuração foi bem sucedida, False caso contrário.
    """
    try:
        if not SECURITY_CONFIG['logging']['enabled']:
            return True

        # Configura logger
        logger = logging.getLogger('security')
        logger.setLevel(SECURITY_CONFIG['logging']['level'])

        # Handler para arquivo
        log_path = os.path.join(
            SECURITY_CONFIG['directories']['logs'],
            'security.log'
        )
        handler = logging.handlers.TimedRotatingFileHandler(
            log_path,
            when=SECURITY_CONFIG['logging']['rotation']['when'],
            interval=SECURITY_CONFIG['logging']['rotation']['interval'],
            backupCount=SECURITY_CONFIG['logging']['rotation']['backupCount']
        )
        handler.setFormatter(logging.Formatter(
            SECURITY_CONFIG['logging']['format']
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
        print('Uso: manage_security.py <comando> [arquivo]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init      Cria estrutura de diretórios', file=sys.stderr)
        print('  keys      Gera chaves criptográficas', file=sys.stderr)
        print('  hash      Calcula hashes de um arquivo', file=sys.stderr)
        print('  sign      Assina um arquivo', file=sys.stderr)
        print('  verify    Verifica um arquivo', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        if not create_directories():
            return 1
        if not setup_logging():
            return 1
        return 0

    elif command == 'keys':
        if not generate_keys():
            return 1
        return 0

    elif command in ['hash', 'sign', 'verify']:
        if len(sys.argv) < 3:
            print('Arquivo não especificado.', file=sys.stderr)
            return 1

        path = sys.argv[2]
        if not os.path.isfile(path):
            print('Arquivo não encontrado.', file=sys.stderr)
            return 1

        if command == 'hash':
            # Calcula e salva hashes
            hashes = calculate_hashes(path)
            if not hashes:
                return 1

            hash_path = os.path.join(
                SECURITY_CONFIG['directories']['hashes'],
                os.path.basename(path) + '.json'
            )
            try:
                with open(hash_path, 'w') as f:
                    json.dump(hashes, f, indent=2)
                print(f'Hashes salvos em: {hash_path}')
                return 0
            except Exception as e:
                print(f'Erro ao salvar hashes: {e}', file=sys.stderr)
                return 1

        elif command == 'sign':
            # Assina e salva assinatura
            signature = sign_file(path)
            if not signature:
                return 1

            sig_path = os.path.join(
                SECURITY_CONFIG['directories']['signatures'],
                os.path.basename(path) + '.sig'
            )
            try:
                with open(sig_path, 'wb') as f:
                    f.write(signature)
                print(f'Assinatura salva em: {sig_path}')
                return 0
            except Exception as e:
                print(f'Erro ao salvar assinatura: {e}', file=sys.stderr)
                return 1

        elif command == 'verify':
            # Verifica arquivo
            return 0 if verify_file(path) else 1

    else:
        print('Comando inválido.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
