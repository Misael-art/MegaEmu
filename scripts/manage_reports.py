#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script para gerenciar a geração de relatórios.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import subprocess
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
from jinja2 import Environment, FileSystemLoader

# Configurações de relatórios
REPORTS_CONFIG = {
    'types': {
        'performance': {
            'metrics': [
                'fps',
                'frame_time',
                'cpu_usage',
                'memory_usage',
                'audio_latency'
            ],
            'thresholds': {
                'fps': 60.0,
                'frame_time': 16.67,  # ms
                'cpu_usage': 80.0,  # %
                'memory_usage': 512.0,  # MB
                'audio_latency': 20.0  # ms
            },
            'charts': [
                'line',
                'histogram',
                'boxplot',
                'heatmap'
            ]
        },
        'compatibility': {
            'metrics': [
                'rom_name',
                'rom_size',
                'rom_region',
                'rom_version',
                'status',
                'issues',
                'fixes'
            ],
            'status_levels': [
                'perfect',
                'playable',
                'ingame',
                'menu',
                'intro',
                'crash'
            ],
            'charts': [
                'pie',
                'bar',
                'treemap'
            ]
        },
        'coverage': {
            'metrics': [
                'lines',
                'functions',
                'branches'
            ],
            'thresholds': {
                'lines': 80.0,
                'functions': 90.0,
                'branches': 70.0
            },
            'charts': [
                'bar',
                'treemap',
                'sunburst'
            ]
        }
    },
    'formats': {
        'html': {
            'template': 'templates/report.html',
            'assets': [
                'templates/assets/style.css',
                'templates/assets/script.js'
            ]
        },
        'pdf': {
            'template': 'templates/report.tex',
            'engine': 'xelatex',
            'options': [
                '-interaction=nonstopmode',
                '-shell-escape'
            ]
        },
        'markdown': {
            'template': 'templates/report.md',
            'extensions': [
                'tables',
                'fenced_code',
                'footnotes'
            ]
        }
    },
    'output': {
        'path': 'reports',
        'name_template': '{type}-{timestamp}',
        'timestamp_format': '%Y%m%d_%H%M%S'
    }
}

def create_directories() -> bool:
    """
    Cria a estrutura de diretórios para relatórios.

    Returns:
        True se os diretórios foram criados com sucesso, False caso contrário.
    """
    try:
        # Cria diretórios principais
        dirs = [
            REPORTS_CONFIG['output']['path'],
            'templates',
            'templates/assets',
            'templates/includes'
        ]

        # Adiciona subdiretórios por tipo
        for report_type in REPORTS_CONFIG['types']:
            dirs.append(f"{REPORTS_CONFIG['output']['path']}/{report_type}")

        # Cria diretórios
        for directory in dirs:
            os.makedirs(directory, exist_ok=True)

        return True
    except Exception as e:
        print(f'Erro ao criar diretórios: {e}', file=sys.stderr)
        return False

