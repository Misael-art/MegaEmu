# Especificação de Criptografia para Save States

**Versão:** 1.0
**Data:** 2025-03-18
**Status:** Desenvolvimento

## 1. Visão Geral

Este documento detalha a implementação de criptografia para o sistema de save states do Mega_Emu, utilizando o algoritmo AES-256 para proteção de dados. A criptografia é projetada para garantir a confidencialidade e integridade dos save states, permitindo armazenamento seguro e compartilhamento entre serviços de nuvem.

## 2. Objetivos

A implementação de criptografia para save states visa atender aos seguintes objetivos:

- Proteger dados sensíveis armazenados nos save states
- Prevenir modificações não autorizadas dos arquivos
- Permitir armazenamento seguro em serviços de nuvem
- Garantir a integridade dos dados
- Minimizar o impacto no desempenho
- Fornecer flexibilidade para diferentes níveis de segurança

## 3. Algoritmos de Criptografia

### 3.1 AES-256

O Advanced Encryption Standard (AES) com chaves de 256 bits foi escolhido como o algoritmo principal devido à:

- Segurança comprovada e resistência a ataques conhecidos
- Disponibilidade de implementações otimizadas em múltiplas plataformas
- Desempenho adequado para arquivos de save state
- Suporte amplo em bibliotecas de criptografia

### 3.2 Modos de Operação

A implementação suporta dois modos de operação AES:

#### 3.2.1 CBC (Cipher Block Chaining)

- Utiliza um vetor de inicialização (IV) de 16 bytes
- Cada bloco depende do bloco anterior
- Implementação mais simples e amplamente suportada

#### 3.2.2 GCM (Galois/Counter Mode)

- Fornece autenticação integrada (AEAD - Authenticated Encryption with Associated Data)
- Produz uma tag de autenticação de 16 bytes
- Melhor desempenho em hardware com instruções especializadas
- Proteção contra manipulação não autorizada

### 3.3 Derivação de Chave

Para gerar chaves a partir de senhas de usuário, usamos PBKDF2 (Password-Based Key Derivation Function 2):

- Utiliza HMAC-SHA256 como função de pseudo-randomização
- Requer no mínimo 10.000 iterações (configurável)
- Utiliza salt aleatório de 16 bytes para prevenir ataques de tabela pré-computada
- Produz chaves de 256 bits (32 bytes)

### 3.4 Geração de Números Aleatórios

A segurança do sistema depende da qualidade dos números aleatórios gerados para:

- Vetores de inicialização (IV)
- Salt para PBKDF2
- Chaves geradas automaticamente

A implementação utiliza geradores de números aleatórios criptograficamente seguros (CSPRNG) apropriados para cada plataforma:

- **Windows**: CryptGenRandom API
- **Linux/macOS**: /dev/urandom
- **Outros sistemas**: Combinação de fontes de entropia e uso de bibliotecas seguras

## 4. Estrutura de Arquivo Criptografado

A estrutura de um save state criptografado preserva o formato básico descrito em `save-state-format.md`, com algumas modificações:

```
+-------------------+
| Assinatura (16B)  | <- Não criptografado
+-------------------+
| Cabeçalho (48B)   | <- Não criptografado, contém flags e metadados de criptografia
+-------------------+
| Dados de Crypto   | <- IV, Salt, Tag de autenticação (modo GCM), etc.
+-------------------+
| Metadados         | <- Criptografado
+-------------------+
| Thumbnail         | <- Opcional, pode ser criptografado separadamente
+-------------------+
| Regiões           | <- Criptografado
+-------------------+
| Dados de Regiões  | <- Criptografado
+-------------------+
| Checksum (32B)    | <- HMAC-SHA256 sobre todos os dados (criptografados)
+-------------------+
```

### 4.1 Flags de Criptografia no Cabeçalho

O cabeçalho do save state foi estendido para incluir informações sobre a criptografia:

```c
typedef struct {
    char signature[16];       /* "MEGA_EMU_SAVE\0\0\0" */
    uint32_t version;         /* Versão do formato (0x0300) */
    uint32_t platform_id;     /* ID da plataforma */
    uint32_t flags;           /* Bits de flag */
    uint64_t timestamp;       /* Timestamp de criação */
    uint32_t metadata_offset; /* Offset para a tabela de metadados */
    uint32_t crypto_info;     /* Informações de criptografia */
    /* ... outros campos ... */
} save_state_header_t;
```

