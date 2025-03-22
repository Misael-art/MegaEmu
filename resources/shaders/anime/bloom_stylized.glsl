#version 460

// Vertex Shader
#ifdef VERTEX_SHADER
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texcoord;

layout(location = 0) out vec2 v_texcoord;

void main()
{
    gl_Position = vec4(a_position, 1.0);
    v_texcoord = a_texcoord;
}
#endif

// Fragment Shader
#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 v_texcoord;

layout(location = 0) out vec4 frag_color;

uniform sampler2D u_texture;
uniform float u_intensity;
uniform float u_threshold;
uniform float u_radius;
uniform vec4 u_tint;
uniform bool u_anamorphic;
uniform vec2 u_screen_size;

// Função para calcular peso gaussiano
float gaussian(float x, float sigma)
{
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

// Função para extrair brilhos
vec4 extract_bright(vec4 color)
{
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    return color * smoothstep(u_threshold, u_threshold + 0.1, brightness);
}

// Função para blur horizontal
vec4 blur_horizontal(sampler2D tex, vec2 uv, float radius)
{
    vec4 color = vec4(0.0);
    float total_weight = 0.0;

    // Ajustar raio para efeito anamórfico
    float adjusted_radius = u_anamorphic ? radius * 2.0 : radius;

    // Número de amostras baseado no raio
    int samples = int(adjusted_radius * 2.0);

    for (int i = -samples; i <= samples; i++)
    {
        float offset = float(i) / float(samples) * adjusted_radius;
        vec2 sample_uv = uv + vec2(offset / u_screen_size.x, 0.0);

        float weight = gaussian(float(i) / float(samples), 0.5);
        color += texture(tex, sample_uv) * weight;
        total_weight += weight;
    }

    return color / total_weight;
}

// Função para blur vertical
vec4 blur_vertical(sampler2D tex, vec2 uv, float radius)
{
    vec4 color = vec4(0.0);
    float total_weight = 0.0;

    // Ajustar raio para efeito anamórfico
    float adjusted_radius = u_anamorphic ? radius * 0.5 : radius;

    // Número de amostras baseado no raio
    int samples = int(adjusted_radius * 2.0);

    for (int i = -samples; i <= samples; i++)
    {
        float offset = float(i) / float(samples) * adjusted_radius;
        vec2 sample_uv = uv + vec2(0.0, offset / u_screen_size.y);

        float weight = gaussian(float(i) / float(samples), 0.5);
        color += texture(tex, sample_uv) * weight;
        total_weight += weight;
    }

    return color / total_weight;
}

// Função para adicionar brilho estilizado
vec4 add_stylized_glow(vec4 original, vec4 bloom)
{
    // Aplicar tint ao bloom
    vec4 tinted_bloom = bloom * u_tint;

    // Adicionar efeito de brilho mais intenso no centro
    float center_boost = 1.0 - length(v_texcoord - vec2(0.5)) * 2.0;
    center_boost = max(0.0, center_boost * center_boost);

    // Adicionar variação de intensidade baseada na luminância
    float luminance = dot(original.rgb, vec3(0.2126, 0.7152, 0.0722));
    float intensity_var = mix(0.8, 1.2, luminance);

    // Combinar com a cor original
    vec4 result = original + tinted_bloom * u_intensity * intensity_var;

    // Adicionar brilho extra no centro
    result += tinted_bloom * center_boost * u_intensity * 0.5;

    // Garantir que não haja oversaturation
    result.rgb = mix(original.rgb, result.rgb, u_intensity);

    return vec4(result.rgb, original.a);
}

void main()
{
    // Obter cor original
    vec4 color = texture(u_texture, v_texcoord);

    // Extrair áreas brilhantes
    vec4 bright = extract_bright(color);

    // Aplicar blur horizontal
    vec4 blur_h = blur_horizontal(u_texture, v_texcoord, u_radius);

    // Aplicar blur vertical
    vec4 blur_v = blur_vertical(u_texture, v_texcoord, u_radius);

    // Combinar blurs
    vec4 bloom = (blur_h + blur_v) * 0.5;

    // Aplicar bloom estilizado
    frag_color = add_stylized_glow(color, bloom);
}
#endif
