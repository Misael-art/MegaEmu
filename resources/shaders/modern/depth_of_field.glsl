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

uniform sampler2D u_scene_texture;    // Cena renderizada
uniform sampler2D u_depth_texture;    // Buffer de profundidade
uniform float u_focal_distance;       // Distância focal
uniform float u_focal_range;          // Range focal
uniform float u_blur_strength;        // Intensidade do desfoque
uniform int u_blur_samples;           // Amostras de blur
uniform bool u_use_bokeh;            // Efeito bokeh
uniform float u_bokeh_threshold;      // Threshold do bokeh
uniform vec2 u_screen_size;          // Tamanho da tela

// Constantes
const float PI = 3.14159265359;
const int MAX_SAMPLES = 64;

// Função para calcular peso gaussiano
float gaussian(float x, float sigma)
{
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

// Função para calcular círculo de confusão
float calculate_coc(float depth)
{
    float distance = abs(depth - u_focal_distance);
    return clamp(distance / u_focal_range, 0.0, 1.0) * u_blur_strength;
}

// Função para gerar posição de amostra bokeh
vec2 bokeh_sample(int index, float radius, float rotation)
{
    float samples = float(u_blur_samples);
    float angle = float(index) * (2.0 * PI / samples) + rotation;
    float r = sqrt(float(index) / samples) * radius;
    return vec2(cos(angle), sin(angle)) * r;
}

// Função para calcular peso bokeh
float calculate_bokeh_weight(vec4 color)
{
    if (!u_use_bokeh)
        return 1.0;

    float luminance = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    return luminance > u_bokeh_threshold ? luminance : 1.0;
}

// Função para desfoque circular
vec4 circular_blur(vec2 uv, float radius)
{
    vec4 color = vec4(0.0);
    float total_weight = 0.0;
    float rotation = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453) * PI;

    // Limitar número de amostras
    int samples = min(u_blur_samples, MAX_SAMPLES);

    for (int i = 0; i < samples; i++)
    {
        // Calcular offset da amostra
        vec2 offset = bokeh_sample(i, radius, rotation);
        vec2 sample_uv = uv + offset / u_screen_size;

        // Amostrar cor
        vec4 sample_color = texture(u_scene_texture, sample_uv);

        // Calcular peso
        float weight = 1.0;
        if (u_use_bokeh)
        {
            weight = calculate_bokeh_weight(sample_color);
        }
        else
        {
            weight = gaussian(length(offset), radius);
        }

        // Acumular cor
        color += sample_color * weight;
        total_weight += weight;
    }

    return color / total_weight;
}

void main()
{
    // Obter profundidade
    float depth = texture(u_depth_texture, v_texcoord).r;

    // Calcular círculo de confusão
    float coc = calculate_coc(depth);

    // Se o pixel está em foco, usar cor original
    if (coc < 0.01)
    {
        frag_color = texture(u_scene_texture, v_texcoord);
        return;
    }

    // Aplicar desfoque baseado no círculo de confusão
    vec4 blurred = circular_blur(v_texcoord, coc);

    // Transição suave entre foco e desfoque
    vec4 original = texture(u_scene_texture, v_texcoord);
    float blend = smoothstep(0.0, 0.02, coc);

    // Resultado final
    frag_color = mix(original, blurred, blend);

    // Adicionar efeito de brilho para áreas desfocadas brilhantes
    if (u_use_bokeh)
    {
        float bokeh_intensity = calculate_bokeh_weight(blurred);
        vec3 bokeh_color = blurred.rgb * bokeh_intensity * coc;
        frag_color.rgb += bokeh_color * 0.5;
    }
}
#endif
