# ROMs de Nintendo Entertainment System (NES)

Este diretório é destinado às ROMs de jogos do NES/Famicom para teste do emulador Mega_Emu.

## Formato Suportado

O emulador atualmente suporta ROMs no formato iNES (.nes) com os seguintes mappers:

- Mapper 0: NROM (jogos simples como Super Mario Bros., Donkey Kong, etc.)
- Mapper 1: MMC1 (jogos como Legend of Zelda, Metroid, etc.)
- Mapper 2: UNROM (jogos como Mega Man, Castlevania, etc.)
- Mapper 3: CNROM (jogos como Arkanoid, Bump'n'Jump, etc.)
- Mapper 4: MMC3 (jogos como Super Mario Bros. 3, Kirby's Adventure, etc.)

## Como Adicionar ROMs

1. Coloque seus arquivos .nes diretamente neste diretório
2. O emulador detectará automaticamente as ROMs durante a inicialização
3. As ROMs não são incluídas no repositório por motivos de direitos autorais

## Estrutura Recomendada

```
roms/
  └── nes/
      ├── comerciais/
      │   ├── super_mario_bros.nes
      │   └── ...
      ├── homebrew/
      │   ├── demo1.nes
      │   └── ...
      └── testes/
          ├── cpu_test.nes
          └── ...
```

## ROMs de Teste Recomendadas

Para testar a compatibilidade do emulador, recomendamos as seguintes ROMs de teste:

1. **nestest.nes** - Teste oficial da CPU 6502
2. **cpu_timing_test.nes** - Testes de timing da CPU
3. **ppu_vbl_nmi.nes** - Testes do VBlank e NMI da PPU
4. **sprite_hit_tests.nes** - Testes de colisão de sprites

## Problemas Conhecidos

- Jogos que usam mappers não suportados não funcionarão corretamente
- Alguns jogos podem apresentar problemas gráficos enquanto o emulador está em desenvolvimento

## Observações Legais

O uso de ROMs está sujeito às leis de direitos autorais do seu país. O Mega_Emu não inclui nenhuma ROM, e você deve possuir legalmente os jogos cujas ROMs utiliza para testes.

---

**Nota para Desenvolvedores:** Para testes automatizados, você pode definir o caminho para a ROM no arquivo de configuração ou via linha de comando ao iniciar o emulador.
