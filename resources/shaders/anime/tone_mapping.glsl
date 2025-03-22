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
uniform int u_pattern_type;
uniform float u_scale;
uniform float u_threshold;
uniform float u_rotation;
uniform bool u_inverted;
uniform vec2 u_screen_size;

// Funções de padrões de trama
float pattern_dots(vec2 uv)
{
    vec2 p = fract(uv * u_scale);
    return length(p - 0.5) > 0.4 ? 1.0 : 0.0;
}

float pattern_lines(vec2 uv)
{
    return fract(uv.x * u_scale) > 0.5 ? 1.0 : 0.0;
}

float pattern_crosshatch(vec2 uv)
{
    float lines1 = pattern_lines(uv);
    float lines2 = pattern_lines(vec2(uv.y, uv.x));
    return min(lines1, lines2);
}

float pattern_diagonal(vec2 uv)
{
    return fract((uv.x + uv.y) * u_scale) > 0.5 ? 1.0 : 0.0;
}

float pattern_circles(vec2 uv)
{
    vec2 p = fract(uv * u_scale) - 0.5;
    float r = length(p);
    return smoothstep(0.3, 0.31, r);
}

// Função para rotacionar UV
vec2 rotate_uv(vec2 uv, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    mat2 rotation = mat2(c, -s, s, c);
    return (rotation * (uv - 0.5)) + 0.5;
}

// Função para aplicar padrão de trama
float apply_pattern(vec2 uv, int pattern_type)
{
    // Rotacionar UV
    uv = rotate_uv(uv, radians(u_rotation));

    float pattern = 0.0;

    switch (pattern_type)
    {
        case 0: // Pontos
            pattern = pattern_dots(uv);
            break;

        case 1: // Linhas
            pattern = pattern_lines(uv);
            break;

        case 2: // Crosshatch
            pattern = pattern_crosshatch(uv);
            break;

        case 3: // Diagonal
            pattern = pattern_diagonal(uv);
            break;

        case 4: // Círculos
            pattern = pattern_circles(uv);
            break;

        case 5: // Padrão composto
            float p1 = pattern_dots(uv * 2.0);
            float p2 = pattern_lines(uv);
            pattern = mix(p1, p2, 0.5);
            break;
    }

    return u_inverted ? 1.0 - pattern : pattern;
}

void main()
{
    // Normalizar coordenadas
    vec2 uv = v_texcoord;
    vec2 aspect_ratio = vec2(u_screen_size.x / u_screen_size.y, 1.0);
    uv = uv * aspect_ratio;

    // Obter cor original
    vec4 color = texture(u_texture, v_texcoord);

    // Calcular luminância
    float luminance = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));

    // Aplicar padrão de trama baseado na luminância
    float pattern = apply_pattern(uv, u_pattern_type);
    float threshold = step(u_threshold, luminance);

    // Combinar padrão com a cor original
    vec4 pattern_color = vec4(vec3(pattern), 1.0);
    vec4 result = mix(pattern_color, color, threshold);

    // Adicionar efeito de papel
    float paper = fract(sin(dot(uv * 100.0, vec2(12.9898, 78.233))) * 43758.5453);
    result += vec4(paper * 0.02);

    // Resultado final
    frag_color = result;
}
#endif