def create_templates() -> bool:
    """
    Cria arquivos de template para relatórios.

    Returns:
        True se os templates foram criados com sucesso, False caso contrário.
    """
    try:
        # Template HTML
        html_template = """
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{{ title }}</title>
    <link rel="stylesheet" href="assets/style.css">
</head>
<body>
    <header>
        <h1>{{ title }}</h1>
        <p>Gerado em {{ timestamp }}</p>
    </header>

    <main>
        {% for section in sections %}
        <section>
            <h2>{{ section.title }}</h2>
            {% if section.description %}
            <p>{{ section.description }}</p>
            {% endif %}

            {% if section.table %}
            <table>
                <thead>
                    <tr>
                    {% for header in section.table.headers %}
                        <th>{{ header }}</th>
                    {% endfor %}
                    </tr>
                </thead>
                <tbody>
                    {% for row in section.table.rows %}
                    <tr>
                        {% for cell in row %}
                        <td>{{ cell }}</td>
                        {% endfor %}
                    </tr>
                    {% endfor %}
                </tbody>
            </table>
            {% endif %}

            {% if section.chart %}
            <div class="chart">
                <img src="{{ section.chart }}" alt="Gráfico">
            </div>
            {% endif %}
        </section>
        {% endfor %}
    </main>

    <footer>
        <p>Mega Emu - {{ report_type }} Report</p>
    </footer>

    <script src="assets/script.js"></script>
</body>
</html>
"""

        # Template LaTeX
        latex_template = r"""
\documentclass[12pt,a4paper]{article}

\usepackage[utf8]{inputenc}
\usepackage[T1]{fontenc}
\usepackage[brazilian]{babel}
\usepackage{graphicx}
\usepackage{booktabs}
\usepackage{longtable}
\usepackage{geometry}
\usepackage{hyperref}

\title{ {{title}} }
\author{Mega Emu}
\date{ {{timestamp}} }

\begin{document}

\maketitle

{% for section in sections %}
\section{ {{section.title}} }

{% if section.description %}
{{section.description}}
{% endif %}

{% if section.table %}
\begin{longtable}{
    {% for _ in section.table.headers %}l{% endfor %}
}
\toprule
{% for header in section.table.headers %}
    {{header}} &
{% endfor %}
\\
\midrule
{% for row in section.table.rows %}
    {% for cell in row %}
        {{cell}} &
    {% endfor %}
    \\
{% endfor %}
\bottomrule
\end{longtable}
{% endif %}

{% if section.chart %}
\begin{figure}[h]
    \centering
    \includegraphics[width=\textwidth]{ {{section.chart}} }
\end{figure}
{% endif %}

{% endfor %}

\end{document}
"""

        # Template Markdown
        markdown_template = """
# {{ title }}

Gerado em {{ timestamp }}

{% for section in sections %}
## {{ section.title }}

{% if section.description %}
{{ section.description }}
{% endif %}

{% if section.table %}
| {% for header in section.table.headers %}{{ header }} |{% endfor %}
|{% for _ in section.table.headers %}---|{% endfor %}
{% for row in section.table.rows %}
| {% for cell in row %}{{ cell }} |{% endfor %}
{% endfor %}
{% endif %}

{% if section.chart %}
![Gráfico]({{ section.chart }})
{% endif %}

{% endfor %}

---
Mega Emu - {{ report_type }} Report
"""

        # CSS
        css_template = """
body {
    font-family: Arial, sans-serif;
    line-height: 1.6;
    margin: 0;
    padding: 20px;
}

header {
    margin-bottom: 30px;
}

h1 {
    color: #333;
}

section {
    margin-bottom: 40px;
}

table {
    width: 100%;
    border-collapse: collapse;
    margin: 20px 0;
}

th, td {
    padding: 10px;
    border: 1px solid #ddd;
    text-align: left;
}

th {
    background-color: #f5f5f5;
}

.chart {
    margin: 20px 0;
    text-align: center;
}

.chart img {
    max-width: 100%;
    height: auto;
}

footer {
    margin-top: 50px;
    padding-top: 20px;
    border-top: 1px solid #ddd;
    color: #666;
}
"""

        # JavaScript
        js_template = """
document.addEventListener('DOMContentLoaded', function() {
    // Adiciona classes para linhas alternadas nas tabelas
    const tables = document.querySelectorAll('table');
    tables.forEach(table => {
        const rows = table.querySelectorAll('tbody tr');
        rows.forEach((row, index) => {
            if (index % 2 === 0) {
                row.style.backgroundColor = '#f9f9f9';
            }
        });
    });

    // Adiciona tooltips nos gráficos
    const charts = document.querySelectorAll('.chart img');
    charts.forEach(chart => {
        chart.title = 'Clique para ampliar';
        chart.style.cursor = 'pointer';
        chart.addEventListener('click', function() {
            this.classList.toggle('expanded');
        });
    });
});
"""

        # Salva templates
        templates = {
            'templates/report.html': html_template,
            'templates/report.tex': latex_template,
            'templates/report.md': markdown_template,
            'templates/assets/style.css': css_template,
            'templates/assets/script.js': js_template
        }

        for path, content in templates.items():
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, 'w') as f:
                f.write(content.lstrip())

        return True
    except Exception as e:
        print(f'Erro ao criar templates: {e}', file=sys.stderr)
        return False

