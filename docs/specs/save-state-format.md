# Formato do Arquivo de Save State

Este documento descreve o formato binário dos arquivos de save state do Mega_Emu.

## Visão Geral

O formato de arquivo de save state do Mega_Emu foi projetado com os seguintes objetivos:

1. **Portabilidade**: Garantir que os saves funcionem em diferentes plataformas
2. **Flexibilidade**: Suportar diferentes plataformas emuladas
3. **Eficiência**: Minimizar o tamanho dos arquivos através de compressão
4. **Robustez**: Incluir verificações para evitar corrupção de dados
5. **Extensibilidade**: Permitir adições futuras sem quebrar compatibilidade
6. **Segurança**: Opções para criptografia e validação
7. **Integração**: Suporte para sincronização com serviços de nuvem

## Estrutura do Arquivo

Um arquivo de save state é estruturado como um formato binário com as seguintes seções:

```
┌─────────────────────────────────────┐
│            ASSINATURA               │ 8 bytes
├─────────────────────────────────────┤
│          CABEÇALHO PRINCIPAL        │ 64 bytes
├─────────────────────────────────────┤
│        TABELA DE METADADOS          │ Tamanho variável
├─────────────────────────────────────┤
│        THUMBNAIL (OPCIONAL)         │ Tamanho variável
├─────────────────────────────────────┤
│        TABELA DE REGIÕES            │ Tamanho variável
├─────────────────────────────────────┤
│        DADOS DAS REGIÕES            │ Tamanho variável
├─────────────────────────────────────┤
│          DADOS ESPECÍFICOS          │ Tamanho variável
│          DA PLATAFORMA              │
├─────────────────────────────────────┤
│        DADOS DE INTEGRAÇÃO          │ Tamanho variável
│           COM NUVEM                 │
├─────────────────────────────────────┤
│             CHECKSUM                │ 32 bytes
└─────────────────────────────────────┘
```

### Assinatura

A assinatura consiste em 8 bytes que identificam o arquivo como um save state do Mega_Emu:

```
4D 65 67 61 45 6D 75 53  // "MegaEmuS" em ASCII
```

### Cabeçalho Principal

O cabeçalho principal contém informações gerais sobre o save state:

| Offset | Tamanho | Descrição |
|--------|---------|-----------|
| 0x00   | 4       | Versão do formato (uint32_t, little-endian) |
| 0x04   | 1       | ID da plataforma (uint8_t) |
| 0x05   | 1       | Flags (uint8_t) |
| 0x06   | 1       | Método de criptografia (uint8_t) |
| 0x07   | 1       | Status de nuvem (uint8_t) |
| 0x08   | 8       | Timestamp de criação (uint64_t, little-endian) |
| 0x10   | 4       | Tamanho total do arquivo (uint32_t, little-endian) |
| 0x14   | 4       | Offset da tabela de metadados (uint32_t, little-endian) |
| 0x18   | 4       | Tamanho da tabela de metadados (uint32_t, little-endian) |
| 0x1C   | 4       | Offset da thumbnail (uint32_t, little-endian, 0 se não existir) |
| 0x20   | 4       | Tamanho da thumbnail (uint32_t, little-endian, 0 se não existir) |
| 0x24   | 4       | Offset da tabela de regiões (uint32_t, little-endian) |
| 0x28   | 4       | Número de regiões (uint32_t, little-endian) |
| 0x2C   | 4       | Offset dos dados da plataforma (uint32_t, little-endian) |
| 0x30   | 4       | Tamanho dos dados da plataforma (uint32_t, little-endian) |
| 0x34   | 4       | Offset dos dados de nuvem (uint32_t, little-endian, 0 se não existir) |
| 0x38   | 4       | Tamanho dos dados de nuvem (uint32_t, little-endian, 0 se não existir) |
| 0x3C   | 4       | Soma de verificação do cabeçalho (CRC32) |

#### Flags

| Bit | Descrição |
|-----|-----------|
| 0   | Compressão ativada (0 = sem compressão, 1 = com compressão) |
| 1   | Compressão delta (0 = compressão normal, 1 = compressão delta) |
| 2   | Criptografia ativada (0 = sem criptografia, 1 = com criptografia) |
| 3   | Inclui thumbnail (0 = sem thumbnail, 1 = com thumbnail) |
| 4   | Integração com nuvem (0 = sem dados de nuvem, 1 = com dados de nuvem) |
| 5   | Verificação de autenticidade (0 = sem verificação, 1 = com verificação) |
| 6-7 | Reservado para uso futuro |

#### Métodos de Criptografia

| Valor | Descrição |
|-------|-----------|
| 0x00  | Sem criptografia |
| 0x01  | AES-256 em modo CBC |
| 0x02  | AES-256 em modo GCM (com autenticação) |
| 0x03-0xFF | Reservado para uso futuro |

