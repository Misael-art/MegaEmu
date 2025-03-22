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
uniform int u_dither_type;
uniform float u_dither_strength;
uniform int u_pattern_size;
uniform float u_threshold;
uniform bool u_color_dither;

// Matriz de Bayer 8x8
const float bayer8x8[64] = float[](
     0, 32,  8, 40,  2, 34, 10, 42,
    48, 16, 56, 24, 50, 18, 58, 26,
    12, 44,  4, 36, 14, 46,  6, 38,
    60, 28, 52, 20, 62, 30, 54, 22,
     3, 35, 11, 43,  1, 33,  9, 41,
    51, 19, 59, 27, 49, 17, 57, 25,
    15, 47,  7, 39, 13, 45,  5, 37,
    63, 31, 55, 23, 61, 29, 53, 21
);

// Função para dithering ordenado
float ordered_dither(vec2 pos)
{
    int x = int(mod(pos.x, u_pattern_size));
    int y = int(mod(pos.y, u_pattern_size));
    int index = y * u_pattern_size + x;

    // Usar matriz de Bayer apropriada
    float dither_value;
    if (u_pattern_size == 8)
    {
        dither_value = bayer8x8[index] / 64.0;
    }
    else
    {
        // Fallback para padrão 2x2
        float bayer2x2[4] = float[](0.0, 0.5, 0.75, 0.25);
        index = (y % 2) * 2 + (x % 2);
        dither_value = bayer2x2[index];
    }

    return dither_value - 0.5;
}

// Função para dithering Floyd-Steinberg
vec3 floyd_steinberg_dither(vec2 uv, vec3 color)
{
    vec2 pixel_size = 1.0 / u_screen_size;
    vec3 error = vec3(0.0);

    // Propagar erro para pixels vizinhos
    error += texture(u_texture, uv + pixel_size * vec2( 1,  0)).rgb * 7.0/16.0;
    error += texture(u_texture, uv + pixel_size * vec2(-1,  1)).rgb * 3.0/16.0;
    error += texture(u_texture, uv + pixel_size * vec2( 0,  1)).rgb * 5.0/16.0;
    error += texture(u_texture, uv + pixel_size * vec2( 1,  1)).rgb * 1.0/16.0;

    return color + error * u_dither_strength;
}

// Função para dithering blue noise
float blue_noise_dither(vec2 pos)
{
    // Usar textura de ruído azul ou gerar proceduralmente
    float noise = fract(sin(dot(pos, vec2(12.9898, 78.233))) * 43758.5453);
    return noise - 0.5;
}

// Função para quantizar cor
vec3 quantize_color(vec3 color, float dither)
{
    // Aplicar dithering
    color += dither * u_dither_strength;

    // Quantizar para níveis de cor desejados
    float levels = 32.0; // Ajustar baseado na profundidade de cor desejada
    return floor(color * levels) / levels;
}

void main()
{
    // Obter cor original
    vec4 color = texture(u_texture, v_texcoord);

    // Calcular valor de dithering
    float dither = 0.0;
    vec2 pos = gl_FragCoord.xy;

    switch (u_dither_type)
    {
        case 0: // Ordered dithering
            dither = ordered_dither(pos);
            break;

        case 1: // Floyd-Steinberg
            color.rgb = floyd_steinberg_dither(v_texcoord, color.rgb);
            break;

        case 2: // Bayer matrix
            dither = ordered_dither(pos); // Usar matriz de Bayer 8x8
            break;

        case 3: // Blue noise
            dither = blue_noise_dither(pos);
            break;
    }

    // Aplicar dithering
    if (u_color_dither)
    {
        // Dithering por canal de cor
        color.r = quantize_color(vec3(color.r), dither).r;
        color.g = quantize_color(vec3(color.g), dither).g;
        color.b = quantize_color(vec3(color.b), dither).b;
    }
    else
    {
        // Dithering em escala de cinza
        float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        float dithered = quantize_color(vec3(gray), dither).r;
        color.rgb = vec3(dithered);
    }

    // Aplicar threshold
    if (u_threshold > 0.0)
    {
        color.rgb = step(u_threshold, color.rgb);
    }

    frag_color = color;
}
#endif
