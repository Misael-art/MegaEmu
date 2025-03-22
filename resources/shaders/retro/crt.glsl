#version 460

// Vertex Shader
#ifdef VERTEX_SHADER
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texcoord;

layout(location = 0) out vec2 v_texcoord;
layout(location = 1) out vec2 v_screen_pos;

void main()
{
    gl_Position = vec4(a_position, 1.0);
    v_texcoord = a_texcoord;
    v_screen_pos = a_position.xy;
}
#endif

// Fragment Shader
#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 v_texcoord;
layout(location = 1) in vec2 v_screen_pos;

layout(location = 0) out vec4 frag_color;

uniform sampler2D u_texture;
uniform vec2 u_screen_size;
uniform float u_curvature;
uniform float u_scanline_intensity;
uniform float u_mask_intensity;
uniform float u_bleed;
uniform float u_brightness;
uniform float u_contrast;
uniform float u_saturation;
uniform float u_vignette;
uniform bool u_phosphor;

// Constantes
const float PI = 3.14159265359;
const vec3 PHOSPHOR_R = vec3(1.0, 0.2, 0.2);
const vec3 PHOSPHOR_G = vec3(0.2, 1.0, 0.2);
const vec3 PHOSPHOR_B = vec3(0.2, 0.2, 1.0);

// Função para distorção de tela
vec2 apply_curvature(vec2 uv)
{
    if (u_curvature <= 0.0)
        return uv;

    // Centralizar coordenadas
    vec2 curved_uv = uv * 2.0 - 1.0;

    // Aplicar distorção
    float barrel = u_curvature * 0.3;
    float dist = length(curved_uv);
    float factor = 1.0 + dist * dist * barrel;
    curved_uv *= factor;

    // Voltar para coordenadas UV
    return (curved_uv * 0.5 + 0.5);
}

// Função para máscara de fósforo
vec3 apply_phosphor_mask(vec2 uv, vec3 color)
{
    if (u_mask_intensity <= 0.0)
        return color;

    // Calcular posição do pixel na grade
    vec2 mask_uv = uv * u_screen_size;
    int pixel_x = int(mod(mask_uv.x, 3.0));

    // Aplicar máscara RGB
    vec3 mask;
    if (pixel_x == 0)
        mask = PHOSPHOR_R;
    else if (pixel_x == 1)
        mask = PHOSPHOR_G;
    else
        mask = PHOSPHOR_B;

    // Misturar com cor original
    return mix(color, color * mask, u_mask_intensity);
}

// Função para scanlines
float apply_scanlines(vec2 uv)
{
    if (u_scanline_intensity <= 0.0)
        return 1.0;

    float scanline = sin(uv.y * u_screen_size.y * PI * 2.0) * 0.5 + 0.5;
    return 1.0 - (scanline * u_scanline_intensity);
}

// Função para sangramento de cores
vec3 apply_color_bleed(vec2 uv, vec3 color)
{
    if (u_bleed <= 0.0)
        return color;

    float bleed = u_bleed * 2.0;
    vec2 offset = vec2(1.0 / u_screen_size.x * bleed, 0.0);

    vec3 color_l = texture(u_texture, uv - offset).rgb;
    vec3 color_r = texture(u_texture, uv + offset).rgb;

    vec3 bleed_color;
    bleed_color.r = color.r;
    bleed_color.g = (color_l.g + color_r.g) * 0.5;
    bleed_color.b = (color_l.b + color_r.b) * 0.5;

    return mix(color, bleed_color, u_bleed);
}

// Função para vinheta
float apply_vignette(vec2 uv)
{
    if (u_vignette <= 0.0)
        return 1.0;

    float dist = length(uv * 2.0 - 1.0);
    return 1.0 - (dist * dist * u_vignette);
}

// Função para ajuste de cor
vec3 adjust_color(vec3 color)
{
    // Brilho
    color *= u_brightness;

    // Contraste
    color = (color - 0.5) * u_contrast + 0.5;

    // Saturação
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luminance), color, u_saturation);

    return clamp(color, 0.0, 1.0);
}

// Função para efeito de fósforo
vec3 apply_phosphor_persistence(vec3 color)
{
    if (!u_phosphor)
        return color;

    // Simular persistência do fósforo
    vec3 persistence = vec3(
        pow(color.r, 1.2),
        pow(color.g, 1.1),
        pow(color.b, 1.3)
    );

    return mix(color, persistence, 0.3);
}

void main()
{
    // Aplicar distorção de tela
    vec2 uv = apply_curvature(v_texcoord);

    // Verificar se o pixel está fora da tela
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    {
        frag_color = vec4(0.0);
        return;
    }

    // Obter cor base
    vec3 color = texture(u_texture, uv).rgb;

    // Aplicar efeitos
    color = apply_color_bleed(uv, color);
    color = apply_phosphor_mask(uv, color);
    color *= apply_scanlines(uv);
    color = apply_phosphor_persistence(color);
    color = adjust_color(color);
    color *= apply_vignette(uv);

    // Resultado final
    frag_color = vec4(color, 1.0);
}
#endif