def load_data(report_type: str, input_file: str) -> Optional[pd.DataFrame]:
    """
    Carrega dados para o relatório.

    Args:
        report_type: Tipo de relatório.
        input_file: Arquivo de entrada.

    Returns:
        DataFrame com os dados ou None em caso de erro.
    """
    try:
        # Carrega dados
        if input_file.endswith('.json'):
            df = pd.read_json(input_file)
        elif input_file.endswith('.csv'):
            df = pd.read_csv(input_file)
        else:
            raise ValueError('Formato de arquivo não suportado')

        # Valida colunas
        required_columns = REPORTS_CONFIG['types'][report_type]['metrics']
        missing_columns = set(required_columns) - set(df.columns)
        if missing_columns:
            raise ValueError(f'Colunas ausentes: {", ".join(missing_columns)}')

        return df
    except Exception as e:
        print(f'Erro ao carregar dados: {e}', file=sys.stderr)
        return None

def generate_charts(df: pd.DataFrame, report_type: str,
                   output_dir: str) -> List[str]:
    """
    Gera gráficos para o relatório.

    Args:
        df: DataFrame com os dados.
        report_type: Tipo de relatório.
        output_dir: Diretório de saída.

    Returns:
        Lista de caminhos dos gráficos gerados.
    """
    try:
        charts = []
        config = REPORTS_CONFIG['types'][report_type]

        # Configura estilo
        plt.style.use('seaborn')
        sns.set_palette('husl')

        # Para cada tipo de gráfico
        for chart_type in config['charts']:
            if chart_type == 'line':
                # Gráfico de linha para métricas ao longo do tempo
                for metric in config['metrics']:
                    if metric in df.columns and df[metric].dtype in ['int64', 'float64']:
                        plt.figure(figsize=(10, 6))
                        sns.lineplot(data=df, y=metric)
                        plt.title(f'{metric.replace("_", " ").title()} ao longo do tempo')
                        plt.grid(True)

                        # Adiciona linha de threshold se existir
                        if 'thresholds' in config and metric in config['thresholds']:
                            plt.axhline(y=config['thresholds'][metric],
                                      color='r', linestyle='--',
                                      label=f'Threshold ({config["thresholds"][metric]})')
                            plt.legend()

                        # Salva gráfico
                        chart_path = f'{output_dir}/{metric}_line.png'
                        plt.savefig(chart_path, dpi=300, bbox_inches='tight')
                        plt.close()
                        charts.append(chart_path)

            elif chart_type == 'histogram':
                # Histograma para distribuição de métricas
                for metric in config['metrics']:
                    if metric in df.columns and df[metric].dtype in ['int64', 'float64']:
                        plt.figure(figsize=(10, 6))
                        sns.histplot(data=df, x=metric, kde=True)
                        plt.title(f'Distribuição de {metric.replace("_", " ").title()}')
                        plt.grid(True)

                        # Salva gráfico
                        chart_path = f'{output_dir}/{metric}_hist.png'
                        plt.savefig(chart_path, dpi=300, bbox_inches='tight')
                        plt.close()
                        charts.append(chart_path)

            elif chart_type == 'boxplot':
                # Boxplot para métricas numéricas
                numeric_cols = df.select_dtypes(include=['int64', 'float64']).columns
                if len(numeric_cols) > 0:
                    plt.figure(figsize=(12, 6))
                    sns.boxplot(data=df[numeric_cols])
                    plt.title('Distribuição das Métricas')
                    plt.xticks(rotation=45)
                    plt.grid(True)

                    # Salva gráfico
                    chart_path = f'{output_dir}/metrics_box.png'
                    plt.savefig(chart_path, dpi=300, bbox_inches='tight')
                    plt.close()
                    charts.append(chart_path)

            elif chart_type == 'heatmap':
                # Heatmap de correlação entre métricas
                numeric_cols = df.select_dtypes(include=['int64', 'float64']).columns
                if len(numeric_cols) > 1:
                    plt.figure(figsize=(10, 8))
                    sns.heatmap(df[numeric_cols].corr(), annot=True, cmap='coolwarm')
                    plt.title('Correlação entre Métricas')

                    # Salva gráfico
                    chart_path = f'{output_dir}/correlation_heatmap.png'
                    plt.savefig(chart_path, dpi=300, bbox_inches='tight')
                    plt.close()
                    charts.append(chart_path)

            elif chart_type == 'pie':
                # Gráfico de pizza para distribuição de categorias
                for metric in config['metrics']:
                    if metric in df.columns and df[metric].dtype == 'object':
                        plt.figure(figsize=(10, 8))
                        df[metric].value_counts().plot(kind='pie', autopct='%1.1f%%')
                        plt.title(f'Distribuição de {metric.replace("_", " ").title()}')

                        # Salva gráfico
                        chart_path = f'{output_dir}/{metric}_pie.png'
                        plt.savefig(chart_path, dpi=300, bbox_inches='tight')
                        plt.close()
                        charts.append(chart_path)

            elif chart_type == 'bar':
                # Gráfico de barras para contagem de categorias
                for metric in config['metrics']:
                    if metric in df.columns and df[metric].dtype == 'object':
                        plt.figure(figsize=(12, 6))
                        sns.countplot(data=df, x=metric)
                        plt.title(f'Contagem por {metric.replace("_", " ").title()}')
                        plt.xticks(rotation=45)
                        plt.grid(True)

                        # Salva gráfico
                        chart_path = f'{output_dir}/{metric}_bar.png'
                        plt.savefig(chart_path, dpi=300, bbox_inches='tight')
                        plt.close()
                        charts.append(chart_path)

        return charts
    except Exception as e:
        print(f'Erro ao gerar gráficos: {e}', file=sys.stderr)
        return []

