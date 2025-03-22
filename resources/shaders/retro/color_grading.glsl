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
uniform sampler2D u_lut_texture;
uniform int u_color_depth;
uniform float u_gamma;
uniform bool u_use_lut;
uniform vec3 u_palette[256];
uniform int u_palette_size;

// Constantes
const float LUT_SIZE = 32.0;
const float LUT_CELLS = LUT_SIZE - 1.0;

// Função para aplicar LUT
vec3 apply_lut(vec3 color)
{
    // Calcular coordenadas na LUT
    float blue = color.b * LUT_CELLS;
    float b1 = floor(blue);
    float b2 = ceil(blue);
    float bfrac = blue - b1;

    // Converter para coordenadas 2D na textura da LUT
    vec2 quad1 = vec2(color.r, color.g);
    float row1 = floor(b1 / LUT_SIZE);
    float col1 = mod(b1, LUT_SIZE);
    vec2 uv1 = (quad1 + vec2(col1, row1)) / LUT_SIZE;

    vec2 quad2 = vec2(color.r, color.g);
    float row2 = floor(b2 / LUT_SIZE);
    float col2 = mod(b2, LUT_SIZE);
    vec2 uv2 = (quad2 + vec2(col2, row2)) / LUT_SIZE;

    // Amostrar LUT
    vec3 color1 = texture(u_lut_texture, uv1).rgb;
    vec3 color2 = texture(u_lut_texture, uv2).rgb;

    // Interpolar entre as fatias
    return mix(color1, color2, bfrac);
}

// Função para encontrar cor mais próxima na paleta
vec3 find_nearest_color(vec3 color)
{
    float min_dist = 10000.0;
    vec3 nearest = color;

    for (int i = 0; i < u_palette_size; i++)
    {
        vec3 pal_color = u_palette[i].rgb;
        float dist = length(color - pal_color);

        if (dist < min_dist)
        {
            min_dist = dist;
            nearest = pal_color;
        }
    }

    return nearest;
}

// Função para correção gamma
vec3 apply_gamma(vec3 color)
{
    return pow(color, vec3(1.0 / u_gamma));
}

// Função para quantização de cores
vec3 quantize_color(vec3 color)
{
    if (u_color_depth >= 32)
        return color;

    float levels = pow(2.0, float(u_color_depth) / 3.0);
    return floor(color * levels) / levels;
}

void main()
{
    // Obter cor original
    vec4 color = texture(u_texture, v_texcoord);

    // Aplicar correção gamma
    color.rgb = apply_gamma(color.rgb);

    // Aplicar LUT se disponível
    if (u_use_lut)
    {
        color.rgb = apply_lut(color.rgb);
    }

    // Quantizar cores se necessário
    if (u_color_depth < 32)
    {
        if (u_palette_size > 0)
        {
            // Usar paleta personalizada
            color.rgb = find_nearest_color(color.rgb);
        }
        else
        {
            // Quantização simples
            color.rgb = quantize_color(color.rgb);
        }
    }

    frag_color = color;
}
#endif
