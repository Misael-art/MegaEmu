# Guia de Migração para o Sistema de Save States Unificado

Este documento serve como guia para a migração das implementações existentes de save states para o novo sistema unificado do Mega_Emu.

## Visão Geral

O processo de migração visa consolidar as diferentes implementações de save states das plataformas Mega Drive, NES e Master System em uma API unificada. A migração ocorrerá em fases, permitindo uma transição gradual sem impactar negativamente a estabilidade do emulador.

## Status Atual

Cada plataforma atualmente implementa seu próprio sistema de save state com diferentes níveis de funcionalidade:

| Plataforma | Localização Atual | Versão Formato | Status |
|------------|-------------------|----------------|--------|
| Mega Drive | `src/platforms/megadrive/state/` | 1.3 | 85% completo |
| NES | `src/platforms/nes/save/` | 2.0 | 90% completo |
| Master System | `src/platforms/mastersystem/state/` | 1.0 | 95% completo |
| SNES | `src/platforms/snes/` | 0.5 | 40% completo |

## Novos Recursos a Implementar

Durante a migração, serão adicionados novos recursos importantes ao sistema unificado:

### 1. Encriptação AES-256

Todos os save states poderão ser criptografados usando o algoritmo AES-256 para garantir a segurança dos dados e prevenir manipulação não autorizada.

**Implementação**:

- Suporte aos modos CBC e GCM (este último com autenticação)
- Derivação de chave baseada em senha usando PBKDF2
- Verificação de integridade via HMAC
- Proteção individual para regiões sensíveis

### 2. Integração com Serviços de Nuvem

O novo sistema suportará sincronização com serviços de nuvem como Dropbox, Google Drive e OneDrive.

**Implementação**:

- Sincronização automática configurável
- Resolução de conflitos
- Metadados estendidos para controle de versão
- Interface extensível para provedores personalizados

## Estrutura Final

Após a migração, a estrutura do código será reorganizada:

```
src/
├── core/
│   ├── save_state.h           # Interface pública unificada
│   ├── save_state.c           # Implementação principal
│   ├── save_state_format.h    # Definições de formato
│   ├── save_state_crypto.h    # Interface de criptografia
│   ├── save_state_crypto.c    # Implementação de criptografia
│   ├── save_state_cloud.h     # Interface de nuvem
│   ├── save_state_cloud.c     # Implementação de nuvem
│   └── save_state_private.h   # Interfaces internas
│
├── platforms/
│   ├── megadrive/
│   │   └── state/
│   │       ├── md_save_state.h    # Adaptador específico MD
│   │       └── md_save_state.c    # Implementação específica MD
│   │
│   ├── nes/
│   │   └── save/
│   │       ├── nes_save_state.h   # Adaptador específico NES
│   │       └── nes_save_state.c   # Implementação específica NES
│   │
│   └── mastersystem/
│       └── state/
│           ├── sms_save_state.h   # Adaptador específico SMS
│           └── sms_save_state.c   # Implementação específica SMS
```

## Etapas de Migração

### Fase 1: Preparação e Análise

1. **Auditar Implementações Existentes**
   - Documentar as diferenças entre as implementações atuais
   - Identificar recursos exclusivos que precisam ser preservados
   - Mapear dependências internas em cada plataforma

2. **Criar Ambiente de Teste**
   - Implementar testes de regressão para verificar compatibilidade
   - Criar conjunto de save states de referência para testes

### Fase 2: Desenvolvimento da Camada Core

1. **Implementar Camada Core**
   - Desenvolver a interface pública em `src/core/save_state.h`
   - Implementar a funcionalidade principal em `src/core/save_state.c`
   - Definir estruturas comuns em `src/core/save_state_format.h`

2. **Implementar Sistema de Criptografia**
   - Desenvolver interface de criptografia em `src/core/save_state_crypto.h`
   - Implementar AES-256 nos modos CBC e GCM
   - Criar sistema de derivação de chave PBKDF2 para uso com senhas
   - Implementar verificação de integridade

3. **Implementar Integração com Nuvem**
   - Desenvolver interface em `src/core/save_state_cloud.h`
   - Implementar conectores para Dropbox, Google Drive e OneDrive
   - Criar sistema de resolução de conflitos
   - Desenvolver mecanismo de sincronização assíncrona

4. **Validar Funcionalidade Core**
   - Testar o ciclo de vida completo (init/save/load/shutdown)
   - Verificar gerenciamento de memória
   - Testar compressão, thumbnails e recursos de segurança
   - Validar sincronização com nuvem