def generate_report(report_type: str, data: pd.DataFrame,
                   charts: List[str], output_format: str) -> bool:
    """
    Gera relatório no formato especificado.

    Args:
        report_type: Tipo de relatório.
        data: DataFrame com os dados.
        charts: Lista de caminhos dos gráficos.
        output_format: Formato de saída.

    Returns:
        True se o relatório foi gerado com sucesso, False caso contrário.
    """
    try:
        # Carrega template
        env = Environment(loader=FileSystemLoader('.'))
        template = env.get_template(REPORTS_CONFIG['formats'][output_format]['template'])

        # Prepara dados para o template
        timestamp = datetime.now().strftime('%d/%m/%Y %H:%M:%S')
        title = f'Relatório de {report_type.title()}'

        sections = []

        # Seção de resumo
        sections.append({
            'title': 'Resumo',
            'description': f'Análise de {len(data)} registros.',
            'table': {
                'headers': ['Métrica', 'Valor'],
                'rows': [
                    ['Total de registros', len(data)],
                    ['Período', f'{data.index.min()} a {data.index.max()}']
                ]
            }
        })

        # Seção de métricas
        sections.append({
            'title': 'Métricas',
            'table': {
                'headers': ['Métrica', 'Mínimo', 'Máximo', 'Média', 'Mediana'],
                'rows': []
            }
        })

        for metric in REPORTS_CONFIG['types'][report_type]['metrics']:
            if metric in data.columns and data[metric].dtype in ['int64', 'float64']:
                sections[-1]['table']['rows'].append([
                    metric.replace('_', ' ').title(),
                    f'{data[metric].min():.2f}',
                    f'{data[metric].max():.2f}',
                    f'{data[metric].mean():.2f}',
                    f'{data[metric].median():.2f}'
                ])

        # Seção de gráficos
        if charts:
            sections.append({
                'title': 'Gráficos',
                'description': 'Visualização das métricas.',
                'charts': charts
            })

        # Gera relatório
        output = template.render(
            title=title,
            timestamp=timestamp,
            report_type=report_type,
            sections=sections
        )

        # Define nome do arquivo
        timestamp = datetime.now().strftime(REPORTS_CONFIG['output']['timestamp_format'])
        output_name = REPORTS_CONFIG['output']['name_template'].format(
            type=report_type,
            timestamp=timestamp
        )
        output_file = f"{REPORTS_CONFIG['output']['path']}/{report_type}/{output_name}.{output_format}"

        # Salva relatório
        with open(output_file, 'w') as f:
            f.write(output)

        # Se for PDF, compila com LaTeX
        if output_format == 'pdf':
            result = subprocess.run(
                ['xelatex', *REPORTS_CONFIG['formats']['pdf']['options'],
                 output_file],
                capture_output=True,
                text=True
            )
            if result.returncode != 0:
                print('Erro ao compilar PDF:', file=sys.stderr)
                print(result.stderr, file=sys.stderr)
                return False

        return True
    except Exception as e:
        print(f'Erro ao gerar relatório: {e}', file=sys.stderr)
        return False

