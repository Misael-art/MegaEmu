# Especificação de Integração com Nuvem para Save States

**Versão:** 1.0
**Data:** 2025-03-18
**Status:** Desenvolvimento

## 1. Visão Geral

Este documento descreve a implementação da integração com serviços de nuvem para o sistema de save states do Mega_Emu, permitindo o armazenamento, sincronização e compartilhamento de save states entre diferentes dispositivos e plataformas através de serviços como Google Drive, Dropbox e OneDrive.

## 2. Objetivos

A integração com serviços de nuvem visa atender aos seguintes objetivos:

- Sincronizar save states automaticamente entre múltiplos dispositivos
- Fornecer backup seguro para save states importantes
- Permitir continuidade de jogo entre diferentes plataformas
- Facilitar o compartilhamento de save states entre usuários
- Gerenciar save states remotamente
- Garantir compatibilidade com diferentes provedores de nuvem

## 3. Provedores Suportados

### 3.1 Provedores Nativos

Os seguintes provedores de nuvem são suportados nativamente:

| Provedor      | ID                     | API Utilizada          | Autenticação   |
|---------------|------------------------|------------------------|----------------|
| Google Drive  | EMU_CLOUD_GOOGLE_DRIVE | Google Drive API v3    | OAuth 2.0      |
| Dropbox       | EMU_CLOUD_DROPBOX      | Dropbox API v2         | OAuth 2.0      |
| OneDrive      | EMU_CLOUD_ONEDRIVE     | Microsoft Graph API    | OAuth 2.0      |

### 3.2 Provedor Personalizado

Além dos provedores nativos, a implementação permite a integração com qualquer serviço de armazenamento através de callbacks personalizados:

- Upload personalizado
- Download personalizado
- Listagem personalizada
- Verificação de timestamp personalizada

Este mecanismo permite integração com:

- Servidores FTP
- Servidores WebDAV
- Serviços de armazenamento personalizados
- Repositórios Git

## 4. Arquitetura

### 4.1 Componentes Principais

A arquitetura da integração com nuvem consiste nos seguintes componentes:

```
+---------------------+
| Interface Unificada | <- emu_save_state_cloud_*
+---------------------+
        |
+---------------------+
| Gerenciador de Sync | <- Gerencia sincronização e conflitos
+---------------------+
        |
+-------------------------------+
| Adaptadores para Provedores   | <- Implementações específicas
+-------------------------------+
        |
+-------------------------------+
| APIs dos Serviços de Nuvem    | <- APIs externas
+-------------------------------+
```

### 4.2 Diagrama de Fluxo de Dados

```
Usuario +-------+ GUI +-------> Core Save State API +-----> Cloud API +---> Serviço
                                        ^                      ^           de Nuvem
                                        |                      |
                                        +-------> Criptografia +
```

## 5. Formato de Armazenamento em Nuvem

### 5.1 Estrutura de Diretórios

Os save states em nuvem são organizados em uma estrutura hierárquica:

```
/MegaEmu/SaveStates/
    ├── <game_id>/
    │   ├── auto/
    │   │   ├── auto_save_1.state
    │   │   └── ...
    │   ├── slot1.state
    │   ├── slot2.state
    │   └── ...
    └── <outro_game_id>/
        └── ...
```

Onde:

- `<game_id>` é um identificador único do jogo (hash ou nome do arquivo ROM)
- `auto/` contém save states automáticos
- Os arquivos `.state` são os save states, mantendo o formato descrito em `save-state-format.md`

### 5.2 Metadados de Nuvem

Cada save state sincronizado com a nuvem contém metadados adicionais:

```json
{
  "cloud_provider": "google_drive",
  "cloud_id": "file_id_on_cloud_service",
  "cloud_version": 3,
  "cloud_last_sync": "2025-03-15T14:30:45Z",
  "cloud_status": "synced",
  "cloud_user": "user@example.com",
  "cloud_device_origin": "desktop-win64"
}
```

Estes metadados são armazenados na seção de metadados do save state e ajudam a:

- Rastrear a origem do save state
- Gerenciar diferentes versões
- Resolver conflitos
- Facilitar buscas e filtragem

## 6. Fluxo de Sincronização

### 6.1 Upload de Save State

O processo de upload para a nuvem segue estes passos:

1. **Preparação**
   - Verificar conectividade com o serviço
   - Validar autenticação
   - Verificar existência do diretório remoto (criar se necessário)

