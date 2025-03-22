#!/usr/bin/env python3

import os
import re
import yaml
from datetime import datetime

class DocVersioner:
    def __init__(self):
        self.version_file = "docs/VERSION.md"
        self.history_file = "docs/version_history.yaml"
    
    def update_version(self, version_type="patch"):
        with open(self.version_file, 'r') as f:
            content = f.read()
            current = re.search(r'Versão atual: (\d+\.\d+\.\d+)', content).group(1)
            major, minor, patch = map(int, current.split('.'))
            
            if version_type == "major":
                major += 1
                minor = patch = 0
            elif version_type == "minor":
                minor += 1
                patch = 0
            else:
                patch += 1
                
            new_version = f"{major}.{minor}.{patch}"
            self._log_version(new_version)
            return new_version
    
    def _log_version(self, version):
        history = self._load_history()
        history['versions'].append({
            'version': version,
            'date': datetime.now().isoformat(),
            'changes': self._get_changes()
        })
        self._save_history(history)

    def _get_changes(self):
        # Implementar lógica para detectar mudanças
        return []

if __name__ == '__main__':
    versioner = DocVersioner()
    new_version = versioner.update_version()
    print(f"Documentation updated to version {new_version}")