def main() -> int:
    """
    Função principal.

    Returns:
        0 se todas as operações foram bem sucedidas, 1 caso contrário.
    """
    if len(sys.argv) < 2:
        print('Uso: manage_reports.py <comando> [argumentos]', file=sys.stderr)
        print('\nComandos disponíveis:', file=sys.stderr)
        print('  init                  Cria estrutura de diretórios', file=sys.stderr)
        print('  templates             Cria arquivos de template', file=sys.stderr)
        print('  generate <tipo> <arquivo> <formato>', file=sys.stderr)
        return 1

    command = sys.argv[1]

    if command == 'init':
        return 0 if create_directories() else 1

    elif command == 'templates':
        return 0 if create_templates() else 1

    elif command == 'generate' and len(sys.argv) > 4:
        report_type = sys.argv[2]
        input_file = sys.argv[3]
        output_format = sys.argv[4]

        # Valida tipo de relatório
        if report_type not in REPORTS_CONFIG['types']:
            print(f'Tipo de relatório inválido: {report_type}', file=sys.stderr)
            print('Tipos válidos: ' +
                  ', '.join(REPORTS_CONFIG['types'].keys()),
                  file=sys.stderr)
            return 1

        # Valida formato de saída
        if output_format not in REPORTS_CONFIG['formats']:
            print(f'Formato de saída inválido: {output_format}', file=sys.stderr)
            print('Formatos válidos: ' +
                  ', '.join(REPORTS_CONFIG['formats'].keys()),
                  file=sys.stderr)
            return 1

        # Carrega dados
        data = load_data(report_type, input_file)
        if data is None:
            print('\nErro ao carregar dados!', file=sys.stderr)
            return 1

        # Define diretório de saída
        output_dir = f"{REPORTS_CONFIG['output']['path']}/{report_type}"
        os.makedirs(output_dir, exist_ok=True)

        # Gera gráficos
        charts = generate_charts(data, report_type, output_dir)
        if not charts:
            print('\nErro ao gerar gráficos!', file=sys.stderr)
            return 1

        # Gera relatório
        if not generate_report(report_type, data, charts, output_format):
            print('\nErro ao gerar relatório!', file=sys.stderr)
            return 1

        return 0

    else:
        print('Comando inválido ou argumentos insuficientes.', file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
