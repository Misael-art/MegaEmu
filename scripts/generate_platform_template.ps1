# Script para gerar esqueletos de plataforma para Mega_Emu
# generate_platform_template.ps1
# Autor: Mega_Emu Team
# Uso: .\generate_platform_template.ps1 -PlatformName <nome_plataforma>

param (
    [Parameter(Mandatory=$true)]
    [string]$PlatformName,

    [Parameter(Mandatory=$false)]
    [string]$OutputDir = $null,

    [Parameter(Mandatory=$false)]
    [switch]$Force = $false
)

# Verifica se o nome da plataforma é válido
if ($PlatformName -notmatch "^[a-z0-9_]+$") {
    Write-Host "ERRO: Nome de plataforma inválido. Use apenas letras minúsculas, números e underscores." -ForegroundColor Red
    exit 1
}

# Configura diretório de saída
$projectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)

if ($null -eq $OutputDir) {
    $OutputDir = Join-Path -Path $projectRoot -ChildPath "src\platforms\$PlatformName"
}

# Verifica se o diretório já existe
if ((Test-Path -Path $OutputDir) -and -not $Force) {
    Write-Host "ERRO: O diretório '$OutputDir' já existe. Use -Force para sobrescrever." -ForegroundColor Red
    exit 1
}

# Cria o diretório se não existir
if (-not (Test-Path -Path $OutputDir)) {
    New-Item -Path $OutputDir -ItemType Directory -Force | Out-Null
    Write-Host "Diretório criado: $OutputDir" -ForegroundColor Green
}

# Função para criar arquivo com conteúdo
function Create-FileWithContent {
    param (
        [string]$FilePath,
        [string]$Content
    )

    if ((Test-Path -Path $FilePath) -and -not $Force) {
        Write-Host "Aviso: Arquivo '$FilePath' já existe e não será sobrescrito." -ForegroundColor Yellow
    } else {
        Set-Content -Path $FilePath -Value $Content -Force
        Write-Host "Arquivo criado: $FilePath" -ForegroundColor Green
    }
}

# Gera o conteúdo do arquivo de cabeçalho principal
$headerContent = @"
/**
 * @file ${PlatformName}.h
 * @brief Interface principal do emulador de ${PlatformName}
 */

#ifndef ${PlatformName.ToUpper()}_H
#define ${PlatformName.ToUpper()}_H

#include "../../core/global_defines.h"

/**
 * @brief Inicializa o emulador de ${PlatformName}
 * @return MEGA_EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int ${PlatformName}_init(void);

/**
 * @brief Reseta o emulador de ${PlatformName}
 * @return MEGA_EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int ${PlatformName}_reset(void);

/**
 * @brief Finaliza o emulador de ${PlatformName} e libera recursos
 * @return MEGA_EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int ${PlatformName}_shutdown(void);

/**
 * @brief Carrega uma ROM no emulador de ${PlatformName}
 * @param rom_data Ponteiro para os dados da ROM
 * @param rom_size Tamanho da ROM em bytes
 * @return MEGA_EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int ${PlatformName}_load_rom(const u8 *rom_data, size_t rom_size);

/**
 * @brief Executa um quadro de emulação
 * @return MEGA_EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int ${PlatformName}_run_frame(void);

/**
 * @brief Obtém o buffer de vídeo atual
 * @param width Ponteiro para armazenar a largura do buffer de vídeo
 * @param height Ponteiro para armazenar a altura do buffer de vídeo
 * @return Ponteiro para o buffer de vídeo, NULL em caso de erro
 */
const u32* ${PlatformName}_get_display_buffer(int *width, int *height);

/**
 * @brief Obtém o buffer de áudio atual
 * @param samples Ponteiro para armazenar o número de amostras no buffer
 * @param sample_rate Ponteiro para armazenar a taxa de amostragem
 * @return Ponteiro para o buffer de áudio, NULL em caso de erro
 */
const s16* ${PlatformName}_get_audio_buffer(int *samples, int *sample_rate);

/**
 * @brief Define o estado dos controles de entrada
 * @param port Número da porta do controle (0-1)
 * @param buttons Estado dos botões (bitmask)
 * @return MEGA_EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int ${PlatformName}_set_input_state(int port, u32 buttons);

/**
 * @brief Configura parâmetros do emulador
 * @param key Chave de configuração
 * @param value Valor da configuração
 * @return MEGA_EMU_SUCCESS em caso de sucesso, código de erro em caso de falha
 */
int ${PlatformName}_set_config(const char *key, const char *value);

#endif /* ${PlatformName.ToUpper()}_H */
"@

