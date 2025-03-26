/**
 * @file sdl_screenshot.c
 * @brief Implementação do sistema de captura de tela para o frontend SDL
 */
#include "sdl_screenshot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#endif

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "utils/enhanced_log.h"
#include "utils/file_utils.h"
#include "utils/time_utils.h"

// Macros de log
#define SCREENSHOT_LOG_ERROR(...) EMU_LOG_ERROR(EMU_LOG_CAT_FRONTEND, __VA_ARGS__)
#define SCREENSHOT_LOG_WARN(...) EMU_LOG_WARN(EMU_LOG_CAT_FRONTEND, __VA_ARGS__)
#define SCREENSHOT_LOG_INFO(...) EMU_LOG_INFO(EMU_LOG_CAT_FRONTEND, __VA_ARGS__)
#define SCREENSHOT_LOG_DEBUG(...) EMU_LOG_DEBUG(EMU_LOG_CAT_FRONTEND, __VA_ARGS__)

// Configuração padrão para screenshots
static const sdl_screenshot_config_t DEFAULT_SCREENSHOT_CONFIG = {
    .output_dir = "screenshots",
    .format = SDL_SCREENSHOT_FORMAT_PNG,
    .quality = 90,
    .include_timestamp = true,
    .include_game_name = true,
    .show_notification = true
};

// Funções auxiliares internas

/**
 * @brief Cria diretório recursivamente
 *
 * @param path Caminho a ser criado
 * @return true Se foi criado com sucesso ou já existe
 * @return false Se falhou
 */
static bool create_directory(const char *path) {
    char tmp[SDL_SCREENSHOT_MAX_PATH];
    char *p = NULL;
    size_t len;

    // Copiar o caminho para buffer temporário
    strncpy(tmp, path, sizeof(tmp));
    len = strlen(tmp);

    // Remover barra no final se existir
    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\') {
        tmp[len - 1] = 0;
    }

    // Criar diretórios recursivamente
    for (p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = 0;

            // Tentar criar o diretório
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                return false;
            }

            *p = '/';
        }
    }

    // Criar o último diretório
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return false;
    }

    return true;
}

/**
 * @brief Gera um nome de arquivo para a captura de tela
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @param buffer Buffer para o nome do arquivo
 * @param max_size Tamanho máximo do buffer
 */
static void generate_screenshot_filename(
    const sdl_screenshot_t *screenshot,
    char *buffer,
    size_t max_size) {

    time_t now;
    struct tm *timeinfo;
    char timestamp[20] = "";
    char extension[5] = "png";

    // Obter timestamp atual
    time(&now);
    timeinfo = localtime(&now);

    // Formatar timestamp se necessário
    if (screenshot->config.include_timestamp) {
        strftime(timestamp, sizeof(timestamp), "_%Y%m%d_%H%M%S", timeinfo);
    }

    // Determinar extensão baseada no formato
    switch (screenshot->config.format) {
        case SDL_SCREENSHOT_FORMAT_BMP:
            strcpy(extension, "bmp");
            break;
        case SDL_SCREENSHOT_FORMAT_JPG:
            strcpy(extension, "jpg");
            break;
        case SDL_SCREENSHOT_FORMAT_PNG:
        default:
            strcpy(extension, "png");
            break;
    }

    // Gerar nome do arquivo
    if (screenshot->config.include_game_name &&
        strlen(screenshot->current_game_name) > 0) {
        snprintf(buffer, max_size, "%s%s.%s",
                 screenshot->current_game_name,
                 timestamp,
                 extension);
    } else {
        snprintf(buffer, max_size, "screenshot%s.%s",
                 timestamp,
                 extension);
    }
}

/**
 * @brief Atualiza a notificação de screenshot
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @param message Mensagem a ser exibida
 */
