# Regras de Build para IA

## Diretrizes Gerais

1. **Estrutura de Diretórios**
   - Sempre respeite a estrutura de diretórios definida
   - Não crie diretórios adicionais sem documentação
   - Mantenha a hierarquia de builds organizada

2. **Scripts de Build**
   - Use apenas os scripts fornecidos em `/scripts/build/`
   - Não modifique os scripts existentes sem atualizar a documentação
   - Mantenha a consistência dos parâmetros entre scripts

3. **CMake**
   - Mantenha as opções de build atualizadas no root CMakeLists.txt
   - Respeite a modularidade dos componentes
   - Documente novas dependências

4. **Dependências**
   - Use vcpkg para gerenciar dependências
   - Mantenha versões específicas no manifest
   - Documente requisitos de sistema

## Regras Específicas

1. **Ao Adicionar Novo Componente**

   ```cmake
   # Exemplo de estrutura a seguir
   option(BUILD_NOVO_COMPONENTE "Build description" OFF)
   if(BUILD_NOVO_COMPONENTE)
     add_subdirectory(novo_componente)
   endif()
   ```

2. **Ao Modificar Build Scripts**

   ```powershell
   # Template para novos scripts
   param(
     [string]$BuildType = "Release",
     [string]$BuildDir = "build",
     [switch]$Clean,
     [switch]$Rebuild
   )
   ```

3. **Ao Configurar Dependências**

   ```json
   {
     "dependencies": [
       {
         "name": "package",
         "version>=": "1.0.0"
       }
     ]
   }
   ```

## Verificações Obrigatórias

1. **Antes de Modificar**
   - Verificar compatibilidade com builds existentes
   - Confirmar que não quebra outros componentes
   - Validar dependências necessárias

2. **Durante Modificações**
   - Manter logs de mudanças
   - Atualizar documentação relevante
   - Testar em diferentes configurações

3. **Após Modificações**
   - Verificar builds em Debug e Release
   - Validar integridade dos binários
   - Atualizar guias de troubleshooting

## Respostas a Problemas Comuns

1. **Erro de Dependência**

   ```powershell
   # Sequência de verificação
   vcpkg update
   Remove-Item build/temp -Recurse -Force
   .\build_component.ps1 -Clean
   ```

2. **Conflito de Versão**

   ```powershell
   # Passos de resolução
   Remove-Item build/CMakeCache.txt
   Remove-Item vcpkg_installed -Recurse -Force
   .\build_component.ps1 -Rebuild
   ```

3. **Falha de Compilação**

   ```powershell
   # Diagnóstico
   Get-Content build/temp/CMakeFiles/CMakeError.log
   Get-Content build/temp/build.log
   ```

## Manutenção

1. **Limpeza Regular**
   - Remover builds temporários antigos
   - Atualizar manifesto do vcpkg
   - Verificar integridade dos scripts

2. **Documentação**
   - Manter AI_GUIDELINE atualizado
   - Documentar novos casos de uso
   - Atualizar exemplos conforme necessário

3. **Versionamento**
   - Seguir semântica de versão
   - Documentar breaking changes
   - Manter changelog atualizado

## Notas de Implementação

1. **Performance**
   - Otimizar flags de compilação
   - Usar compilação paralela quando possível
   - Minimizar reconstruções desnecessárias

2. **Segurança**
   - Validar inputs dos scripts
   - Verificar integridade de downloads
   - Usar hashes para verificação

3. **Compatibilidade**
   - Testar em diferentes versões do Windows
   - Validar com diferentes toolchains
   - Manter compatibilidade retroativa