O campo `flags` contém bits específicos para criptografia:

- Bit 8 (0x0100): Indica se o arquivo está criptografado
- Bit 9 (0x0200): Indica se o thumbnail está criptografado separadamente
- Bit 10 (0x0400): Indica se regiões sensíveis estão criptografadas individualmente

O campo `crypto_info` contém informações sobre o método de criptografia:

- Bits 0-3: Método de criptografia
  - 0: Sem criptografia
  - 1: AES-256-CBC
  - 2: AES-256-GCM
- Bits 4-7: Método de derivação de chave
  - 0: Sem derivação (chave direta)
  - 1: PBKDF2 com HMAC-SHA256
- Bits 8-15: Reservado para uso futuro
- Bits 16-31: Número de iterações PBKDF2 dividido por 1000

### 4.2 Seção de Dados Criptográficos

Esta seção é inserida logo após o cabeçalho e contém dados necessários para a descriptografia:

```c
typedef struct {
    uint32_t size;            /* Tamanho total desta seção */
    uint8_t iv[16];           /* Vetor de inicialização */
    uint8_t salt[16];         /* Salt para PBKDF2 (se usado) */
    uint8_t auth_tag[16];     /* Tag de autenticação para GCM */
    uint8_t reserved[16];     /* Reservado para uso futuro */
} crypto_data_section_t;
```

## 5. Processo de Criptografia

### 5.1 Criptografia de Save State

O processo para criptografar um save state é o seguinte:

1. **Preparação**:
   - Gerar um IV aleatório de 16 bytes
   - Se usando senha, gerar salt aleatório e derivar chave com PBKDF2
   - Atualizar cabeçalho com flags e informações de criptografia
   - Criar seção de dados criptográficos

2. **Para modo CBC**:
   - Criptografar todos os dados após a seção de dados criptográficos
   - Calcular HMAC-SHA256 dos dados criptografados para verificação de integridade

3. **Para modo GCM**:
   - Criptografar e autenticar os dados após a seção de dados criptográficos
   - Armazenar a tag de autenticação na seção de dados criptográficos

4. **Finalização**:
   - Escrever o arquivo completo

### 5.2 Descriptografia de Save State

O processo para descriptografar um save state é o seguinte:

1. **Validação**:
   - Ler o cabeçalho para obter flags e informações de criptografia
   - Verificar se a assinatura é válida
   - Extrair IV, salt e outros dados criptográficos

2. **Preparação da Chave**:
   - Se derivação de chave está ativa, derivar chave usando password e salt
   - Caso contrário, usar chave fornecida diretamente

3. **Para modo CBC**:
   - Verificar HMAC para confirmar integridade
   - Descriptografar os dados

4. **Para modo GCM**:
   - Descriptografar os dados e verificar a tag de autenticação em uma operação
   - Se a verificação falhar, abortar (dados corrompidos ou chave incorreta)

5. **Processamento Final**:
   - Continuar com o carregamento normal do save state

## 6. Criptografia Seletiva

O sistema permite dois níveis de criptografia:

### 6.1 Criptografia Total

Todo o conteúdo do save state (exceto cabeçalho e dados criptográficos) é criptografado como um único bloco.

### 6.2 Criptografia Seletiva

Apenas regiões marcadas como sensíveis são criptografadas. Isso permite:

- Melhor desempenho
- Visualização de metadados e thumbnails sem descriptografia completa
- Proteção apenas dos dados críticos (como SRAM, senhas, etc.)

A criptografia seletiva utiliza múltiplos blocos criptográficos, cada um com seu próprio IV:

```
+-------------------+
| Cabeçalho + Flags | <- Não criptografado
+-------------------+
| Dados de Crypto   | <- Dados gerais de criptografia
+-------------------+
| Metadados         | <- Não criptografado
+-------------------+
| Thumbnail         | <- Não criptografado
+-------------------+
| Mapa de Regiões   | <- Não criptografado
+-------------------+
| Região 1          | <- Não criptografado
+-------------------+
| Região 2          | <- Criptografado (região sensível)
| Crypto Dados 2    |
+-------------------+
| Região 3          | <- Não criptografado
+-------------------+
| ...               |
+-------------------+
| Checksum          | <- Sobre todos os dados
+-------------------+
```

Cada região sensível inclui seus próprios dados criptográficos (IV, tag, etc.).

