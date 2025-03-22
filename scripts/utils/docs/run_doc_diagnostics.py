#!/usr/bin/env python3

import os
from pathlib import Path
from typing import Dict, List

class DocDiagnostics:
    def __init__(self):
        self.results = {
            'structure': {},
            'coverage': {},
            'quality': {},
            'freshness': {},
            'links': {},
            'issues': []
        }

    def run_diagnostics(self):
        self._check_required_structure()
        self._analyze_coverage()
        self._validate_quality()
        self._check_freshness()
        self._verify_links()
        return self.results

    def _check_required_structure(self):
        required_dirs = [
            'docs/architecture',
            'docs/guidelines',
            'docs/api',
            'docs/monitoring',
            'docs/user',
            'docs/development'
        ]
        
        required_files = [
            'docs/INDEX.md',
            'docs/AI_GUIDELINES.md',
            'docs/PROCESSO_CONSULTA.md'
        ]

        for dir_path in required_dirs:
            self.results['structure'][dir_path] = os.path.exists(dir_path)
        
        for file_path in required_files:
            self.results['structure'][file_path] = os.path.exists(file_path)

    def _analyze_coverage(self):
        from tools.analyze_docs import DocHealthAnalyzer
        analyzer = DocHealthAnalyzer('docs/')
        metrics = analyzer.analyze_docs()
        self.results['coverage'] = metrics

    def _validate_quality(self):
        for md_file in Path('docs').rglob('*.md'):
            with open(md_file, 'r', encoding='utf-8') as f:
                content = f.read()
                self.results['quality'][str(md_file)] = {
                    'has_headers': bool(content.count('#')),
                    'has_examples': 'exemplo' in content.lower() or 'example' in content.lower(),
                    'has_structure': bool(content.strip()),
                }

    def generate_report(self) -> str:
        report = "📊 Relatório de Diagnóstico de Documentação\n"
        report += "=" * 50 + "\n\n"
        
        # Estrutura
        report += "📁 Estrutura de Diretórios e Arquivos\n"
        for path, exists in self.results['structure'].items():
            status = "✅" if exists else "❌"
            report += f"{status} {path}\n"
        
        # Cobertura
        report += "\n📝 Cobertura de Documentação\n"
        coverage = self.results['coverage'].get('coverage', 0)
        report += f"Cobertura Total: {coverage:.1f}%\n"
        
        # Qualidade
        report += "\n🎯 Qualidade da Documentação\n"
        quality_score = sum(len(q) for q in self.results['quality'].values()) / len(self.results['quality'])
        report += f"Pontuação de Qualidade: {quality_score:.1f}/3.0\n"
        
        # Links
        report += "\n🔗 Verificação de Links\n"
        broken_links = len(self.results.get('links', {}).get('broken', []))
        report += f"Links Quebrados: {broken_links}\n"
        
        return report

if __name__ == '__main__':
    diagnostics = DocDiagnostics()
    results = diagnostics.run_diagnostics()
    print(diagnostics.generate_report())