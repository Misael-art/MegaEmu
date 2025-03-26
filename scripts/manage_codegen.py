#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar a geração de código.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import re
from jinja2 import Environment, FileSystemLoader

# Configurações de geração de código
CODEGEN_CONFIG = {
    'templates': {
        'class': {
            'header': 'templates/class.hpp.j2',
            'source': 'templates/class.cpp.j2'
        },
        'interface': {
            'header': 'templates/interface.hpp.j2'
        },
        'enum': {
            'header': 'templates/enum.hpp.j2'
        },
        'test': {
            'source': 'templates/test.cpp.j2'
        }
    },
    'namespaces': {
        'core': [
            'memory',
            'system',
            'config'
        ],
        'hw': [
            'cpu',
            'vdp',
            'psg',
            'io'
        ],
        'ui': [
            'window',
            'renderer',
            'input',
            'menu'
        ],
        'util': [
            'log',
            'file',
            'time'
        ]
    },
    'license': """
/*
 * Copyright (c) 2024 Your Name
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
""".strip()
}

def create_directories() -> bool:
    """
    Cria a estrutura de diretórios para templates.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        # Cria diretório de templates
        os.makedirs('templates', exist_ok=True)

        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def create_templates() -> bool:
    """
    Cria arquivos de template.

    Returns:
        True se os templates foram criados com sucesso, False caso contrário.
    """
    try:
        # Template de header de classe
        class_header = """
#pragma once

{{ license }}

#include <string>
#include <memory>
#include <vector>

namespace {{ namespace }} {

/**
 * @brief {{ description }}
 */
class {{ name }} {
public:
    /**
     * @brief Construtor.
     */
    {{ name }}();

    /**
     * @brief Destrutor.
     */
    virtual ~{{ name }}();

    {% for method in methods %}
    /**
     * @brief {{ method.description }}
     {% for param in method.params %}
     * @param {{ param.name }} {{ param.description }}
     {% endfor %}
     {% if method.return %}
     * @return {{ method.return.description }}
     {% endif %}
     */
    {{ method.type }} {{ method.name }}(
        {% for param in method.params %}
        {{ param.type }} {{ param.name }}{% if not loop.last %},{% endif %}
        {% endfor %}
    );

    {% endfor %}
private:
    {% for member in members %}
    {{ member.type }} {{ member.name }}; ///< {{ member.description }}
    {% endfor %}
};

} // namespace {{ namespace }}
""".strip()

        # Template de source de classe
        class_source = """
{{ license }}

#include "{{ header }}"

namespace {{ namespace }} {

{{ name }}::{{ name }}() {
    // TODO: Implementar construtor
}

{{ name }}::~{{ name }}() {
    // TODO: Implementar destrutor
}

{% for method in methods %}
{{ method.type }} {{ name }}::{{ method.name }}(
    {% for param in method.params %}
    {{ param.type }} {{ param.name }}{% if not loop.last %},{% endif %}
    {% endfor %}
) {
    // TODO: Implementar método
    {% if method.return %}
    return {{ method.return.default }};
    {% endif %}
}

{% endfor %}
} // namespace {{ namespace }}
""".strip()

        # Template de interface
        interface_header = """
#pragma once

{{ license }}

namespace {{ namespace }} {

/**
 * @brief {{ description }}
 */
class {{ name }} {
public:
    /**
     * @brief Destrutor virtual.
     */
    virtual ~{{ name }}() = default;

    {% for method in methods %}
    /**
     * @brief {{ method.description }}
     {% for param in method.params %}
     * @param {{ param.name }} {{ param.description }}
     {% endfor %}
     {% if method.return %}
     * @return {{ method.return.description }}
     {% endif %}
     */
    virtual {{ method.type }} {{ method.name }}(
        {% for param in method.params %}
        {{ param.type }} {{ param.name }}{% if not loop.last %},{% endif %}
        {% endfor %}
    ) = 0;

    {% endfor %}
};

} // namespace {{ namespace }}
""".strip()

        # Template de enum
        enum_header = """
#pragma once

{{ license }}

#include <string>