## 7. Gerenciamento de Chaves

O sistema oferece várias opções para gerenciamento de chaves:

### 7.1 Chaves Baseadas em Senha

- Usuário fornece uma senha
- Sistema deriva a chave usando PBKDF2
- Salt é armazenado no arquivo para permitir regeneração da chave
- Senha nunca é armazenada

### 7.2 Chaves Aleatórias

- Sistema gera uma chave aleatória de 256 bits
- Chave pode ser armazenada em um arquivo de chave separado
- Oferece maior segurança, mas requer gerenciamento da chave

### 7.3 Arquivo de Chave

Para chaves aleatórias, o sistema pode exportar e importar arquivos de chave:

```
+-------------------+
| Assinatura (12B)  | "MEGA_EMU_KEY"
+-------------------+
| Método (1B)       | Método de criptografia (CBC/GCM)
+-------------------+
| Flags (1B)        | Flags de capacidades
+-------------------+
| Reservado (2B)    |
+-------------------+
| Chave (32B)       | Chave AES-256
+-------------------+
| IV (16B)          | IV opcional para certos modos
+-------------------+
```

## 8. Integração com a API

A API de criptografia está integrada com o sistema de save states:

```c
/* Configuração de criptografia */
emu_encryption_config_t config = {
    .method = EMU_CRYPT_AES256_GCM,
    .derive_from_password = true,
    .password = "senha_segura",
    .kdf_iterations = 10000
};

/* Configura criptografia para o contexto */
emu_save_state_set_encryption(state, &config);

/* Salva com criptografia ativada */
emu_save_options_t options = {
    .flags = EMU_SAVE_OPT_COMPRESS | EMU_SAVE_OPT_ENCRYPT,
    .compression_level = 6
};
emu_save_state_save(state, "savefile.state", &options);
```

Para carregar:

```c
/* Configuração para carregar */
emu_encryption_config_t config = {
    .method = EMU_CRYPT_AES256_GCM,
    .derive_from_password = true,
    .password = "senha_segura"
};

/* Configura criptografia */
emu_save_state_set_encryption(state, &config);

/* Carrega arquivo criptografado */
emu_save_state_load(state, "savefile.state", NULL);
```

## 9. Considerações de Segurança

### 9.1 Proteção de Chaves em Memória

- Chaves e senhas são limpos da memória após o uso
- Chaves derivadas são armazenadas em regiões protegidas da memória quando possível
- Operações com chaves usam comparações de tempo constante para prevenir timing attacks

### 9.2 Resistência a Ataques

A implementação considera resistência a:

- Ataques de força bruta (através de PBKDF2 com muitas iterações)
- Ataques de dicionário (através de salt aleatório)
- Ataques de replay (IVs únicos)
- Manipulação de dados (autenticação em GCM, HMAC em CBC)
- Análise de timing (operações de tempo constante)

### 9.3 Limitações

A implementação tem algumas limitações conhecidas:

- Depende da segurança da senha escolhida pelo usuário
- No modo CBC, não protege contra ataques de padding oracle se implementado incorretamente
- A segurança é limitada pela força do algoritmo AES-256 e sua implementação

## 10. Considerações de Desempenho

O desempenho da criptografia foi avaliado em várias plataformas:

| Plataforma       | Tamanho | Tempo CBC | Tempo GCM | Notas                     |
|------------------|---------|-----------|-----------|---------------------------|
| PC (x86-64/AES-NI) | 1MB   | <10ms     | <5ms      | Hardware acelerado       |
| PC (x86 sem AES-NI) | 1MB  | ~30ms     | ~40ms     | Implementação em software |
| Dispositivos ARM | 1MB    | ~50ms     | ~60ms     | Varia com suporte de HW   |
| Dispositivos Web | 1MB    | ~100ms    | ~120ms    | Via WebCrypto API         |

A criptografia seletiva pode reduzir esses tempos significativamente.

## 11. Integração com Sistema de Nuvem

A criptografia é projetada para trabalhar bem com o sistema de sincronização em nuvem:

- Save states são criptografados antes de serem enviados para a nuvem
- Chaves nunca são enviadas para servidores remotos
- Mesmo provedores sem suporte a criptografia armazenam dados seguros
- Metadados de nuvem podem ser mantidos não criptografados para permitir indexação

## 12. Compatibilidade e Migração