### Fase 3: Adaptadores para Plataformas

Para cada plataforma, o processo de migração seguirá estas etapas:

1. **Implementar Adaptador**
   - Criar wrapper em `<platform>_save_state.h/.c`
   - Manter compatibilidade com chamadas existentes

2. **Refatorar Componentes da Plataforma**
   - Atualizar o código do emulador da plataforma para usar a nova API
   - Implementar hooks para o ciclo de vida do save state

3. **Verificar Compatibilidade**
   - Testar compatibilidade com save states antigos
   - Verificar a preservação da funcionalidade específica da plataforma

### Fase 4: Integração Frontend

1. **Atualizar APIs Públicas**
   - Revisar API do websocket e REST
   - Atualizar métodos do frontend para utilizar o sistema unificado

2. **Atualizar UI**
   - Implementar visualização de thumbnails
   - Adicionar campos de metadados na interface de save states
   - Implementar controles para recursos avançados (rewind, etc.)
   - Criar interface para gerenciamento de criptografia
   - Desenvolver UI para sincronização com nuvem

### Fase 5: Finalização

1. **Testes Abrangentes**
   - Executar testes de regressão em todas as plataformas
   - Verificar desempenho e uso de memória
   - Testar segurança da implementação criptográfica
   - Validar integração com nuvem

2. **Documentação**
   - Atualizar documentação para desenvolvedores
   - Criar documentação para usuários finais
   - Documentar APIs de criptografia e nuvem

3. **Limpeza**
   - Remover código legado após período de transição
   - Resolver problemas pendentes e otimizar implementação

## Guia de Migração por Plataforma

### Mega Drive

```c
// ANTES
md_save_state_t* state = md_save_state_create();
md_save_state_register_components(state, &md_context);
md_save_state_save(state, "savefile.md", 0);
md_save_state_destroy(state);

// DEPOIS
emu_save_state_t* state = emu_save_state_init(EMU_PLATFORM_MEGA_DRIVE, md_context.rom_data, md_context.rom_size);
md_save_state_register(state);  // Registra automaticamente todos os componentes
emu_save_options_t options = {
    .flags = EMU_SAVE_OPT_COMPRESS | EMU_SAVE_OPT_THUMBNAIL
};
emu_save_state_save(state, "savefile.state", &options);
emu_save_state_shutdown(state);
```

#### Detalhes Específicos

- A compressão delta precisa ser preservada
- Metadados extras específicos do MD precisam ser migrados
- Implementação de thumbnail existente deve ser convertida para WebP

#### Usando Criptografia

```c
// Configuração de criptografia
emu_encryption_config_t crypto_config = {
    .method = EMU_CRYPT_AES256_GCM,  // GCM para autenticação
    .derive_from_password = true,
    .password = "senha_do_usuario",
    .kdf_iterations = 10000
};

// Aplicar configuração
emu_save_state_set_encryption(state, &crypto_config);

// Opções salvar com criptografia
emu_save_options_t options = {
    .flags = EMU_SAVE_OPT_COMPRESS | EMU_SAVE_OPT_THUMBNAIL | EMU_SAVE_OPT_ENCRYPT,
    .compression_level = 6,
    .encryption_method = EMU_CRYPT_AES256_GCM
};

// Salvar com criptografia
emu_save_state_save(state, "savefile.state", &options);
```

#### Usando Integração com Nuvem

```c
// Configuração nuvem
emu_cloud_config_t cloud_config = {
    .provider = EMU_CLOUD_GOOGLE_DRIVE,
    .auth_token = "oauth_token_obtido_na_autenticacao",
    .folder_path = "/MegaEmu/SaveStates",
    .auto_sync = true,
    .sync_interval = 300  // 5 minutos
};

// Configurar sincronização
emu_save_state_cloud_configure(state, &cloud_config);

// Opções com sincronização
emu_save_options_t options = {
    .flags = EMU_SAVE_OPT_COMPRESS | EMU_SAVE_OPT_THUMBNAIL | EMU_SAVE_OPT_CLOUD_SYNC,
    .compression_level = 6
};

// Salvar e sincronizar
emu_save_state_save(state, "savefile.state", &options);
```

### NES

```c
// ANTES
nes_savestate_context_t context;
nes_savestate_init(&context, &nes_context);
nes_savestate_save(&context, "savefile.nes");
nes_savestate_free(&context);

// DEPOIS
emu_save_state_t* state = emu_save_state_init(EMU_PLATFORM_NES, nes_context.rom_data, nes_context.rom_size);
nes_save_state_register(state);
emu_save_state_save(state, "savefile.state", NULL);
emu_save_state_shutdown(state);
```

