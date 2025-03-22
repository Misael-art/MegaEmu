#!/usr/bin/env python3

import os
import re
import requests
from pathlib import Path

def validate_internal_links(root_dir):
    """Valida links internos na documentação"""
    broken_links = []
    for md_file in Path(root_dir).rglob('*.md'):
        with open(md_file, 'r') as f:
            content = f.read()
            links = re.findall(r'\[([^\]]+)\]\(([^)]+)\)', content)
            for text, link in links:
                if not link.startswith(('http://', 'https://')):
                    full_path = os.path.join(os.path.dirname(md_file), link)
                    if not os.path.exists(full_path):
                        broken_links.append((md_file, link))
    return broken_links

def validate_external_links(root_dir):
    """Valida links externos na documentação"""
    broken_links = []
    for md_file in Path(root_dir).rglob('*.md'):
        with open(md_file, 'r') as f:
            content = f.read()
            links = re.findall(r'\[([^\]]+)\]\((https?://[^)]+)\)', content)
            for text, link in links:
                try:
                    response = requests.head(link, timeout=5)
                    if response.status_code >= 400:
                        broken_links.append((md_file, link))
                except:
                    broken_links.append((md_file, link))
    return broken_links

if __name__ == '__main__':
    docs_dir = 'docs'
    internal_broken = validate_internal_links(docs_dir)
    external_broken = validate_external_links(docs_dir)
    
    print("Relatório de Validação de Links")
    print("==============================")
    print(f"Links internos quebrados: {len(internal_broken)}")
    print(f"Links externos quebrados: {len(external_broken)}")