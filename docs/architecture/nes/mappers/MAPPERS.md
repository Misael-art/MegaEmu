# Documentação dos Mappers do NES

## Visão Geral

Os mappers são circuitos especiais dentro dos cartuchos do NES que expandem as capacidades do console, permitindo jogos maiores e mais complexos. Este documento fornece uma visão geral dos mappers implementados em nosso emulador, detalhando suas características, jogos compatíveis e peculiaridades de implementação.

## Mappers Implementados

Atualmente, o emulador suporta os seguintes mappers:

| Mapper | Nome | Status | Jogos Notáveis |
|--------|------|--------|----------------|
| 0 | NROM | Implementado | Super Mario Bros., Donkey Kong, Balloon Fight |
| 1 | MMC1 (SxROM) | Implementado | The Legend of Zelda, Metroid, Final Fantasy |
| 2 | UxROM | Implementado | Mega Man, Castlevania, Duck Tales |
| 3 | CNROM | Implementado | Solomon's Key, Adventure Island, Paperboy |
| 4 | MMC3 (TxROM) | Implementado | Super Mario Bros. 2 & 3, Kirby's Adventure |
| 5 | MMC5 (ExROM) | Implementado | Castlevania III, Just Breed, Laser Invasion |
| 7 | AxROM | Implementado | Battletoads, Wizards & Warriors |
| 9 | MMC2 (PxROM) | Implementado | Punch-Out!! |
| 10 | MMC4 (FxROM) | Implementado | Fire Emblem |
| 20 | FDS | Implementado | Suporte ao Famicom Disk System |
| 24 | VRC6 | Implementado | Akumajou Densetsu, Madara, Esper Dream 2 |

## Detalhes de Implementação

### Mapper 0 (NROM)

O mapper mais simples, sem chaveamento de bancos.

- **Características**:
  - PRG-ROM: 16KB ou 32KB fixo
  - CHR-ROM: 8KB fixo
  - Sem PRG-RAM
  - Espelhamento: Horizontal ou Vertical (definido pelo hardware)

- **Implementação**:
  - Mapeamento direto de memória
  - Leitura/escrita simples
  - Sem registradores de controle

### Mapper 1 (MMC1/SxROM)

Mapper sofisticado com múltiplas configurações.

- **Características**:
  - PRG-ROM: até 512KB
  - CHR-ROM/RAM: até 128KB
  - PRG-RAM: Suporte a bateria
  - Controle de registradores serial (escrita bit a bit)
  - Múltiplos modos de espelhamento

- **Implementação**:
  - Registrador de deslocamento para configuração
  - Quatro registradores de controle (0-3)
  - Modos de chaveamento PRG: 16KB/32KB
  - Modos de chaveamento CHR: 4KB/8KB

### Mapper 2 (UxROM)

Mapper simples com chaveamento de banco PRG.

- **Características**:
  - PRG-ROM: até 512KB (chaveado em bancos de 16KB)
  - CHR-RAM: 8KB
  - Banco fixo superior ($C000-$FFFF)
  - Banco inferior chaveável ($8000-$BFFF)

- **Implementação**:
  - Registrador único para seleção de banco
  - Escrita em qualquer endereço $8000-$FFFF

### Mapper 3 (CNROM)

Mapper simples com chaveamento de banco CHR.

- **Características**:
  - PRG-ROM: 32KB fixo
  - CHR-ROM: até 32KB (chaveado em bancos de 8KB)
  - Sem PRG-RAM

- **Implementação**:
  - Registrador único para seleção de banco CHR
  - Escrita em qualquer endereço $8000-$FFFF

### Mapper 4 (MMC3/TxROM)

Mapper avançado com IRQ baseado em scanline.

- **Características**:
  - PRG-ROM: até 512KB
  - CHR-ROM/RAM: até 256KB
  - PRG-RAM: Suporte a bateria
  - IRQ baseado em scanline
  - Chaveamento flexível de bancos PRG e CHR

- **Implementação**:
  - Registrador de comando e banco
  - Seleção de banco baseada em registradores
  - Sistema de IRQ por contagem de tiles

### Mapper 5 (MMC5/ExROM)

O mapper mais complexo do NES.

- **Características**:
  - PRG-ROM: até 1MB
  - CHR-ROM/RAM: até 1MB
  - PRG-RAM: até 64KB com bateria
  - Área adicional de EXRAM
  - Múltiplos modos de chaveamento
  - Canais de áudio extras