2. **Verificação de Conflitos**
   - Verificar se o arquivo já existe remotamente
   - Comparar timestamps para detectar conflitos

3. **Upload**
   - Se o arquivo for criptografado, enviar como está
   - Se não for criptografado, considerar criptografia opcional
   - Incluir metadados de nuvem atualizados

4. **Confirmação**
   - Atualizar metadados locais com ID do arquivo remoto
   - Registrar timestamp de sincronização

### 6.2 Download de Save State

O processo de download da nuvem segue estes passos:

1. **Verificação**
   - Verificar conectividade com o serviço
   - Validar autenticação
   - Verificar existência do arquivo remoto

2. **Comparação**
   - Se o arquivo existir localmente, comparar timestamps
   - Decidir se o download é necessário com base em timestamps

3. **Download**
   - Baixar o arquivo para armazenamento local
   - Atualizar metadados de nuvem

4. **Validação**
   - Verificar integridade do arquivo (checksum)
   - Verificar se é um save state válido

### 6.3 Sincronização Automática

O sistema suporta sincronização automática periodicamente:

1. **Configuração**
   - Usuário define intervalo de sincronização
   - Configura quais jogos devem ser sincronizados

2. **Processo em Background**
   - Thread separada verifica atualizações periodicamente
   - Baixa novos save states ou uploads alterações

3. **Notificações**
   - Interface notifica o usuário sobre novas sincronizações
   - Alerta sobre conflitos que exigem intervenção

## 7. Gestão de Conflitos

### 7.1 Detecção de Conflitos

Os conflitos são detectados nas seguintes situações:

- Save state remoto e local foram modificados desde a última sincronização
- Mesmo save state foi modificado em dispositivos diferentes
- Timestamps inconsistentes entre versões

### 7.2 Estratégias de Resolução

O sistema oferece várias estratégias para resolver conflitos:

| Estratégia | ID | Descrição |
|------------|-------|-----------|
| Perguntar  | EMU_CLOUD_CONFLICT_ASK | Solicita ao usuário qual versão manter |
| Local      | EMU_CLOUD_CONFLICT_LOCAL | Sempre usa a versão local |
| Remota     | EMU_CLOUD_CONFLICT_REMOTE | Sempre usa a versão remota |
| Mais recente | EMU_CLOUD_CONFLICT_NEWEST | Usa a versão com timestamp mais recente |
| Mesclar    | EMU_CLOUD_CONFLICT_MERGE | Tenta mesclar os dados (para conflitos específicos) |

### 7.3 Interface de Resolução

Quando o sistema detecta um conflito e a estratégia é "Perguntar", ele:

1. Notifica o usuário sobre o conflito
2. Apresenta informações de ambas versões (local e remota)
3. Permite que o usuário selecione qual versão manter
4. Opcionalmente salva uma cópia da versão não escolhida

## 8. Autenticação e Segurança

### 8.1 Autenticação OAuth

A integração utiliza OAuth 2.0 para autenticação com provedores de nuvem:

1. **Fluxo de Autorização**
   - Aplicativo redireciona para página de autorização do provedor
   - Usuário concede acesso à aplicação
   - Provedor retorna código de autorização

2. **Troca de Token**
   - Aplicativo troca código por tokens de acesso e atualização
   - Token de acesso é usado para operações na API
   - Token de atualização é armazenado para renovação automática

3. **Renovação de Token**
   - Tokens de acesso expiram após um período
   - Sistema renova automaticamente usando o token de atualização

### 8.2 Armazenamento de Tokens

Os tokens são armazenados com segurança:

- Em sistemas desktop, usando o gerenciador de credenciais do sistema
- Em sistemas móveis, usando armazenamento seguro da plataforma
- Em sistemas web, usando armazenamento protegido do navegador

### 8.3 Permissões

O sistema solicita apenas as permissões necessárias:

| Provedor | Permissões Solicitadas |
|----------|------------------------|
| Google Drive | `drive.file` (acesso apenas a arquivos criados pelo app) |
| Dropbox | `files.content.write` e `files.content.read` |
| OneDrive | `Files.ReadWrite.AppFolder` |

### 8.4 Integração com Criptografia

A integração com nuvem funciona em conjunto com o sistema de criptografia:

- Save states são criptografados antes do upload (opcional)
- Chaves de criptografia nunca são enviadas para a nuvem
- Dados sensíveis podem ser protegidos mesmo em armazenamento compartilhado

## 9. API para Desenvolvedores

### 9.1 Configurando Integração com Nuvem