static void show_screenshot_notification(sdl_screenshot_t *screenshot, const char *message) {
    if (!screenshot || !screenshot->config.show_notification ||
        !screenshot->renderer || !screenshot->renderer->renderer) {
        return;
    }

    // Liberar texturas anteriores se existirem
    if (screenshot->notification.message) {
        SDL_DestroyTexture(screenshot->notification.message);
        screenshot->notification.message = NULL;
    }

    // Criar fonte para a notificação se não existir ainda
    static TTF_Font *notification_font = NULL;

    if (!notification_font) {
        notification_font = TTF_OpenFont("assets/fonts/roboto_regular.ttf", 16);
        if (!notification_font) {
            return;
        }
    }

    // Renderizar texto
    SDL_Color text_color = {255, 255, 255, 255};
    SDL_Surface *text_surface = TTF_RenderText_Blended(
        notification_font,
        message,
        text_color);

    if (!text_surface) {
        return;
    }

    // Criar textura a partir da superfície
    screenshot->notification.message = SDL_CreateTextureFromSurface(
        screenshot->renderer->renderer,
        text_surface);

    SDL_FreeSurface(text_surface);

    if (!screenshot->notification.message) {
        return;
    }

    // Configurar posição da notificação
    int text_width, text_height;
    SDL_QueryTexture(screenshot->notification.message, NULL, NULL, &text_width, &text_height);

    int window_width, window_height;
    SDL_GetRendererOutputSize(screenshot->renderer->renderer, &window_width, &window_height);

    // Posicionar no canto superior direito com margem
    screenshot->notification.position.x = window_width - text_width - 20;
    screenshot->notification.position.y = 20;
    screenshot->notification.position.w = text_width;
    screenshot->notification.position.h = text_height;

    // Configurar tempo de exibição
    screenshot->notification.visible = true;
    screenshot->notification.start_time = SDL_GetTicks();
}

/**
 * @brief Salva uma superfície como arquivo de imagem
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @param surface Superfície a ser salva
 * @param filepath Caminho completo para o arquivo
 * @return true Se salvo com sucesso
 * @return false Se falhou
 */
static bool save_surface_to_file(
    const sdl_screenshot_t *screenshot,
    SDL_Surface *surface,
    const char *filepath) {

    bool result = false;

    // Salvar no formato especificado
    switch (screenshot->config.format) {
        case SDL_SCREENSHOT_FORMAT_BMP:
            result = (SDL_SaveBMP(surface, filepath) == 0);
            break;

        case SDL_SCREENSHOT_FORMAT_JPG:
            result = (IMG_SaveJPG(surface, filepath, screenshot->config.quality) == 0);
            break;

        case SDL_SCREENSHOT_FORMAT_PNG:
        default:
            result = (IMG_SavePNG(surface, filepath) == 0);
            break;
    }

    return result;
}

// Implementação das funções públicas

/**
 * @brief Inicializa o sistema de screenshots
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @param renderer Ponteiro para o renderizador do jogo
 * @return true Se inicializado com sucesso
 * @return false Se falhou
 */
bool sdl_screenshot_init(sdl_screenshot_t *screenshot, sdl_game_renderer_t *renderer) {
    if (!screenshot || !renderer) {
        return false;
    }

    // Limpar a estrutura
    memset(screenshot, 0, sizeof(sdl_screenshot_t));

    // Configurar valores iniciais
    screenshot->renderer = renderer;
    screenshot->initialized = true;
    screenshot->last_screenshot_time = 0;

    // Usar configuração padrão
    sdl_screenshot_set_default_config(screenshot);

    // Criar diretório de saída
    if (!create_directory(screenshot->config.output_dir)) {
        SCREENSHOT_LOG_WARN("Não foi possível criar diretório para screenshots: %s",
                          screenshot->config.output_dir);
    }

    return true;
}

/**
 * @brief Finaliza o sistema de screenshots
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 */
void sdl_screenshot_shutdown(sdl_screenshot_t *screenshot) {
    if (!screenshot || !screenshot->initialized) {
        return;
    }

    // Liberar recursos da notificação
    if (screenshot->notification.message) {
        SDL_DestroyTexture(screenshot->notification.message);
        screenshot->notification.message = NULL;
    }

    if (screenshot->notification.icon) {
        SDL_DestroyTexture(screenshot->notification.icon);
        screenshot->notification.icon = NULL;
    }

    // Limpar estado
    screenshot->initialized = false;
}

/**
 * @brief Captura a tela atual
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @return true Se capturado com sucesso
 * @return false Se falhou
 */