#### Detalhes Específicos

- Sistema de rewind precisa ser integrado ao novo framework
- Compatibilidade com mappers especiais deve ser preservada
- Implementação existente de validation precisa ser migrada

#### Usando Criptografia e Nuvem

```c
// Configuração combinada
emu_encryption_config_t crypto_config = {
    .method = EMU_CRYPT_AES256_CBC
};

// Gerar chave aleatória em vez de usar senha
uint8_t random_key[32];
emu_generate_random_bytes(random_key, sizeof(random_key));
memcpy(crypto_config.key, random_key, sizeof(random_key));

// Configurar criptografia
emu_save_state_set_encryption(state, &crypto_config);

// Configurar integração com Dropbox
emu_cloud_config_t cloud_config = {
    .provider = EMU_CLOUD_DROPBOX,
    .auth_token = dropbox_token,
    .folder_path = "/Apps/MegaEmu",
    .auto_sync = true
};

// Configurar nuvem
emu_save_state_cloud_configure(state, &cloud_config);

// Opções salvar
emu_save_options_t options = {
    .flags = EMU_SAVE_OPT_ENCRYPT | EMU_SAVE_OPT_CLOUD_SYNC,
    .encryption_method = EMU_CRYPT_AES256_CBC,
    .cloud_provider = EMU_CLOUD_DROPBOX
};

// Salvar
emu_save_state_save(state, "savefile.state", &options);
```

### Master System

```c
// ANTES
sms_state_t state;
sms_state_init(&state);
sms_state_save(&sms_context, &state, "savefile.sms");
sms_state_close(&state);

// DEPOIS
emu_save_state_t* state = emu_save_state_init(EMU_PLATFORM_MASTER_SYSTEM, sms_context.rom_data, sms_context.rom_size);
sms_save_state_register(state);
emu_save_state_save(state, "savefile.state", NULL);
emu_save_state_shutdown(state);
```

#### Detalhes Específicos

- Sistema de validação de ROM existente deve ser integrado
- Compatibilidade com Game Gear deve ser preservada
- Metadados de região precisam ser preservados

#### Usando Webhook Personalizado para Nuvem

```c
// Função de upload personalizada
void custom_upload_handler(const char* local_path, const char* remote_path, void* user_data) {
    // Implementação personalizada de upload
    http_client_t* client = (http_client_t*)user_data;
    http_upload_file(client, local_path, remote_path);
}

// Função de download personalizada
void custom_download_handler(const char* remote_path, const char* local_path, void* user_data) {
    // Implementação personalizada de download
    http_client_t* client = (http_client_t*)user_data;
    http_download_file(client, remote_path, local_path);
}

// Configurar provedor de nuvem personalizado
emu_cloud_config_t cloud_config = {
    .provider = EMU_CLOUD_CUSTOM,
    .user_data = http_client,
    .custom_upload = custom_upload_handler,
    .custom_download = custom_download_handler
};

// Configurar nuvem
emu_save_state_cloud_configure(state, &cloud_config);

// Opções salvar
emu_save_options_t options = {
    .flags = EMU_SAVE_OPT_CLOUD_SYNC
};

// Salvar
emu_save_state_save(state, "savefile.state", &options);
```

## Convertendo Formatos de Arquivo

Durante o período de transição, o emulador suportará automaticamente a conversão de formatos antigos:

1. **Detecção Automática**
   - Detectar formato do arquivo com base na assinatura
   - Rotear para o carregador apropriado

2. **Conversão On-The-Fly**
   - Converter save states antigos para o novo formato durante o carregamento
   - Oferecer opção para resalvar no novo formato

3. **Ferramenta de Migração em Lote**
   - Fornecer utilitário para converter todos os saves de uma pasta
   - Preservar arquivos originais como backup

4. **Migração de Arquivos Criptografados**
   - Detectar automaticamente save states criptografados
   - Solicitar senha quando necessário
   - Oferecer opção para migrar configurações de criptografia

## Migração de Metadados

Cada plataforma tem metadados específicos que precisam ser preservados:

| Plataforma | Metadados Específicos |
|------------|------------------------|
| Mega Drive | Região, valores VDP, flags de hardware |
| NES | Informações do mapper, flags PPU |
| Master System | Flags de região, modo de compatibilidade |

Estes metadados serão armazenados na seção de metadados expandidos e na seção de dados específicos da plataforma.

## Novos Metadados

Além dos metadados específicos da plataforma, o novo sistema introduz metadados adicionais:

### Metadados de Segurança

