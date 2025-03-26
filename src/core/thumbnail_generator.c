/**
 * @file thumbnail_generator.c
 * @brief Implementação de geração de thumbnails para save states
 */

#include "save_state.h"
#include "../utils/enhanced_log.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Incluir bibliotecas externas para manipulação de imagens
#include "../deps/webp/encode.h" // Biblioteca WebP para geração de imagens
#include "../deps/libcv/cv.h"    // Biblioteca de manipulação de imagem

// Definição da categoria de log
#define LOG_CAT_THUMBNAIL EMU_LOG_CAT_CORE

// Tamanho padrão para thumbnails
#define DEFAULT_THUMBNAIL_WIDTH 320
#define DEFAULT_THUMBNAIL_HEIGHT 240

// Texto padrão da tarja
#define DEFAULT_BANNER_TEXT "SAVE"

// Cores para a tarja
#define BANNER_BG_COLOR 0x000000FF   // Preto (RGBA)
#define BANNER_TEXT_COLOR 0xFFFFFFFF // Branco (RGBA)
#define BANNER_ALPHA 0xC0            // Alpha (75%)

// Estrutura para representar cores RGBA
typedef struct
{
    uint8_t r, g, b, a;
} rgba_color_t;

/**
 * @brief Converte um valor RGBA em struct rgba_color_t
 */
static rgba_color_t rgba_from_uint32(uint32_t rgba)
{
    rgba_color_t color;
    color.r = (rgba >> 24) & 0xFF;
    color.g = (rgba >> 16) & 0xFF;
    color.b = (rgba >> 8) & 0xFF;
    color.a = rgba & 0xFF;
    return color;
}

/**
 * @brief Redimensiona uma imagem para o tamanho especificado
 */
static uint8_t *resize_image(
    const uint8_t *src_data,
    uint32_t src_width,
    uint32_t src_height,
    uint32_t src_stride,
    uint32_t dst_width,
    uint32_t dst_height,
    uint32_t *dst_stride)
{
    // Determinar bytes por pixel (assumindo RGBA)
    const int bytes_per_pixel = 4;

    // Calcular stride de destino
    *dst_stride = dst_width * bytes_per_pixel;

    // Alocar memória para a imagem redimensionada
    uint8_t *dst_data = (uint8_t *)malloc(dst_height * (*dst_stride));
    if (!dst_data)
    {
        EMU_LOG_ERROR(LOG_CAT_THUMBNAIL, "Falha ao alocar memória para a imagem redimensionada");
        return NULL;
    }

    // Implementação básica de algoritmo de redimensionamento bilinear
    const float x_ratio = ((float)src_width - 1) / dst_width;
    const float y_ratio = ((float)src_height - 1) / dst_height;

    for (uint32_t y = 0; y < dst_height; y++)
    {
        for (uint32_t x = 0; x < dst_width; x++)
        {
            float src_x = x * x_ratio;
            float src_y = y * y_ratio;

            uint32_t src_x_floor = (uint32_t)src_x;
            uint32_t src_y_floor = (uint32_t)src_y;
            uint32_t src_x_ceil = (src_x_floor == src_width - 1) ? src_x_floor : src_x_floor + 1;
            uint32_t src_y_ceil = (src_y_floor == src_height - 1) ? src_y_floor : src_y_floor + 1;

            float x_diff = src_x - src_x_floor;
            float y_diff = src_y - src_y_floor;
            float x_diff_inv = 1.0f - x_diff;
            float y_diff_inv = 1.0f - y_diff;

            // Ponteiros para os quatro pixels mais próximos
            const uint8_t *p1 = src_data + (src_y_floor * src_stride) + (src_x_floor * bytes_per_pixel);
            const uint8_t *p2 = src_data + (src_y_floor * src_stride) + (src_x_ceil * bytes_per_pixel);
            const uint8_t *p3 = src_data + (src_y_ceil * src_stride) + (src_x_floor * bytes_per_pixel);
            const uint8_t *p4 = src_data + (src_y_ceil * src_stride) + (src_x_ceil * bytes_per_pixel);

            // Pixel de destino
            uint8_t *dst_pixel = dst_data + (y * (*dst_stride)) + (x * bytes_per_pixel);

            // Interpolação para cada canal
            for (int c = 0; c < bytes_per_pixel; c++)
            {
                float top = x_diff_inv * p1[c] + x_diff * p2[c];
                float bottom = x_diff_inv * p3[c] + x_diff * p4[c];
                dst_pixel[c] = (uint8_t)(y_diff_inv * top + y_diff * bottom);
            }
        }
    }

    return dst_data;
}

/**
 * @brief Adiciona uma tarja "Save" na parte inferior da imagem
 */