bool sdl_screenshot_capture(sdl_screenshot_t *screenshot) {
    if (!screenshot || !screenshot->initialized || !screenshot->renderer) {
        return false;
    }

    char filename[SDL_SCREENSHOT_MAX_PATH];
    char filepath[SDL_SCREENSHOT_MAX_PATH];

    // Gerar nome do arquivo
    generate_screenshot_filename(screenshot, filename, sizeof(filename));

    // Construir caminho completo
    snprintf(filepath, sizeof(filepath), "%s/%s",
             screenshot->config.output_dir, filename);

    return sdl_screenshot_capture_to_file(screenshot, filepath);
}

/**
 * @brief Captura a tela atual para um arquivo específico
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @param filepath Caminho completo para o arquivo
 * @return true Se capturado com sucesso
 * @return false Se falhou
 */
bool sdl_screenshot_capture_to_file(sdl_screenshot_t *screenshot, const char *filepath) {
    if (!screenshot || !screenshot->initialized ||
        !screenshot->renderer || !filepath) {
        return false;
    }

    // Obter tamanho da janela de renderização
    int width, height;
    SDL_GetRendererOutputSize(screenshot->renderer->renderer, &width, &height);

    // Criar superfície para capturar a tela
    SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 32,
                                               0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!surface) {
        SCREENSHOT_LOG_ERROR("Falha ao criar superfície para screenshot: %s", SDL_GetError());
        return false;
    }

    // Ler pixels do renderizador
    if (SDL_RenderReadPixels(screenshot->renderer->renderer, NULL,
                           surface->format->format,
                           surface->pixels,
                           surface->pitch) != 0) {
        SCREENSHOT_LOG_ERROR("Falha ao ler pixels do renderizador: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return false;
    }

    // Verificar diretório e criar se necessário
    char dir[SDL_SCREENSHOT_MAX_PATH];
    strncpy(dir, filepath, sizeof(dir));

    // Encontrar última barra para extrair diretório
    char *last_slash = strrchr(dir, '/');
    if (last_slash) {
        *last_slash = '\0';
        create_directory(dir);
    }

    // Salvar para arquivo
    bool result = save_surface_to_file(screenshot, surface, filepath);

    if (result) {
        // Atualizar informações da última captura
        strncpy(screenshot->last_screenshot_path, filepath,
                sizeof(screenshot->last_screenshot_path) - 1);
        screenshot->last_screenshot_time = time(NULL);

        // Mostrar notificação
        char notification[256];
        snprintf(notification, sizeof(notification),
                "Screenshot salvo: %s", strrchr(filepath, '/') + 1);
        show_screenshot_notification(screenshot, notification);

        SCREENSHOT_LOG_INFO("Screenshot salvo com sucesso: %s", filepath);
    } else {
        SCREENSHOT_LOG_ERROR("Falha ao salvar screenshot: %s", filepath);
    }

    SDL_FreeSurface(surface);
    return result;
}

/**
 * @brief Captura um framebuffer específico para um arquivo
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @param framebuffer Buffer de pixels a ser capturado
 * @param width Largura da imagem
 * @param height Altura da imagem
 * @return true Se capturado com sucesso
 * @return false Se falhou
 */
bool sdl_screenshot_capture_framebuffer(sdl_screenshot_t *screenshot,
                                      const uint32_t *framebuffer,
                                      int width, int height) {
    if (!screenshot || !screenshot->initialized || !framebuffer || width <= 0 || height <= 0) {
        return false;
    }

    char filename[SDL_SCREENSHOT_MAX_PATH];
    char filepath[SDL_SCREENSHOT_MAX_PATH];

    // Gerar nome do arquivo
    generate_screenshot_filename(screenshot, filename, sizeof(filename));

    // Construir caminho completo
    snprintf(filepath, sizeof(filepath), "%s/%s",
             screenshot->config.output_dir, filename);

    // Criar superfície para armazenar o framebuffer
    SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 32,
                                               0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!surface) {
        SCREENSHOT_LOG_ERROR("Falha ao criar superfície para framebuffer: %s", SDL_GetError());
        return false;
    }

    // Copiar dados do framebuffer para a superfície
    memcpy(surface->pixels, framebuffer, width * height * 4);

    // Verificar diretório e criar se necessário
    char dir[SDL_SCREENSHOT_MAX_PATH];
    strncpy(dir, filepath, sizeof(dir));

    // Encontrar última barra para extrair diretório
    char *last_slash = strrchr(dir, '/');
    if (last_slash) {
        *last_slash = '\0';
        create_directory(dir);
    }

    // Salvar para arquivo
    bool result = save_surface_to_file(screenshot, surface, filepath);

    if (result) {
        // Atualizar informações da última captura
        strncpy(screenshot->last_screenshot_path, filepath,
                sizeof(screenshot->last_screenshot_path) - 1);
        screenshot->last_screenshot_time = time(NULL);

        // Mostrar notificação
        char notification[256];
        snprintf(notification, sizeof(notification),
                "Screenshot salvo: %s", strrchr(filepath, '/') + 1);
        show_screenshot_notification(screenshot, notification);

        SCREENSHOT_LOG_INFO("Screenshot do framebuffer salvo com sucesso: %s", filepath);
    } else {
        SCREENSHOT_LOG_ERROR("Falha ao salvar screenshot do framebuffer: %s", filepath);
    }

    SDL_FreeSurface(surface);
    return result;
}

