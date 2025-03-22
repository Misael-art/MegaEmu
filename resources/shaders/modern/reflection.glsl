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

uniform sampler2D u_scene_texture;     // Cena renderizada
uniform sampler2D u_depth_texture;     // Buffer de profundidade
uniform sampler2D u_normal_texture;    // Buffer de normais
uniform float u_reflection_strength;   // Intensidade dos reflexos
uniform float u_roughness;            // Rugosidade da superfície
uniform float u_fresnel;              // Efeito fresnel
uniform int u_max_steps;              // Passos de ray marching
uniform float u_thickness;            // Espessura da superfície
uniform bool u_use_blur;              // Desfocar reflexos
uniform float u_blur_radius;          // Raio do desfoque
uniform vec2 u_screen_size;           // Tamanho da tela
uniform float u_time;                 // Tempo para animação

// Constantes
const float PI = 3.14159265359;
const float MAX_DISTANCE = 1.0;

// Função para ruído 2D
float noise2D(vec2 st)
{
    vec2 i = floor(st);
    vec2 f = fract(st);
    float a = fract(sin(dot(i, vec2(12.9898, 78.233))) * 43758.5453);
    float b = fract(sin(dot(i + vec2(1.0, 0.0), vec2(12.9898, 78.233))) * 43758.5453);
    float c = fract(sin(dot(i + vec2(0.0, 1.0), vec2(12.9898, 78.233))) * 43758.5453);
    float d = fract(sin(dot(i + vec2(1.0, 1.0), vec2(12.9898, 78.233))) * 43758.5453);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

// Função para calcular normal da superfície
vec3 get_normal(vec2 uv)
{
    vec3 normal = texture(u_normal_texture, uv).rgb * 2.0 - 1.0;
    return normalize(normal);
}

// Função para calcular profundidade
float get_depth(vec2 uv)
{
    return texture(u_depth_texture, uv).r;
}

// Função para calcular reflexão
vec2 calculate_reflection(vec2 uv, vec3 normal)
{
    // Direção do raio
    vec3 view_dir = normalize(vec3(0.0, 0.0, 1.0));
    vec3 reflect_dir = reflect(-view_dir, normal);

    // Projetar direção de reflexão em 2D
    vec2 reflect_uv = uv;
    float step_size = (1.0 - u_roughness) * 0.1;

    // Ray marching
    float current_depth = get_depth(uv);

    for (int i = 0; i < u_max_steps; i++)
    {
        reflect_uv += reflect_dir.xy * step_size;

        // Verificar limites da tela
        if (any(lessThan(reflect_uv, vec2(0.0))) || any(greaterThan(reflect_uv, vec2(1.0))))
            break;

        float sample_depth = get_depth(reflect_uv);

        // Verificar interseção
        if (sample_depth < current_depth - u_thickness)
        {
            return reflect_uv;
        }

        current_depth = sample_depth;
    }

    return uv;
}

// Função para aplicar desfoque gaussiano
vec4 apply_blur(sampler2D tex, vec2 uv, float radius)
{
    vec4 color = vec4(0.0);
    float total_weight = 0.0;

    for (int x = -2; x <= 2; x++)
    {
        for (int y = -2; y <= 2; y++)
        {
            vec2 offset = vec2(float(x), float(y)) * radius / u_screen_size;
            float weight = exp(-dot(offset, offset));
            color += texture(tex, uv + offset) * weight;
            total_weight += weight;
        }
    }

    return color / total_weight;
}

// Função para calcular termo Fresnel
float calculate_fresnel(vec3 normal, vec3 view_dir)
{
    float base = 1.0 - max(dot(normal, view_dir), 0.0);
    return pow(base, u_fresnel);
}

void main()
{
    // Obter cor da cena
    vec4 scene_color = texture(u_scene_texture, v_texcoord);

    // Obter normal e profundidade
    vec3 normal = get_normal(v_texcoord);
    float depth = get_depth(v_texcoord);

    // Calcular reflexão apenas em superfícies reflexivas
    vec4 reflection_color = scene_color;
    if (depth < 1.0) // Superfície visível
    {
        // Calcular coordenadas de reflexão
        vec2 reflect_uv = calculate_reflection(v_texcoord, normal);

        // Amostrar reflexão
        if (u_use_blur)
        {
            reflection_color = apply_blur(u_scene_texture, reflect_uv, u_blur_radius);
        }
        else
        {
            reflection_color = texture(u_scene_texture, reflect_uv);
        }

        // Adicionar variação baseada em ruído
        float noise = noise2D(v_texcoord * 10.0 + vec2(u_time * 0.1));
        reflection_color += vec4(noise * 0.1);

        // Calcular termo Fresnel
        vec3 view_dir = normalize(vec3(0.0, 0.0, 1.0));
        float fresnel = calculate_fresnel(normal, view_dir);

        // Combinar com a cena
        float reflection_factor = u_reflection_strength * fresnel;
        scene_color = mix(scene_color, reflection_color, reflection_factor);
    }

    frag_color = scene_color;
}
#endif