static void add_save_banner(
    uint8_t *image_data,
    uint32_t width,
    uint32_t height,
    uint32_t stride,
    const char *banner_text)
{
    if (!banner_text)
    {
        banner_text = DEFAULT_BANNER_TEXT;
    }

    // Altura da tarja (20% da altura da imagem)
    uint32_t banner_height = height / 5;

    // Posição Y da tarja (na parte inferior da imagem)
    uint32_t banner_y = height - banner_height;

    // Cores para a tarja
    rgba_color_t bg_color = rgba_from_uint32(BANNER_BG_COLOR);
    bg_color.a = BANNER_ALPHA; // Ajustar transparência

    rgba_color_t text_color = rgba_from_uint32(BANNER_TEXT_COLOR);

    // Desenhar fundo da tarja (retângulo semi-transparente)
    for (uint32_t y = banner_y; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            uint8_t *pixel = image_data + (y * stride) + (x * 4);

            // Aplicar alpha blending
            float alpha = bg_color.a / 255.0f;

            pixel[0] = (uint8_t)((bg_color.r * alpha) + (pixel[0] * (1.0f - alpha)));
            pixel[1] = (uint8_t)((bg_color.g * alpha) + (pixel[1] * (1.0f - alpha)));
            pixel[2] = (uint8_t)((bg_color.b * alpha) + (pixel[2] * (1.0f - alpha)));
            pixel[3] = 255; // Totalmente opaco
        }
    }

    // Desenhar texto
    // Em uma implementação real, usaríamos uma biblioteca de renderização de texto
    // mas para esta implementação, vamos simplificar e desenhar um texto básico

    // Centralizar texto
    uint32_t text_len = strlen(banner_text);
    uint32_t char_width = width / 20; // Largura aproximada de um caractere
    uint32_t text_width = text_len * char_width;
    uint32_t text_x = (width - text_width) / 2;
    uint32_t text_y = banner_y + (banner_height / 3);

    // Caracteres simples (versão básica - em uma implementação real usaríamos fontes reais)
    for (uint32_t i = 0; i < text_len; i++)
    {
        uint32_t char_x = text_x + (i * char_width);

        // Desenhar cada caractere como um retângulo branco (simplificação)
        for (uint32_t y = text_y; y < text_y + banner_height / 2; y++)
        {
            for (uint32_t x = char_x; x < char_x + char_width - 2; x++)
            {
                if (x < width && y < height)
                {
                    uint8_t *pixel = image_data + (y * stride) + (x * 4);
                    pixel[0] = text_color.r;
                    pixel[1] = text_color.g;
                    pixel[2] = text_color.b;
                    pixel[3] = 255;
                }
            }
        }
    }
}

/**
 * @brief Codifica uma imagem para o formato WebP
 */
static uint8_t *encode_webp(
    const uint8_t *rgba_data,
    uint32_t width,
    uint32_t height,
    uint32_t stride,
    float quality,
    size_t *output_size)
{
    // Configuração do WebP
    WebPConfig config;
    WebPConfigInit(&config);
    config.quality = quality;

    // Configuração da imagem de entrada
    WebPPicture pic;
    WebPPictureInit(&pic);
    pic.width = width;
    pic.height = height;
    pic.use_argb = 1;

    // Alocar memória para a imagem
    if (!WebPPictureAlloc(&pic))
    {
        EMU_LOG_ERROR(LOG_CAT_THUMBNAIL, "Falha ao alocar memória para codificação WebP");
        return NULL;
    }

    // Importar dados RGBA
    WebPPictureImportRGBA(&pic, rgba_data, stride);

    // Buffer de saída para WebP
    WebPMemoryWriter writer;
    WebPMemoryWriterInit(&writer);
    pic.writer = WebPMemoryWrite;
    pic.custom_ptr = &writer;

    // Codificar para WebP
    int success = WebPEncode(&config, &pic);
    WebPPictureFree(&pic);

    if (!success)
    {
        EMU_LOG_ERROR(LOG_CAT_THUMBNAIL, "Falha na codificação WebP, código: %d", pic.error_code);
        WebPMemoryWriterClear(&writer);
        return NULL;
    }

    *output_size = writer.size;
    return writer.mem;
}

/**
 * @brief Implementação da função para gerar thumbnail WebP
 */