/**
 * @brief Define a configuração do sistema de screenshots
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @param config Nova configuração
 */
void sdl_screenshot_set_config(sdl_screenshot_t *screenshot, const sdl_screenshot_config_t *config) {
    if (!screenshot || !screenshot->initialized || !config) {
        return;
    }

    // Copiar configuração
    memcpy(&screenshot->config, config, sizeof(sdl_screenshot_config_t));

    // Verificar se o diretório de saída existe, criar se necessário
    create_directory(screenshot->config.output_dir);
}

/**
 * @brief Obtém a configuração atual do sistema de screenshots
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @param config Ponteiro para armazenar a configuração
 */
void sdl_screenshot_get_config(const sdl_screenshot_t *screenshot, sdl_screenshot_config_t *config) {
    if (!screenshot || !screenshot->initialized || !config) {
        return;
    }

    // Copiar configuração
    memcpy(config, &screenshot->config, sizeof(sdl_screenshot_config_t));
}

/**
 * @brief Restaura a configuração padrão
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 */
void sdl_screenshot_set_default_config(sdl_screenshot_t *screenshot) {
    if (!screenshot || !screenshot->initialized) {
        return;
    }

    // Copiar a configuração padrão
    memcpy(&screenshot->config, &DEFAULT_SCREENSHOT_CONFIG, sizeof(sdl_screenshot_config_t));

    // Verificar se o diretório de saída existe, criar se necessário
    create_directory(screenshot->config.output_dir);
}

/**
 * @brief Salva a configuração atual para um arquivo
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @param filepath Caminho do arquivo
 * @return true Se salvo com sucesso
 * @return false Se falhou
 */
bool sdl_screenshot_save_config(const sdl_screenshot_t *screenshot, const char *filepath) {
    if (!screenshot || !screenshot->initialized || !filepath) {
        return false;
    }

    FILE *file = fopen(filepath, "wb");
    if (!file) {
        SCREENSHOT_LOG_ERROR("Falha ao abrir arquivo para salvar configuração: %s", filepath);
        return false;
    }

    // Salvar versão do formato
    uint32_t version = 1;
    fwrite(&version, sizeof(uint32_t), 1, file);

    // Salvar configuração
    fwrite(&screenshot->config, sizeof(sdl_screenshot_config_t), 1, file);

    fclose(file);
    return true;
}

/**
 * @brief Carrega a configuração de um arquivo
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @param filepath Caminho do arquivo
 * @return true Se carregado com sucesso
 * @return false Se falhou
 */
bool sdl_screenshot_load_config(sdl_screenshot_t *screenshot, const char *filepath) {
    if (!screenshot || !screenshot->initialized || !filepath) {
        return false;
    }

    FILE *file = fopen(filepath, "rb");
    if (!file) {
        SCREENSHOT_LOG_ERROR("Falha ao abrir arquivo para carregar configuração: %s", filepath);
        return false;
    }

    // Verificar versão do formato
    uint32_t version;
    if (fread(&version, sizeof(uint32_t), 1, file) != 1 || version != 1) {
        SCREENSHOT_LOG_ERROR("Versão de arquivo de configuração inválida: %s", filepath);
        fclose(file);
        return false;
    }

    // Ler configuração
    sdl_screenshot_config_t config;
    if (fread(&config, sizeof(sdl_screenshot_config_t), 1, file) != 1) {
        SCREENSHOT_LOG_ERROR("Falha ao ler configuração do arquivo: %s", filepath);
        fclose(file);
        return false;
    }

    fclose(file);

    // Aplicar a configuração carregada
    sdl_screenshot_set_config(screenshot, &config);

    return true;
}