```c
// Configura integração com Google Drive
emu_cloud_config_t config = {
    .provider = EMU_CLOUD_GOOGLE_DRIVE,
    .auth_token = "oauth_token_obtido_anteriormente",
    .refresh_token = "oauth_refresh_token",
    .folder_path = "/MegaEmu/SaveStates",
    .auto_sync = true,
    .sync_interval = 300,  // 5 minutos
    .conflict_strategy = EMU_CLOUD_CONFLICT_ASK
};

// Aplicar configuração ao contexto de save state
emu_save_state_cloud_configure(state, &config);
```

### 9.2 Sincronização Manual

```c
// Sincronizar um save state específico
emu_save_state_cloud_sync(state, "savefile.state", true);  // true = upload

// Sincronizar todos os save states do diretório
emu_save_state_cloud_sync_all(state, "/caminho/para/savestates");
```

### 9.3 Listar Save States na Nuvem

```c
// Lista até 50 save states na nuvem
emu_cloud_file_info_t files[50];
size_t num_files = 0;

if (emu_save_state_cloud_list(state, files, 50, &num_files)) {
    for (size_t i = 0; i < num_files; i++) {
        printf("Save state: %s, Tamanho: %llu, Modificado: %llu\n",
               files[i].filename, files[i].size, files[i].modified_time);
    }
}
```

### 9.4 Provedor Personalizado

```c
// Definir callbacks personalizados
bool my_upload(const char* local, const char* remote, void* data) {
    // Implementação de upload personalizada
    return true;
}

bool my_download(const char* remote, const char* local, void* data) {
    // Implementação de download personalizada
    return true;
}

// Configurar provedor personalizado
emu_cloud_config_t custom_config = {
    .provider = EMU_CLOUD_CUSTOM,
    .custom_upload = my_upload,
    .custom_download = my_download,
    .custom_list = my_list_func,
    .custom_timestamp = my_timestamp_func,
    .user_data = my_context,  // passado para os callbacks
    .folder_path = "my_server/savestates",
    .auto_sync = false
};

emu_save_state_cloud_configure(state, &custom_config);
```

## 10. Indicadores de Status

A API oferece informações sobre o status da sincronização:

```c
// Verifica se há uma operação em andamento
int progress = 0;
if (emu_save_state_cloud_is_busy(state, &progress)) {
    printf("Sincronização em andamento: %d%%\n", progress);
}

// Verifica o status de um arquivo específico
emu_cloud_sync_status_t status;
if (emu_save_state_cloud_get_status(state, "savefile.state", &status)) {
    if (status == EMU_CLOUD_SYNC_CONFLICT) {
        printf("Conflito detectado!\n");
    }
}
```

## 11. Gestão de Erros

### 11.1 Códigos de Erro

A API retorna informações detalhadas sobre erros:

| Código | Nome | Descrição |
|--------|------|-----------|
| 0 | EMU_CLOUD_ERR_NONE | Sem erro |
| -1 | EMU_CLOUD_ERR_NETWORK | Erro de conexão de rede |
| -2 | EMU_CLOUD_ERR_AUTH | Falha de autenticação |
| -3 | EMU_CLOUD_ERR_PERMISSION | Permissão negada |
| -4 | EMU_CLOUD_ERR_NOT_FOUND | Arquivo não encontrado |
| -5 | EMU_CLOUD_ERR_CONFLICT | Conflito detectado |
| -6 | EMU_CLOUD_ERR_QUOTA | Cota de armazenamento excedida |
| -7 | EMU_CLOUD_ERR_INVALID | Dados inválidos |
| -8 | EMU_CLOUD_ERR_TIMEOUT | Operação expirou |
| -9 | EMU_CLOUD_ERR_API | Erro na API do provedor |

### 11.2 Recuperação de Erros

O sistema implementa estratégias de recuperação:

- Tentativas automáticas para erros temporários
- Renovação de token quando há falha de autenticação
- Cache de operações para tentar novamente quando houver conexão
- Backoff exponencial para evitar sobrecarga do servidor

## 12. Interface com o Usuário

### 12.1 Configuração

A interface permite ao usuário configurar:

- Provedor de nuvem
- Autenticação
- Diretório remoto
- Sincronização automática
- Intervalo de sincronização
- Estratégia de resolução de conflitos
- Criptografia de arquivos remotos

### 12.2 Monitoramento

A interface permite monitorar:

- Status de sincronização
- Progresso de operações
- Conflitos pendentes
- Histórico de sincronização
- Uso de armazenamento