### 12.1 Compatibilidade com Versões Anteriores

Save states criptografados não são compatíveis com versões anteriores do emulador. Uma migração controlada é necessária.

### 12.2 Estratégia de Migração

Para migrar save states existentes:

1. O usuário ativa a criptografia nas configurações
2. Ao carregar um save state não criptografado, o sistema oferece criptografá-lo
3. Após confirmação, o sistema salva uma nova versão criptografada

### 12.3 Detecção Automática

O sistema detecta automaticamente se um save state está criptografado e qual método é usado:

```c
emu_crypto_method_t method;
if (emu_save_state_is_encrypted("savefile.state", &method)) {
    printf("Arquivo criptografado usando método %d\n", method);
} else {
    printf("Arquivo não criptografado\n");
}
```

## 13. Implementação de Referência

### 13.1 Dependências

A implementação de referência depende de:

- OpenSSL 1.1.1 ou superior
- Compilador compatível com C99
- Suporte a tipos de dados de 64 bits

### 13.2 Portabilidade

A implementação é projetada para ser portável entre:

- Windows (Win32/Win64)
- Linux
- macOS
- Compilações Web via Emscripten

## 14. Testes de Segurança

A implementação inclui testes abrangentes:

- Vetores de teste padrão para AES-256
- Testes de integridade para CBC+HMAC e GCM
- Testes de recuperação para casos de corrupção
- Testes de desempenho para diferentes configurações

## 15. Exemplo de Uso Completo

### 15.1 Criptografia com senha

```c
// Inicializa contexto de save state
emu_save_state_t* state = emu_save_state_init(EMU_PLATFORM_MEGA_DRIVE, rom_data, rom_size);

// Configura criptografia com senha
emu_encryption_config_t crypto_config = {
    .method = EMU_CRYPT_AES256_GCM,  // Usar GCM para autenticação
    .derive_from_password = true,
    .password = "senha_secreta",
    .kdf_iterations = 10000,
    .kdf = EMU_KDF_PBKDF2
};

// Aplica configuração de criptografia
emu_save_state_set_encryption(state, &crypto_config);

// Registra componentes normalmente
md_save_state_register(state);

// Salva com criptografia
emu_save_options_t options = {
    .flags = EMU_SAVE_OPT_COMPRESS | EMU_SAVE_OPT_THUMBNAIL | EMU_SAVE_OPT_ENCRYPT,
    .compression_level = 6
};

// Salva o arquivo criptografado
emu_save_state_save(state, "jogo_secreto.state", &options);

// Libera recursos
emu_save_state_shutdown(state);
```

### 15.2 Criptografia com chave aleatória

```c
// Inicializa contexto de save state
emu_save_state_t* state = emu_save_state_init(EMU_PLATFORM_MEGA_DRIVE, rom_data, rom_size);

// Configura criptografia com chave aleatória
emu_encryption_config_t crypto_config = {
    .method = EMU_CRYPT_AES256_CBC,
    .derive_from_password = false
};

// Gera chave aleatória
emu_generate_random_bytes(crypto_config.key, 32);

// Aplica configuração de criptografia
emu_save_state_set_encryption(state, &crypto_config);

// Salva com criptografia
emu_save_options_t options = {
    .flags = EMU_SAVE_OPT_ENCRYPT,
};

// Salva o arquivo criptografado
emu_save_state_save(state, "jogo_secreto.state", &options);

// Exporta a chave para uso futuro
emu_crypto_export_key("jogo_secreto.key", &crypto_config, true);

// Libera recursos
emu_save_state_shutdown(state);
```

## 16. Considerações Futuras

Para versões futuras, estão sendo consideradas:

- Suporte a outros algoritmos (ChaCha20-Poly1305)
- Proteção com TPM/Secure Enclave em plataformas suportadas
- Integração com sistemas de gestão de identidade
- Recuperação de chaves via sistema de perguntas de segurança

## 17. Referências

1. NIST FIPS 197: Advanced Encryption Standard (AES)
2. NIST SP 800-38A: Recommendation for Block Cipher Modes of Operation
3. NIST SP 800-38D: Recommendation for Block Cipher Modes of Operation: GCM
4. RFC 2898: PKCS #5: Password-Based Cryptography Specification (PBKDF2)
5. RFC 4868: HMAC-SHA256 (SHA-224, SHA-256, SHA-384, and SHA-512)