namespace {{ namespace }} {

/**
 * @brief {{ description }}
 */
enum class {{ name }} {
    {% for value in values %}
    {{ value.name }}, ///< {{ value.description }}
    {% endfor %}
};

/**
 * @brief Converte enum para string.
 * @param value Valor do enum.
 * @return String correspondente.
 */
inline std::string to_string({{ name }} value) {
    switch (value) {
        {% for value in values %}
        case {{ name }}::{{ value.name }}:
            return "{{ value.name }}";
        {% endfor %}
        default:
            return "UNKNOWN";
    }
}

} // namespace {{ namespace }}
""".strip()

        # Template de teste
        test_source = """
{{ license }}

#include <gtest/gtest.h>
#include "{{ header }}"

namespace {{ namespace }} {
namespace test {

class {{ name }}Test : public ::testing::Test {
protected:
    void SetUp() override {
        // TODO: Configurar teste
    }

    void TearDown() override {
        // TODO: Limpar teste
    }

    {% for member in members %}
    {{ member.type }} {{ member.name }}; ///< {{ member.description }}
    {% endfor %}
};

{% for test in tests %}
TEST_F({{ name }}Test, {{ test.name }}) {
    // TODO: Implementar teste
    EXPECT_TRUE(true);
}

{% endfor %}
} // namespace test
} // namespace {{ namespace }}
""".strip()

        # Salva templates
        templates = {
            CODEGEN_CONFIG['templates']['class']['header']: class_header,
            CODEGEN_CONFIG['templates']['class']['source']: class_source,
            CODEGEN_CONFIG['templates']['interface']['header']: interface_header,
            CODEGEN_CONFIG['templates']['enum']['header']: enum_header,
            CODEGEN_CONFIG['templates']['test']['source']: test_source
        }

        for path, content in templates.items():
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, 'w') as f:
                f.write(content.lstrip())

        return True
    except Exception as e:
        print(f'Erro ao criar templates: {e}', file=sys.stderr)
        return False

def generate_class(namespace: str, name: str, description: str,
                  methods: List[Dict], members: List[Dict]) -> bool:
    """
    Gera arquivos de classe.

    Args:
        namespace: Namespace da classe.
        name: Nome da classe.
        description: Descrição da classe.
        methods: Lista de métodos.
        members: Lista de membros.

    Returns:
        True se os arquivos foram gerados com sucesso, False caso contrário.
    """
    try:
        # Carrega ambiente Jinja2
        env = Environment(loader=FileSystemLoader('.'))

        # Prepara dados
        data = {
            'license': CODEGEN_CONFIG['license'],
            'namespace': namespace,
            'name': name,
            'description': description,
            'methods': methods,
            'members': members
        }

        # Gera header
        header_template = env.get_template(CODEGEN_CONFIG['templates']['class']['header'])
        header_path = f'include/{namespace}/{name.lower()}.hpp'
        os.makedirs(os.path.dirname(header_path), exist_ok=True)
        with open(header_path, 'w') as f:
            f.write(header_template.render(**data))

        # Gera source
        source_template = env.get_template(CODEGEN_CONFIG['templates']['class']['source'])
        source_path = f'src/{namespace}/{name.lower()}.cpp'
        data['header'] = f'{namespace}/{name.lower()}.hpp'
        os.makedirs(os.path.dirname(source_path), exist_ok=True)
        with open(source_path, 'w') as f:
            f.write(source_template.render(**data))

        return True
    except Exception as e:
        print(f'Erro ao gerar classe: {e}', file=sys.stderr)
        return False

def generate_interface(namespace: str, name: str, description: str,
                      methods: List[Dict]) -> bool:
    """
    Gera arquivo de interface.

    Args:
        namespace: Namespace da interface.
        name: Nome da interface.
        description: Descrição da interface.
        methods: Lista de métodos.

    Returns:
        True se o arquivo foi gerado com sucesso, False caso contrário.
    """
    try:
        # Carrega ambiente Jinja2
        env = Environment(loader=FileSystemLoader('.'))

        # Prepara dados
        data = {
            'license': CODEGEN_CONFIG['license'],
            'namespace': namespace,
            'name': name,
            'description': description,
            'methods': methods
        }

        # Gera header
        template = env.get_template(CODEGEN_CONFIG['templates']['interface']['header'])
        header_path = f'include/{namespace}/{name.lower()}.hpp'
        os.makedirs(os.path.dirname(header_path), exist_ok=True)
        with open(header_path, 'w') as f:
            f.write(template.render(**data))

        return True
    except Exception as e:
        print(f'Erro ao gerar interface: {e}', file=sys.stderr)
        return False

def generate_enum(namespace: str, name: str, description: str,
                 values: List[Dict]) -> bool:
    """
    Gera arquivo de enum.

    Args:
        namespace: Namespace do enum.
        name: Nome do enum.
        description: Descrição do enum.
        values: Lista de valores.

    Returns:
        True se o arquivo foi gerado com sucesso, False caso contrário.
    """
    try:
        # Carrega ambiente Jinja2
        env = Environment(loader=FileSystemLoader('.'))

        # Prepara dados
        data = {
            'license': CODEGEN_CONFIG['license'],
            'namespace': namespace,
            'name': name,
            'description': description,
            'values': values
        }

        # Gera header
        template = env.get_template(CODEGEN_CONFIG['templates']['enum']['header'])
        header_path = f'include/{namespace}/{name.lower()}.hpp'
        os.makedirs(os.path.dirname(header_path), exist_ok=True)
        with open(header_path, 'w') as f:
            f.write(template.render(**data))

        return True
    except Exception as e:
        print(f'Erro ao gerar enum: {e}', file=sys.stderr)
        return False

def generate_test(namespace: str, name: str, header: str,
                 members: List[Dict], tests: List[Dict]) -> bool:
    """
    Gera arquivo de teste.

    Args:
        namespace: Namespace da classe testada.
        name: Nome da classe testada.
        header: Caminho do header da classe.
        members: Lista de membros do teste.
        tests: Lista de testes.

    Returns:
        True se o arquivo foi gerado com sucesso, False caso contrário.
    """
    try:
        # Carrega ambiente Jinja2
        env = Environment(loader=FileSystemLoader('.'))

        # Prepara dados
        data = {
            'license': CODEGEN_CONFIG['license'],
            'namespace': namespace,
            'name': name,
            'header': header,
            'members': members,
            'tests': tests
        }

        # Gera source
        template = env.get_template(CODEGEN_CONFIG['templates']['test']['source'])
        source_path = f'tests/{namespace}/{name.lower()}_test.cpp'
        os.makedirs(os.path.dirname(source_path), exist_ok=True)
        with open(source_path, 'w') as f:
            f.write(template.render(**data))

        return True
    except Exception as e:
        print(f'Erro ao gerar teste: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_codegen.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  templates             Cria arquivos de template', file=sys.stderr)
        print('  class <json>          Gera classe a partir de JSON', file=sys.stderr)
        print('  interface <json>      Gera interface a partir de JSON', file=sys.stderr)
        print('  enum <json>           Gera enum a partir de JSON', file=sys.stderr)
        print('  test <json>           Gera teste a partir de JSON', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_directories() else 1

    elif command == 'templates':
        return 0 if create_templates() else 1

    elif command == 'class' and len(sys.argv) > 2:
        # Carrega JSON
        try:
            with open(sys.argv[2], 'r') as f:
                data = json.load(f)
        except Exception as e:
            print(f'Erro ao carregar JSON: {e}', file=sys.stderr)
            return 1

        # Gera classe
        return 0 if generate_class(
            data['namespace'],
            data['name'],
            data['description'],
            data['methods'],
            data['members']
        ) else 1

    elif command == 'interface' and len(sys.argv) > 2:
        # Carrega JSON
        try:
            with open(sys.argv[2], 'r') as f:
                data = json.load(f)
        except Exception as e:
            print(f'Erro ao carregar JSON: {e}', file=sys.stderr)
            return 1

        # Gera interface
        return 0 if generate_interface(
            data['namespace'],
            data['name'],
            data['description'],
            data['methods']
        ) else 1

    elif command == 'enum' and len(sys.argv) > 2:
        # Carrega JSON
        try:
            with open(sys.argv[2], 'r') as f:
                data = json.load(f)
        except Exception as e:
            print(f'Erro ao carregar JSON: {e}', file=sys.stderr)
            return 1

        # Gera enum
        return 0 if generate_enum(
            data['namespace'],
            data['name'],
            data['description'],
            data['values']
        ) else 1

    elif command == 'test' and len(sys.argv) > 2:
        # Carrega JSON
        try:
            with open(sys.argv[2], 'r') as f:
                data = json.load(f)
        except Exception as e:
            print(f'Erro ao carregar JSON: {e}', file=sys.stderr)
            return 1

        # Gera teste
        return 0 if generate_test(
            data['namespace'],
            data['name'],
            data['header'],
            data['members'],
            data['tests']
        ) else 1

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
