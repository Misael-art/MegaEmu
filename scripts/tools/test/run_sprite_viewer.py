#!/usr/bin/env python3
import sys
import os
from pathlib import Path

# Adiciona o diretório src ao PYTHONPATH
src_path = Path(__file__).parent.parent / "src"
sys.path.append(str(src_path))

from tools.sprite_viewer import main

if __name__ == "__main__":
    main()