# Padrões de Codificação do Mega Emu

## Visão Geral

Este documento define os padrões de codificação para o projeto Mega Emu. Todos os contribuidores devem seguir estas diretrizes para manter a consistência do código.

## Sumário

1. [Estilo Geral](#estilo-geral)
2. [Nomenclatura](#nomenclatura)
3. [Indentação e Formatação](#indentação-e-formatação)
4. [Comentários](#comentários)
5. [Diretivas de Pré-processador](#diretivas-de-pré-processador)
6. [Tipos de Dados](#tipos-de-dados)
7. [Funções](#funções)
8. [Estruturas e Enumerações](#estruturas-e-enumerações)
9. [Tratamento de Erros](#tratamento-de-erros)
10. [Práticas Específicas por Plataforma](#práticas-específicas-por-plataforma)

## Estilo Geral

- O código deve ser escrito em C padrão (C99) para garantir compatibilidade.
- Evite códigos específicos de compilador, a menos que seja absolutamente necessário.
- Utilize as macros de plataforma definidas em `global_defines.h` quando precisar de código específico para uma plataforma.
- Limite as linhas a 100 caracteres quando possível.
- Use espaços em vez de tabulações.

## Nomenclatura

### Arquivos

- Nomes de arquivos devem ser em minúsculas com palavras separadas por underscore (`_`).
- Arquivos de cabeçalho usam a extensão `.h`.
- Arquivos de implementação usam a extensão `.c`.
- Nomeie os arquivos de forma descritiva: `megadrive_vdp.c`, `nes_ppu.c`.

### Variáveis

- Use snake_case para variáveis (todas as letras minúsculas com palavras separadas por underscore).
- Nomes de variáveis devem ser descritivos:
  ```c
  int counter;             // Bom
  int i;                   // Aceitável em loops curtos
  int x;                   // Ruim, a menos que seja uma coordenada
  ```

- Variáveis globais devem ter um prefixo indicando o módulo:
  ```c
  int md_current_scanline;  // Variável global para o Mega Drive
  ```

### Constantes e Macros

- Constantes e macros devem ser em MAIÚSCULAS com palavras separadas por underscore:
  ```c
  #define MAX_ROM_SIZE 8388608
  #define VDP_CONTROL_PORT 0xC00004
  ```

### Funções

- Funções devem usar snake_case.
- Funções públicas devem incluir o nome da plataforma ou módulo como prefixo:
  ```c
  void megadrive_init();
  int z80_execute_instruction();
  ```

- Funções estáticas (privadas) devem começar com underscore:
  ```c
  static void _update_vdp_registers();
  ```

### Tipos

- Tipos e Typedefs: `snake_case_t` (ex: `emulator_context_t`)
- Estruturas: `CamelCase` (ex: `EmulatorContext`)
- Tipos definidos com `typedef` devem terminar com `_t`:
  ```c
  typedef struct {
      uint16_t registers[8];
  } vdp_t;
  ```

## Indentação e Formatação

- Use 4 espaços para indentação.
- Chaves devem seguir o estilo K&R (chave de abertura na mesma linha da declaração, chave de fechamento em sua própria linha):
  ```c
  if (condition) {
      do_something();
  } else {
      do_alternative();
  }
  ```

- Coloque um espaço após palavras-chave (`if`, `for`, `while`, etc.).
- Coloque um espaço antes e depois de operadores binários (`+`, `-`, `*`, `/`, etc.).
- Não coloque espaços após um operador unário (`!`, `~`, `++`, `--`, etc.).
- Coloque um espaço entre uma função e seus parênteses:
  ```c
  void function_name (int parameter);
  ```

- Separe funções com duas linhas em branco.
- Separe seções lógicas do código com uma linha em branco.

## Comentários

- Use comentários de estilo Doxygen para funções públicas:
  ```c
  /**
   * @brief Inicializa o emulador do Mega Drive
   * @param rom_data Ponteiro para os dados da ROM
   * @param rom_size Tamanho da ROM em bytes
   * @return 0 em caso de sucesso, código de erro em caso de falha
   */
  int megadrive_init(const uint8_t *rom_data, size_t rom_size);
  ```

- Use comentários simples para código interno:
  ```c
  // Atualiza registradores de controle
  control_reg = (control_reg & 0xFFF0) | value;
  ```

- Agrupe comentários relacionados a uma seção específica:
  ```c
  /****************************
   * Controladores de Entrada
   ****************************/
  ```

- Comente decisões não óbvias e comportamentos complexos, não o que o código está fazendo.

## Diretivas de Pré-processador

- Diretivas de inclusão devem ser agrupadas e ordenadas:
  1. Cabeçalhos da biblioteca padrão
  2. Cabeçalhos do projeto
  3. Cabeçalhos locais

  ```c
  #include <stdio.h>
  #include <stdlib.h>

  #include "core/core.h"

  #include "megadrive.h"
  #include "megadrive_vdp.h"
  ```

- Use guardas de inclusão em todos os arquivos de cabeçalho:
  ```c
  #ifndef MEGADRIVE_H
  #define MEGADRIVE_H

  // Conteúdo do cabeçalho

  #endif /* MEGADRIVE_H */
  ```

- Use `#if` em vez de `#ifdef` quando verificar definições de macros:
  ```c
  #if defined(PLATFORM_WINDOWS)
      // Código específico do Windows
  #elif defined(PLATFORM_LINUX)
      // Código específico do Linux
  #endif
  ```

## Tipos de Dados

- Use os tipos definidos em `stdint.h` para garantir tamanhos consistentes:
  ```c
  uint8_t byte_value;
  int16_t signed_word;
  uint32_t long_value;
  ```

- Alternativamente, use os tipos abreviados definidos em `global_defines.h`:
  ```c
  u8 byte_value;
  s16 signed_word;
  u32 long_value;
  ```

- Use `size_t` para tamanhos e contagens.
- Use `bool` de `stdbool.h` para valores booleanos.

## Funções

- Funções devem ser pequenas e ter uma única responsabilidade.
- Limite o número de parâmetros a no máximo 5, quando possível.
- Retorne 0 ou um valor positivo para sucesso e valores negativos para erros.
- Documente os códigos de retorno em comentários da função.
- Valide parâmetros de entrada no início da função.

## Estruturas e Enumerações

- Use estruturas para agrupar dados relacionados.
- Alinhe os membros de estrutura para facilitar a leitura:
  ```c
  typedef struct {
      uint16_t  control;       // Registrador de controle
      uint8_t   status;        // Registrador de status
      uint32_t  address;       // Endereço atual
      uint8_t  *buffer;        // Buffer de dados
  } io_device_t;
  ```

- Use enumerações para grupos de constantes relacionadas:
  ```c
  typedef enum {
      REGION_JAPAN,
      REGION_US,
      REGION_EUROPE
  } region_t;
  ```

## Tratamento de Erros

- Verifique valores de retorno de funções.
- Use constantes de erro definidas em `global_defines.h`.
- Seja consistente no tratamento de erros:
  ```c
  int result = function_call();
  if (result < 0) {
      return result;  // Propaga o erro
  }
  ```

- Forneça mensagens de erro claras e descritivas.

## Práticas Específicas por Plataforma

### Mega Drive / Genesis

- Use prefixos `md_` ou `megadrive_` para funções e variáveis específicas.
- Siga as convenções de nomenclatura do hardware original quando relevante.

### Master System

- Use prefixos `sms_` ou `mastersystem_` para funções e variáveis específicas.

### NES / Famicom

- Use prefixos `nes_` para funções e variáveis específicas.

### SNES / Super Famicom

- Use prefixos `snes_` para funções e variáveis específicas.

## Tamanho e Complexidade

1. **Funções:**
   - Limite de 50 linhas por função (ideal)
   - Máximo de 100 linhas por função (absoluto)
   - Uma única responsabilidade por função

2. **Arquivos:**
   - Limite de 300-400 linhas por arquivo (ideal)
   - Máximo de 1000 linhas por arquivo (absoluto)
   - Separar em módulos lógicos quando ultrapassar o limite

3. **Complexidade:**
   - Evite aninhamento excessivo (máximo 3 níveis)
   - Prefira early returns para reduzir aninhamento
   - Limite de 10 para complexidade ciclomática

## Mensagens de Commit

- Use mensagens claras e descritivas
- Siga o formato: `tipo: descrição curta`
- Tipos: feat, fix, docs, style, refactor, test, chore

Exemplo:
```
feat: adiciona suporte ao mapper MMC3
fix: corrige timing da PPU
docs: atualiza documentação da API
```

---

Este guia deve ser seguido por todos os contribuidores do projeto. Modificações a este guia devem ser discutidas e aprovadas pela equipe de desenvolvimento.