# Gera o conteúdo do arquivo de implementação principal
$implContent = @"
/**
 * @file ${PlatformName}.c
 * @brief Implementação principal do emulador de ${PlatformName}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../core/global_defines.h"
#include "${PlatformName}.h"
#include "${PlatformName}_internal.h"

// Estrutura principal do emulador
static ${PlatformName}_t ${PlatformName};

// Buffer de vídeo
static u32 display_buffer[MAX_DISPLAY_WIDTH * MAX_DISPLAY_HEIGHT];

// Buffer de áudio
static s16 audio_buffer[MAX_AUDIO_BUFFER_SIZE];

/**
 * @brief Inicializa o emulador de ${PlatformName}
 */
int ${PlatformName}_init(void) {
    // Inicializa estrutura com zeros
    memset(&${PlatformName}, 0, sizeof(${PlatformName}_t));

    // Inicializa subsistemas
    if (_init_memory() != MEGA_EMU_SUCCESS) {
        return MEGA_EMU_ERROR;
    }

    if (_init_cpu() != MEGA_EMU_SUCCESS) {
        return MEGA_EMU_ERROR;
    }

    if (_init_video() != MEGA_EMU_SUCCESS) {
        return MEGA_EMU_ERROR;
    }

    if (_init_audio() != MEGA_EMU_SUCCESS) {
        return MEGA_EMU_ERROR;
    }

    if (_init_input() != MEGA_EMU_SUCCESS) {
        return MEGA_EMU_ERROR;
    }

    return MEGA_EMU_SUCCESS;
}

/**
 * @brief Reseta o emulador de ${PlatformName}
 */
int ${PlatformName}_reset(void) {
    // Implementar reset dos componentes

    return MEGA_EMU_SUCCESS;
}

/**
 * @brief Finaliza o emulador de ${PlatformName} e libera recursos
 */
int ${PlatformName}_shutdown(void) {
    // Liberar memória e recursos
    _shutdown_memory();
    _shutdown_cpu();
    _shutdown_video();
    _shutdown_audio();
    _shutdown_input();

    return MEGA_EMU_SUCCESS;
}

/**
 * @brief Carrega uma ROM no emulador de ${PlatformName}
 */
int ${PlatformName}_load_rom(const u8 *rom_data, size_t rom_size) {
    if (rom_data == NULL || rom_size == 0) {
        return MEGA_EMU_INVALID_ARG;
    }

    // Validar ROM
    if (!_validate_rom(rom_data, rom_size)) {
        return MEGA_EMU_ERROR;
    }

    // Carregar ROM na memória
    if (_load_rom_to_memory(rom_data, rom_size) != MEGA_EMU_SUCCESS) {
        return MEGA_EMU_ERROR;
    }

    // Reset após carregar a ROM
    return ${PlatformName}_reset();
}

/**
 * @brief Executa um quadro de emulação
 */
int ${PlatformName}_run_frame(void) {
    // Executar um frame completo de emulação

    // Atualizar buffers de áudio e vídeo
    _update_display_buffer();
    _update_audio_buffer();

    return MEGA_EMU_SUCCESS;
}

/**
 * @brief Obtém o buffer de vídeo atual
 */
const u32* ${PlatformName}_get_display_buffer(int *width, int *height) {
    if (width != NULL) {
        *width = ${PlatformName}.display_width;
    }

    if (height != NULL) {
        *height = ${PlatformName}.display_height;
    }

    return display_buffer;
}

/**
 * @brief Obtém o buffer de áudio atual
 */
const s16* ${PlatformName}_get_audio_buffer(int *samples, int *sample_rate) {
    if (samples != NULL) {
        *samples = ${PlatformName}.audio_samples;
    }

    if (sample_rate != NULL) {
        *sample_rate = ${PlatformName}.sample_rate;
    }

    return audio_buffer;
}

/**
 * @brief Define o estado dos controles de entrada
 */
int ${PlatformName}_set_input_state(int port, u32 buttons) {
    if (port < 0 || port >= MAX_INPUT_PORTS) {
        return MEGA_EMU_INVALID_ARG;
    }

    ${PlatformName}.input_state[port] = buttons;

    return MEGA_EMU_SUCCESS;
}

/**
 * @brief Configura parâmetros do emulador
 */
int ${PlatformName}_set_config(const char *key, const char *value) {
    if (key == NULL || value == NULL) {
        return MEGA_EMU_INVALID_ARG;
    }

    // Implementar configurações específicas

    return MEGA_EMU_SUCCESS;
}

// Funções internas de atualização
static void _update_display_buffer(void) {
    // Implementar renderização no buffer de vídeo
}

static void _update_audio_buffer(void) {
    // Implementar geração de áudio no buffer de áudio
}
"@

# Gera o conteúdo do arquivo de cabeçalho interno
$internalHeaderContent = @"
/**
 * @file ${PlatformName}_internal.h
 * @brief Definições internas do emulador de ${PlatformName}
 */