#### Status de Nuvem

| Valor | Descrição |
|-------|-----------|
| 0x00  | Sem integração com nuvem |
| 0x01  | Sincronizado com nuvem |
| 0x02  | Alterado localmente (pendente de upload) |
| 0x03  | Versão mais recente disponível na nuvem |
| 0x04  | Conflito (versão local e na nuvem diferentes) |
| 0x05-0xFF | Reservado para uso futuro |

### Tabela de Metadados

A tabela de metadados contém pares chave-valor para informações adicionais:

```
┌─────────────────────────────────────┐
│          NÚMERO DE ENTRADAS         │ 4 bytes
├─────────────────────────────────────┤
│            METADADO 1               │ Tamanho variável
├─────────────────────────────────────┤
│            METADADO 2               │ Tamanho variável
├─────────────────────────────────────┤
│               ...                   │
└─────────────────────────────────────┘
```

Cada entrada de metadado segue o formato:

```
┌─────────────────────────────────────┐
│         TAMANHO DA CHAVE            │ 2 bytes
├─────────────────────────────────────┤
│           CHAVE (UTF-8)             │ Tamanho variável
├─────────────────────────────────────┤
│         TAMANHO DO VALOR            │ 4 bytes
├─────────────────────────────────────┤
│           VALOR (UTF-8)             │ Tamanho variável
└─────────────────────────────────────┘
```

Metadados obrigatórios:

| Chave | Descrição |
|-------|-----------|
| "rom_name" | Nome da ROM |
| "rom_hash" | Hash SHA-256 da ROM (representação hexadecimal) |

Metadados opcionais comuns:

| Chave | Descrição |
|-------|-----------|
| "description" | Descrição do save state |
| "tags" | Tags separadas por vírgula |
| "version" | Versão do emulador |
| "creator" | Ferramenta que criou o save state |
| "platform_region" | Região da plataforma (NTSC, PAL, etc.) |
| "game_timestamp" | Tempo de jogo no momento do salvamento |
| "user_id" | ID do usuário (para sistemas multi-usuário) |

### Thumbnail

Se presente (bit 3 da flag definido), a thumbnail é armazenada como:

```
┌─────────────────────────────────────┐
│             LARGURA                 │ 2 bytes
├─────────────────────────────────────┤
│              ALTURA                 │ 2 bytes
├─────────────────────────────────────┤
│             FORMATO                 │ 1 byte
├─────────────────────────────────────┤
│         TAMANHO DA IMAGEM           │ 4 bytes
├─────────────────────────────────────┤
│          DADOS DA IMAGEM            │ Tamanho variável
│          (formato WebP)             │
└─────────────────────────────────────┘
```

Formatos de thumbnail:

| Valor | Formato |
|-------|---------|
| 0x01  | RGB565 (16-bit) |
| 0x02  | RGB888 (24-bit) |
| 0x03  | RGBA8888 (32-bit) |
| 0x04  | WebP (comprimido) |

### Tabela de Regiões

A tabela de regiões lista todas as regiões de memória armazenadas:

```
┌─────────────────────────────────────┐
│          REGIÃO 1                   │ 80 bytes
├─────────────────────────────────────┤
│          REGIÃO 2                   │ 80 bytes
├─────────────────────────────────────┤
│            ...                      │
└─────────────────────────────────────┘
```

Cada entrada de região tem o formato:

| Offset | Tamanho | Descrição |
|--------|---------|-----------|
| 0x00   | 64      | Nome da região (string terminada com nulo) |
| 0x40   | 4       | Offset dos dados da região (uint32_t, relativo ao início dos dados das regiões) |
| 0x44   | 4       | Tamanho original (uint32_t, tamanho antes da compressão) |
| 0x48   | 4       | Tamanho armazenado (uint32_t, tamanho após compressão) |
| 0x4C   | 1       | Flags da região |
| 0x4D   | 3       | Reservado para alinhamento |

Flags da região:

| Bit | Descrição |
|-----|-----------|
| 0   | Compressão ativada (0 = sem compressão, 1 = com compressão) |
| 1   | Compressão delta (0 = compressão normal, 1 = compressão delta) |
| 2   | Criptografia individual (0 = segue configuração global, 1 = configuração específica) |
| 3-7 | Reservado para uso futuro |

### Dados das Regiões

Os dados das regiões são armazenados sequencialmente após a tabela de regiões. Cada região pode estar individualmente comprimida usando o algoritmo DEFLATE (zlib) e/ou criptografada.

```
┌─────────────────────────────────────┐
│       DADOS DA REGIÃO 1             │ Tamanho variável
├─────────────────────────────────────┤
│       DADOS DA REGIÃO 2             │ Tamanho variável
├─────────────────────────────────────┤
│               ...                   │
└─────────────────────────────────────┘
```