/**
 * @brief Define o nome do jogo atual
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @param game_name Nome do jogo
 */
void sdl_screenshot_set_game_name(sdl_screenshot_t *screenshot, const char *game_name) {
    if (!screenshot || !screenshot->initialized || !game_name) {
        return;
    }

    // Copiar nome do jogo
    strncpy(screenshot->current_game_name, game_name, sizeof(screenshot->current_game_name) - 1);
    screenshot->current_game_name[sizeof(screenshot->current_game_name) - 1] = '\0';
}

/**
 * @brief Obtém o caminho do último screenshot
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @return const char* Caminho do último screenshot, ou NULL se não houver
 */
const char* sdl_screenshot_get_last_path(const sdl_screenshot_t *screenshot) {
    if (!screenshot || !screenshot->initialized ||
        screenshot->last_screenshot_path[0] == '\0') {
        return NULL;
    }

    return screenshot->last_screenshot_path;
}

/**
 * @brief Obtém o timestamp do último screenshot
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @return time_t Timestamp do último screenshot, ou 0 se não houver
 */
time_t sdl_screenshot_get_last_time(const sdl_screenshot_t *screenshot) {
    if (!screenshot || !screenshot->initialized) {
        return 0;
    }

    return screenshot->last_screenshot_time;
}

/**
 * @brief Atualiza o estado da notificação
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 */
void sdl_screenshot_update_notification(sdl_screenshot_t *screenshot) {
    if (!screenshot || !screenshot->initialized || !screenshot->notification.visible) {
        return;
    }

    // Verificar se a notificação precisa ser escondida
    Uint32 current_time = SDL_GetTicks();
    Uint32 elapsed = current_time - screenshot->notification.start_time;

    if (elapsed > screenshot->notification.duration) {
        screenshot->notification.visible = false;
    }
}

/**
 * @brief Renderiza a notificação na tela
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 */
void sdl_screenshot_render_notification(sdl_screenshot_t *screenshot) {
    if (!screenshot || !screenshot->initialized ||
        !screenshot->notification.visible ||
        !screenshot->notification.message ||
        !screenshot->renderer || !screenshot->renderer->renderer) {
        return;
    }

    // Calcular alpha baseado no tempo restante
    Uint32 current_time = SDL_GetTicks();
    Uint32 elapsed = current_time - screenshot->notification.start_time;
    Uint32 remaining = (elapsed >= screenshot->notification.duration) ?
                       0 : screenshot->notification.duration - elapsed;

    // Fade out nos últimos 500ms
    Uint8 alpha = 255;
    if (remaining < 500) {
        alpha = (Uint8)((remaining * 255) / 500);
    }

    // Desenhar fundo semi-transparente
    SDL_SetRenderDrawColor(screenshot->renderer->renderer, 0, 0, 0, alpha / 2);
    SDL_SetRenderDrawBlendMode(screenshot->renderer->renderer, SDL_BLENDMODE_BLEND);

    SDL_Rect bg_rect = screenshot->notification.position;
    bg_rect.x -= 10;
    bg_rect.y -= 5;
    bg_rect.w += 20;
    bg_rect.h += 10;

    SDL_RenderFillRect(screenshot->renderer->renderer, &bg_rect);

    // Desenhar texto com alpha ajustado
    SDL_SetTextureAlphaMod(screenshot->notification.message, alpha);
    SDL_RenderCopy(
        screenshot->renderer->renderer,
        screenshot->notification.message,
        NULL,
        &screenshot->notification.position);

    // Verificar se a notificação precisa ser escondida
    if (alpha == 0) {
        screenshot->notification.visible = false;
    }
}

/**
 * @brief Define a duração da notificação em milissegundos
 *
 * @param screenshot Ponteiro para a estrutura de screenshot
 * @param duration_ms Duração em milissegundos
 */
void sdl_screenshot_set_notification_duration(sdl_screenshot_t *screenshot, Uint32 duration_ms) {
    if (!screenshot || !screenshot->initialized) {
        return;
    }

    screenshot->notification.duration = duration_ms;
}