```json
{
  "crypto_method": "aes256-gcm",
  "crypto_kdf": "pbkdf2",
  "crypto_iterations": 10000,
  "crypto_salt": "base64_encoded_salt",
  "integrity_verification": "hmac-sha256"
}
```

### Metadados de Nuvem

```json
{
  "cloud_provider": "google_drive",
  "cloud_path": "/Apps/MegaEmu/SaveStates",
  "cloud_id": "file_id_on_cloud",
  "cloud_version": 3,
  "cloud_last_sync": "2025-03-15T14:30:45Z",
  "cloud_status": "synced"
}
```

## Compatibilidade com Frontend

A migração da API afeta vários componentes do frontend:

1. **Gerenciador de Save States**
   - Atualizar para utilizar a nova API unificada
   - Adicionar suporte para exibição de thumbnails e metadados

2. **Sistema de Rewind**
   - Integrar com o novo framework de rewind da API core
   - Preservar controles da interface existente

3. **Menu de Opções**
   - Adicionar configurações para novos recursos (compressão, criptografia)
   - Manter compatibilidade com configurações existentes

4. **Interface de Segurança**
   - Adicionar controles para configuração de criptografia
   - Implementar diálogo para entrada de senha
   - Adicionar opções para gerenciamento de chaves

5. **Interface de Nuvem**
   - Adicionar autenticação com provedores de nuvem
   - Implementar visualização de arquivos na nuvem
   - Adicionar controles para sincronização manual
   - Implementar sistema de resolução de conflitos

## Cronograma de Migração Atualizado

| Fase | Duração | Período |
|------|---------|---------|
| Preparação e Análise | 1 semana | Semana 1 |
| Desenvolvimento da Camada Core | 2 semanas | Semanas 2-3 |
| Implementação de Criptografia | 1 semana | Semana 4 |
| Implementação de Nuvem | 2 semanas | Semanas 5-6 |
| Adaptadores Mega Drive | 1 semana | Semana 7 |
| Adaptadores NES | 1 semana | Semana 8 |
| Adaptadores Master System | 1 semana | Semana 9 |
| Integração Frontend | 2 semanas | Semanas 10-11 |
| Testes e Finalização | 2 semanas | Semanas 12-13 |

## Resolução de Problemas Comuns

### Incompatibilidade com Save States Antigos

- **Sintoma**: Save states antigos não carregam ou causam crash
- **Solução**: Verificar detecção de formato, confirmar hash da ROM, verificar registro de regiões

### Falhas de Serialização de Componentes

- **Sintoma**: Alguns componentes não são salvos/carregados corretamente
- **Solução**: Verificar versionamento de componentes, confirmar ordem de registro, verificar callbacks

### Problemas de Criptografia

- **Sintoma**: Falha ao carregar arquivos criptografados
- **Soluções**:
  - Verificar se a senha/chave está correta
  - Confirmar se o método de criptografia é suportado
  - Verificar a tag de autenticação para arquivos GCM
  - Confirmar integridade do arquivo

### Problemas de Sincronização com Nuvem

- **Sintoma**: Falha na sincronização com serviços de nuvem
- **Soluções**:
  - Verificar token de autenticação
  - Confirmar conectividade de rede
  - Verificar permissões de acesso ao armazenamento
  - Verificar resolução de conflitos

## Recursos Adicionais

### Ferramentas de Teste de Criptografia

- `crypto_tester`: Utilitário para testar implementação de criptografia
- `encryption_benchmark`: Ferramenta para avaliar desempenho de diferentes algoritmos

### Ferramentas de Sincronização

- `cloud_sync_tester`: Utilitário para testar sincronização com nuvem
- `conflict_simulator`: Ferramenta para simular e testar resolução de conflitos

### Documentação Complementar

- [Especificação de Criptografia AES-256](../specs/save-state-crypto.md)
- [Guia de Integração com Nuvem](../specs/save-state-cloud.md)
- [Protocolo de Resolução de Conflitos](../specs/conflict-resolution.md)

## Contatos para Suporte

- Problemas de migração da API Core: [responsável pela API core]
- Questões específicas do Mega Drive: [responsável pelo MD]
- Questões específicas do NES: [responsável pelo NES]
- Questões específicas do Master System: [responsável pelo SMS]
- Integração com Frontend: [responsável pelo frontend]

## Referências

- [Especificação da API unificada de Save States](../api/save-states-api.md)
- [Formato do Arquivo de Save State](../specs/save-state-format.md)
- [Documentação do Sistema de Save States](../components/SAVE_STATES.md)