#ifndef ${PlatformName.ToUpper()}_INTERNAL_H
#define ${PlatformName.ToUpper()}_INTERNAL_H

#include "../../core/global_defines.h"

// Tamanhos máximos para buffers
#define MAX_DISPLAY_WIDTH 320
#define MAX_DISPLAY_HEIGHT 240
#define MAX_AUDIO_BUFFER_SIZE 4096
#define MAX_INPUT_PORTS 2

// Estrutura principal do emulador
typedef struct {
    // Estado do sistema
    bool     initialized;
    bool     rom_loaded;

    // Informações de vídeo
    int      display_width;
    int      display_height;

    // Informações de áudio
    int      audio_samples;
    int      sample_rate;

    // Estado de entrada
    u32      input_state[MAX_INPUT_PORTS];

    // Memória do sistema
    u8      *rom;
    size_t   rom_size;
    u8      *ram;
    size_t   ram_size;

    // Registradores e estado do CPU
    // TODO: Adicionar estrutura de CPU específica

    // Estado de vídeo
    // TODO: Adicionar estrutura de vídeo específica

    // Estado de áudio
    // TODO: Adicionar estrutura de áudio específica

} ${PlatformName}_t;

// Funções internas de inicialização
int _init_memory(void);
int _init_cpu(void);
int _init_video(void);
int _init_audio(void);
int _init_input(void);

// Funções internas de finalização
void _shutdown_memory(void);
void _shutdown_cpu(void);
void _shutdown_video(void);
void _shutdown_audio(void);
void _shutdown_input(void);

// Funções de manipulação de ROM
bool _validate_rom(const u8 *rom_data, size_t rom_size);
int _load_rom_to_memory(const u8 *rom_data, size_t rom_size);

// Funções internas de atualização
static void _update_display_buffer(void);
static void _update_audio_buffer(void);

#endif /* ${PlatformName.ToUpper()}_INTERNAL_H */
"@

# Cria arquivos
$headerFile = Join-Path -Path $OutputDir -ChildPath "${PlatformName}.h"
Create-FileWithContent -FilePath $headerFile -Content $headerContent

$implFile = Join-Path -Path $OutputDir -ChildPath "${PlatformName}.c"
Create-FileWithContent -FilePath $implFile -Content $implContent

$internalHeaderFile = Join-Path -Path $OutputDir -ChildPath "${PlatformName}_internal.h"
Create-FileWithContent -FilePath $internalHeaderFile -Content $internalHeaderContent

# Gera o arquivo CMakeLists.txt para a plataforma
$cmakeContent = @"
# CMakeLists.txt para o emulador de ${PlatformName}

set(${PlatformName.ToUpper()}_SOURCES
    ${PlatformName}.c
    # Adicione outros arquivos de origem aqui
)

set(${PlatformName.ToUpper()}_HEADERS
    ${PlatformName}.h
    ${PlatformName}_internal.h
    # Adicione outros arquivos de cabeçalho aqui
)

add_library(${PlatformName} STATIC
    \${${PlatformName.ToUpper()}_SOURCES}
    \${${PlatformName.ToUpper()}_HEADERS}
)

target_include_directories(${PlatformName} PUBLIC
    \${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PlatformName}
    core
    # Adicione outras dependências aqui
)
"@

$cmakeFile = Join-Path -Path $OutputDir -ChildPath "CMakeLists.txt"
Create-FileWithContent -FilePath $cmakeFile -Content $cmakeContent

# Gera um arquivo README.md básico para a plataforma
$readmeContent = @"
# Emulador de ${PlatformName} para Mega_Emu

Este módulo implementa a emulação da plataforma ${PlatformName} para o projeto Mega_Emu.

## Características

- [Listar características suportadas]
- [Listar jogos/ROMs testados]

## Implementação

- [Descrever o estado atual da implementação]
- [Listar funcionalidades pendentes]

## Referências

- [Listar documentação e referências usadas]
"@

$readmeFile = Join-Path -Path $OutputDir -ChildPath "README.md"
Create-FileWithContent -FilePath $readmeFile -Content $readmeContent

Write-Host ""
Write-Host "Esqueleto de plataforma '${PlatformName}' gerado com sucesso!" -ForegroundColor Green
Write-Host "Diretório: $OutputDir" -ForegroundColor Cyan
Write-Host ""
Write-Host "Próximos passos:" -ForegroundColor Yellow
Write-Host "1. Adicione a plataforma ao CMakeLists.txt principal" -ForegroundColor Yellow
Write-Host "2. Implemente as funções específicas do ${PlatformName}" -ForegroundColor Yellow
Write-Host "3. Adicione os componentes específicos da plataforma (CPU, vídeo, áudio)" -ForegroundColor Yellow
Write-Host ""