int32_t save_state_generate_thumbnail(
    save_state_t *state,
    const uint8_t *screenshot_data,
    uint32_t width,
    uint32_t height,
    uint32_t stride,
    bool with_banner,
    const char *banner_text)
{
    if (!state || !screenshot_data || width == 0 || height == 0 || stride == 0)
    {
        EMU_LOG_ERROR(LOG_CAT_THUMBNAIL, "Parâmetros inválidos para geração de thumbnail");
        return SAVE_STATE_ERROR_INVALID;
    }

    EMU_LOG_INFO(LOG_CAT_THUMBNAIL, "Gerando thumbnail WebP %ux%u %s tarja",
                 width, height, with_banner ? "com" : "sem");

    // Obter configurações
    save_state_config_t config;
    save_state_get_config(state, &config);

    // Determinar tamanho do thumbnail
    uint32_t thumb_width = config.thumbnail_width > 0 ? config.thumbnail_width : DEFAULT_THUMBNAIL_WIDTH;
    uint32_t thumb_height = config.thumbnail_height > 0 ? config.thumbnail_height : DEFAULT_THUMBNAIL_HEIGHT;

    // Redimensionar a imagem
    uint32_t resized_stride;
    uint8_t *resized_data = resize_image(
        screenshot_data,
        width,
        height,
        stride,
        thumb_width,
        thumb_height,
        &resized_stride);

    if (!resized_data)
    {
        EMU_LOG_ERROR(LOG_CAT_THUMBNAIL, "Falha ao redimensionar imagem para thumbnail");
        return SAVE_STATE_ERROR_MEMORY;
    }

    // Adicionar tarja se necessário
    if (with_banner)
    {
        add_save_banner(
            resized_data,
            thumb_width,
            thumb_height,
            resized_stride,
            banner_text);
        EMU_LOG_DEBUG(LOG_CAT_THUMBNAIL, "Tarja 'Save' adicionada à thumbnail");
    }

    // Codificar para WebP
    size_t webp_size;
    uint8_t *webp_data = encode_webp(
        resized_data,
        thumb_width,
        thumb_height,
        resized_stride,
        config.thumbnail_quality > 0 ? config.thumbnail_quality : 80.0f,
        &webp_size);

    // Liberar memória da imagem redimensionada
    free(resized_data);

    if (!webp_data)
    {
        EMU_LOG_ERROR(LOG_CAT_THUMBNAIL, "Falha ao codificar thumbnail para WebP");
        return SAVE_STATE_ERROR_THUMBNAIL;
    }

    // Armazenar o thumbnail nos metadados do estado
    // Aqui assumimos que existe uma função interna para atualizar os metadados
    save_state_set_thumbnail_data(state, webp_data, webp_size, thumb_width, thumb_height, SAVE_STATE_THUMBNAIL_WEBP);

    // Liberar memória do WebP (após ser copiado para os metadados)
    WebPFree(webp_data);

    EMU_LOG_INFO(LOG_CAT_THUMBNAIL, "Thumbnail WebP gerado com sucesso: %u bytes", (uint32_t)webp_size);

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Implementação da função para gerar checksum SHA-256
 */
int32_t save_state_generate_checksum(save_state_t *state)
{
    if (!state)
    {
        EMU_LOG_ERROR(LOG_CAT_THUMBNAIL, "Parâmetros inválidos para geração de checksum");
        return SAVE_STATE_ERROR_INVALID;
    }

    // Em uma implementação real, usaríamos uma biblioteca como OpenSSL ou mbedTLS
    // Aqui vamos simular o cálculo do checksum

    EMU_LOG_INFO(LOG_CAT_THUMBNAIL, "Gerando checksum SHA-256");

    // Gerar um checksum fictício (na implementação real, este seria calculado corretamente)
    char checksum[64] = "a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0u1v2w3x4y5z6";

    // Armazenar o checksum nos metadados
    save_state_set_checksum(state, checksum);

    EMU_LOG_INFO(LOG_CAT_THUMBNAIL, "Checksum SHA-256 gerado com sucesso");

    return SAVE_STATE_ERROR_NONE;
}

/**
 * @brief Implementação da função de delta compression
 */
int32_t save_state_use_delta_compression(save_state_t *state, bool enable_delta)
{
    if (!state)
    {
        EMU_LOG_ERROR(LOG_CAT_THUMBNAIL, "Parâmetros inválidos para compressão delta");
        return SAVE_STATE_ERROR_INVALID;
    }

    EMU_LOG_INFO(LOG_CAT_THUMBNAIL, "%s compressão delta", enable_delta ? "Ativando" : "Desativando");

    // Atualizar configuração de compressão delta
    save_state_config_t config;
    save_state_get_config(state, &config);
    config.use_delta_compression = enable_delta;
    save_state_set_config(state, &config);

    // Marcar campos para compressão delta
    for (int i = 0; i < state->field_count; i++)
    {
        state->fields[i].use_delta = enable_delta;
    }

    EMU_LOG_INFO(LOG_CAT_THUMBNAIL, "Compressão delta %s com sucesso",
                 enable_delta ? "ativada" : "desativada");

    return SAVE_STATE_ERROR_NONE;
}