### Dados Específicos da Plataforma

Esta seção contém dados específicos da plataforma que não se encaixam no modelo de regiões de memória:

```
┌─────────────────────────────────────┐
│           ASSINATURA                │ 4 bytes
├─────────────────────────────────────┤
│             VERSÃO                  │ 4 bytes
├─────────────────────────────────────┤
│        DADOS ESPECÍFICOS            │ Tamanho variável
└─────────────────────────────────────┘
```

Assinaturas por plataforma:

| Plataforma | Assinatura |
|------------|------------|
| Mega Drive | "MDST" |
| NES | "NEST" |
| Master System | "SMST" |
| SNES | "SNST" |

O formato dos dados específicos da plataforma é definido separadamente para cada plataforma.

### Dados de Integração com Nuvem

Se presente (bit 4 da flag definido), esta seção contém informações para sincronização com serviços de nuvem:

```
┌─────────────────────────────────────┐
│           PROVEDOR                  │ 1 byte
├─────────────────────────────────────┤
│        TAMANHO DO URL               │ 2 bytes
├─────────────────────────────────────┤
│         URL NA NUVEM                │ Tamanho variável
├─────────────────────────────────────┤
│       TIMESTAMP DE SINCRONIZAÇÃO    │ 8 bytes
├─────────────────────────────────────┤
│         VERSÃO REMOTA               │ 4 bytes
├─────────────────────────────────────┤
│        METADADOS DE NUVEM           │ Tamanho variável
└─────────────────────────────────────┘
```

Provedores de nuvem:

| Valor | Provedor |
|-------|----------|
| 0x00  | Nenhum |
| 0x01  | Dropbox |
| 0x02  | Google Drive |
| 0x03  | Microsoft OneDrive |
| 0xFF  | Personalizado |

Os metadados de nuvem seguem o mesmo formato da tabela de metadados principal, mas com informações específicas do serviço de nuvem.

### Checksum

O arquivo termina com um hash SHA-256 calculado sobre todo o conteúdo do arquivo, excluindo os próprios 32 bytes do checksum.

## Criptografia

### Estrutura de Dados Criptografados

Quando a criptografia está ativada (bit 2 da flag definido), as seções criptografadas seguem o formato:

```
┌─────────────────────────────────────┐
│         VETOR DE INICIALIZAÇÃO      │ 16 bytes
├─────────────────────────────────────┤
│        DADOS CRIPTOGRAFADOS         │ Tamanho variável
├─────────────────────────────────────┤
│         TAG DE AUTENTICAÇÃO         │ 16 bytes (apenas para AES-GCM)
└─────────────────────────────────────┘
```

As seções que podem ser criptografadas incluem:

- Tabela de metadados
- Dados das regiões
- Dados específicos da plataforma
- Dados de integração com nuvem

O cabeçalho principal e a thumbnail nunca são criptografados para permitir identificação e visualização sem necessidade de descriptografia.

### Processo de Criptografia AES-256

1. **Derivação de Chave**:
   - Se baseada em senha: A chave é derivada usando PBKDF2 com 10.000 iterações
   - Salt de 16 bytes armazenado nos metadados

2. **Modos de Criptografia**:
   - **CBC (Cipher Block Chaining)**:
     - IV aleatório de 16 bytes
     - Padding PKCS#7

   - **GCM (Galois/Counter Mode)**:
     - IV aleatório de 12 bytes
     - Sem padding (modo CTR subjacente)
     - Tag de autenticação de 16 bytes

3. **Processo de Criptografia**:
   - Compressão aplicada antes da criptografia
   - Cada seção criptografada tem seu próprio IV
   - Cabeçalho e thumbnail permanecem não criptografados

## Compressão

### Compressão Regular

Quando a compressão está ativada (bit 0 da flag definido), cada região é individualmente comprimida usando o algoritmo DEFLATE (zlib). O nível de compressão pode variar de 1 (mais rápido) a 9 (melhor compressão).

### Compressão Delta

Quando a compressão delta está ativada (bit 1 da flag definido), o sistema armazena apenas as diferenças em relação a um estado base. Este modo é especialmente útil para o sistema de rewind, onde muitas regiões permanecem inalteradas entre frames consecutivos.

## Sincronização com Nuvem

O sistema de save state suporta sincronização com serviços de nuvem para permitir que os usuários acessem seus saves em diferentes dispositivos.

### Identificação de Versões

Cada save state tem um identificador único e uma versão que são usados para resolução de conflitos:

- **Identificador**: Hash SHA-256 do nome da ROM, checksum da ROM e timestamp de criação
- **Versão**: Contador incrementado a cada modificação

### Resolução de Conflitos

Quando são detectadas versões diferentes do mesmo save state:

1. **Automática**: Baseada em timestamps e versões
2. **Manual**: Usuário seleciona qual versão manter
3. **Unificação**: Criação de um novo save state combinando os dois quando possível

## Compatibilidade entre Versões

O sistema mantém compatibilidade com versões anteriores do formato:

### Versão 1.0 (0x0100)

- Formato original sem suporte a thumbnails ou criptografia
- Suporte apenas para metadados básicos

### Versão 2.0 (0x0200)

- Adição de suporte a thumbnails
- Suporte a compressão delta
- Metadados expandidos

### Versão 3.0 (0x0300)

- Adição de suporte a criptografia AES-256
- Integração com serviços de nuvem
- Autenticação de dados
- Suporte a thumbnails WebP com marcações

## Exemplo de Implementação

### Salvar com Criptografia

```c
// Configurar cabeçalho
save_header.version = 0x0300;
save_header.platform_id = EMU_PLATFORM_MEGA_DRIVE;
save_header.flags = EMU_SAVE_FLAG_COMPRESS | EMU_SAVE_FLAG_ENCRYPT | EMU_SAVE_FLAG_THUMBNAIL;
save_header.encryption_method = EMU_CRYPT_AES256_GCM;
save_header.timestamp = get_current_timestamp();

// Configurar criptografia
uint8_t key[32];
uint8_t iv[16];
derive_key_from_password("senha_secreta", key, salt);
generate_random_iv(iv);

// Para cada região
for (int i = 0; i < num_regions; i++) {
    // Comprimir
    compressed_data = compress_zlib(region[i].data, region[i].size, &compressed_size);

    // Criptografar
    encrypted_data = encrypt_aes256_gcm(compressed_data, compressed_size, key, iv, &encrypted_size);

    // Escrever
    write_data(file, encrypted_data, encrypted_size);
}

// Calcular e escrever checksum
uint8_t checksum[32];
calculate_sha256(file_data, file_size - 32, checksum);
write_data(file, checksum, 32);
```

### Carregar com Descriptografia

```c
// Ler cabeçalho
read_data(file, &save_header, sizeof(save_header));

// Verificar versão e flags
if (save_header.version > EMU_CURRENT_FORMAT_VERSION) {
    return EMU_ERROR_VERSION_NOT_SUPPORTED;
}

// Verificar criptografia
if (save_header.flags & EMU_SAVE_FLAG_ENCRYPT) {
    // Obter senha do usuário e derivar chave
    uint8_t key[32];
    derive_key_from_password(password, key, header.salt);

    // Para cada região
    for (int i = 0; i < header.num_regions; i++) {
        // Ler IV e dados criptografados
        read_data(file, iv, 16);
        read_data(file, encrypted_data, region[i].encrypted_size);

        // Descriptografar
        decrypted_data = decrypt_aes256_gcm(encrypted_data, region[i].encrypted_size,
                                           key, iv, &decrypted_size);

        // Descomprimir
        if (save_header.flags & EMU_SAVE_FLAG_COMPRESS) {
            original_data = decompress_zlib(decrypted_data, decrypted_size,
                                           region[i].original_size);
        } else {
            original_data = decrypted_data;
        }

        // Restaurar para o emulador
        memcpy(region[i].target, original_data, region[i].original_size);
    }
}

// Verificar checksum
uint8_t file_checksum[32];
uint8_t calculated_checksum[32];
read_data(file, file_checksum, 32);
calculate_sha256(file_data, file_size - 32, calculated_checksum);

if (memcmp(file_checksum, calculated_checksum, 32) != 0) {
    return EMU_ERROR_INVALID_CHECKSUM;
}
```

## Considerações de Segurança

1. **Validação de Entrada**: Todos os campos devem ser validados antes do uso.
2. **Limites de Tamanho**: Impor limites rígidos em todos os campos de tamanho para evitar alocações de memória excessivas.
3. **Verificações de Integridade**: Sempre verificar checksums para garantir que o arquivo não foi corrompido.
4. **Segurança da Criptografia**: Utilizar bibliotecas de criptografia robustas e bem testadas.

## Questões de Compatibilidade Futura

Para manter a compatibilidade com versões futuras do formato:

1. Ignore campos desconhecidos durante a leitura.
2. Preserve campos desconhecidos durante a escrita.
3. Verifique a versão do formato e falhe graciosamente se não for suportada.
4. Utilize os campos reservados apenas em novas versões do formato.

## Referências

- [RFC 1950](https://tools.ietf.org/html/rfc1950) - Especificação zlib
- [RFC 1951](https://tools.ietf.org/html/rfc1951) - Especificação DEFLATE
- [WebP](https://developers.google.com/speed/webp/) - Formato de imagem WebP
- [FIPS 197](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197.pdf) - Especificação AES
- [FIPS 180-4](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf) - Especificação SHA-256