## 13. Limitações e Considerações

### 13.1 Considerações de Rede

- A sincronização depende de conexão de rede estável
- Arquivos grandes podem levar mais tempo
- Algumas operações são executadas assincronamente

### 13.2 Limitações de API

Os provedores de nuvem têm limitações:

| Provedor | Limites Conhecidos |
|----------|-------------------|
| Google Drive | 1000 requisições/usuário/100 segundos |
| Dropbox | 100-600 requisições/app/hora (varia) |
| OneDrive | 60 requisições/minuto |

O sistema implementa limitação de taxa para evitar exceder estes limites.

### 13.3 Considerações de Tamanho

- Save states com thumbnail podem ser maiores (típicamente 1-10 MB)
- Compression é recomendada para reduzir tráfego
- Alguns provedores têm limitações de tamanho de arquivo

## 14. Implementação de Referência

### 14.1 Dependências

A implementação depende de:

- libcurl para operações HTTP
- Bibliotecas JSON para parsear respostas
- Biblioteca OAuth para autenticação
- Ferramentas de networking da plataforma

### 14.2 Portabilidade

A implementação é projetada para ser portável entre:

- Windows (Win32/Win64)
- Linux
- macOS
- Mobile (iOS/Android)
- Web (via emscripten + indexedDB)

## 15. Testes e Validação

A implementação inclui testes:

- Testes unitários para cada função
- Testes de integração com cada provedor
- Testes de sincronização entre plataformas
- Simulação de condições de erro e recuperação
- Testes de desempenho para diferentes tamanhos de arquivo

## 16. Exemplos de Uso

### 16.1 Backup Automático

```c
// Configuração para backup automático no Google Drive
emu_cloud_config_t backup_config = {
    .provider = EMU_CLOUD_GOOGLE_DRIVE,
    .auth_token = google_token,
    .folder_path = "/MegaEmu/Backups",
    .auto_sync = true,
    .sync_interval = 3600  // Backup a cada hora
};

// Configurar backup automático
emu_save_state_cloud_configure(state, &backup_config);
emu_save_state_cloud_configure_auto_backup(state, true, 3600);
```

### 16.2 Sincronização Entre Dispositivos

```c
// No dispositivo A (Upload)
emu_save_state_cloud_sync(state, "savefile.state", true);

// No dispositivo B (Verificar e baixar novas versões)
size_t changes = 0;
if (emu_save_state_cloud_check_updates(state, save_dir, &changes) && changes > 0) {
    printf("%zu novos save states disponíveis na nuvem\n", changes);
    emu_save_state_cloud_sync_all(state, save_dir);
}
```

### 16.3 Compartilhamento de Save State

```c
// Exportação para compartilhamento (com criptografia)
emu_encryption_config_t crypto_config = {
    .method = EMU_CRYPT_AES256_GCM,
    .derive_from_password = true,
    .password = "senha_compartilhada"
};

// Configurar criptografia e nuvem
emu_save_state_set_encryption(state, &crypto_config);
emu_save_state_cloud_configure(state, &cloud_config);

// Salvar com criptografia e enviar para nuvem
emu_save_options_t options = {
    .flags = EMU_SAVE_OPT_ENCRYPT | EMU_SAVE_OPT_CLOUD_SYNC
};

emu_save_state_save(state, "compartilhado.state", &options);

// Compartilhar a senha por um canal separado
// O link para o arquivo na nuvem pode ser obtido via:
char share_url[512];
emu_save_state_cloud_get_share_url(state, "compartilhado.state", share_url, sizeof(share_url));
```

## 17. Roteiro de Implementação

| Fase | Duração | Descrição |
|------|---------|-----------|
| 1 | 1 semana | Implementação da API básica |
| 2 | 1 semana | Implementação do adaptador Google Drive |
| 3 | 1 semana | Implementação dos adaptadores Dropbox e OneDrive |
| 4 | 1 semana | Sistema de resolução de conflitos |
| 5 | 1 semana | Integração com UI e testes |

## 18. Referências

1. [Google Drive API v3](https://developers.google.com/drive/api/v3/reference)
2. [Dropbox API v2](https://www.dropbox.com/developers/documentation/http/documentation)
3. [Microsoft Graph API](https://docs.microsoft.com/en-us/graph/api/resources/onedrive?view=graph-rest-1.0)
4. [OAuth 2.0 Specification](https://oauth.net/2/)
5. [HTTP/1.1 RFC 2616](https://tools.ietf.org/html/rfc2616)