- **Implementação**:
  - Múltiplos registradores de controle
  - Modos PRG: 8KB, 16KB, 32KB
  - Modos CHR: 1KB, 2KB, 4KB, 8KB
  - Suporte a split screen
  - Multiplicador de hardware
  - Suporte a áudio expandido
  - IRQ baseado em scanline

### Mapper 7 (AxROM)

Mapper simples com chaveamento de banco de 32KB.

- **Características**:
  - PRG-ROM: até 256KB
  - CHR-RAM: 8KB
  - Espelhamento de uma tela

- **Implementação**:
  - Registrador único
  - Bits 0-2: Seleção de banco PRG
  - Bit 4: Seleção de tela (0 = $2000, 1 = $2400)

### Mapper 9 (MMC2/PxROM)

Mapper especializado usado principalmente em Punch-Out!!

- **Características**:
  - PRG-ROM: até 128KB
  - CHR-ROM: até 128KB
  - Chaveamento CHR dinâmico baseado em acesso à PPU

- **Implementação**:
  - Registradores para bancos PRG e CHR
  - Latches para detecção de padrões de tiles
  - Chaveamento CHR automático durante renderização

### Mapper 10 (MMC4/FxROM)

Similar ao MMC2, mas com pequenas diferenças.

- **Características**:
  - PRG-ROM: até 128KB
  - CHR-ROM: até 128KB
  - Chaveamento CHR dinâmico baseado em acesso à PPU

- **Implementação**:
  - Registradores para bancos PRG e CHR
  - Latches para detecção de padrões de tiles (endereços diferentes do MMC2)

### Mapper 20 (FDS)

Suporte ao sistema de disquetes do Famicom.

- **Características**:
  - RAM de disquete
  - Canais de áudio extras (wavetable)
  - I/O de disco
  - Interrupções de timer

- **Implementação**:
  - Emulação do sistema de arquivos FDS
  - Suporte a I/O de disco
  - Canal de wavetable
  - Registradores de I/O expandido

### Mapper 24 (VRC6)

Mapper Konami com áudio expandido.

- **Características**:
  - PRG-ROM: até 256KB
  - CHR-ROM: até 256KB
  - IRQ controlado por timer
  - Canais de áudio adicionais (2 pulsos + sawtooth)

- **Implementação**:
  - Registradores para bancos PRG e CHR
  - Sistema de IRQ por timer
  - Canais de áudio VRC6

## Detalhes de Mapeamento de Memória

### Espaço CPU ($0000-$FFFF)

- **$0000-$1FFF**: RAM interna (2KB espelhada 4 vezes)
- **$2000-$3FFF**: Registradores da PPU (8 bytes espelhados)
- **$4000-$401F**: Registradores da APU e I/O
- **$4020-$5FFF**: Área de expansão
- **$6000-$7FFF**: SRAM/PRG-RAM (geralmente)
- **$8000-$FFFF**: PRG-ROM (mapeado através do mapper)

### Espaço PPU ($0000-$3FFF)

- **$0000-$1FFF**: CHR-ROM/RAM (mapeado através do mapper)
- **$2000-$2FFF**: Name tables (VRAM)
- **$3000-$3EFF**: Espelho de $2000-$2EFF
- **$3F00-$3FFF**: Paletas

## Considerações de Implementação

Ao implementar mappers, consideramos os seguintes aspectos:

1. **Precisão**: Emulação fiel ao hardware original
2. **Timing**: Respeito aos timings específicos de cada mapper
3. **Compatibilidade**: Suporte a quirks específicos de certos jogos
4. **Desempenho**: Otimizações quando possível sem comprometer a precisão

## Testes e Validação

Todos os mappers implementados passam por uma bateria de testes:

1. **ROM Tests**: ROMs de teste específicas para mappers
2. **Game Compatibility**: Testes com jogos comerciais
3. **Edge Cases**: Testes para comportamentos específicos
4. **IRQ Timing**: Testes de precisão de timing de IRQ

## Conclusão

Nossa implementação de mappers visa máxima compatibilidade com o catálogo de jogos do NES. Os mappers atualmente implementados cobrem aproximadamente 90% dos jogos comercialmente lançados, com planos para expansão contínua para incluir mappers mais raros e especializados.

## Referências

1. NesDev Wiki - Mappers: https://wiki.nesdev.org/w/index.php/Mapper
2. NESDev Forums
3. FCEUX/Mesen source code
4. Documentação original da Nintendo (quando disponível)
5. Análises de hardware por kevtris e outros
