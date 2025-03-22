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
uniform float u_time;
uniform vec2 u_direction;
uniform float u_intensity;
uniform float u_length;
uniform vec4 u_line_color;
uniform float u_density;
uniform vec2 u_screen_size;

// Função de ruído 2D
float random(vec2 st)
{
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Função de ruído com valor
float value_noise(vec2 st)
{
    vec2 i = floor(st);
    vec2 f = fract(st);

    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
           (c - a)* u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

// Função para gerar linhas de velocidade
float speed_lines(vec2 uv, vec2 direction, float time)
{
    // Rotacionar UV baseado na direção
    float angle = atan(direction.y, direction.x);
    mat2 rotation = mat2(cos(angle), -sin(angle),
                        sin(angle), cos(angle));
    vec2 rotated_uv = rotation * uv;

    // Gerar várias camadas de linhas
    float lines = 0.0;
    float scale = 1.0;
    float amplitude = 1.0;

    for (int i = 0; i < 3; i++)
    {
        // Deslocar UV baseado no tempo e direção
        vec2 displaced_uv = rotated_uv * scale + vec2(time * (1.0 + float(i) * 0.5), 0.0);

        // Gerar linhas usando ruído
        float noise = value_noise(displaced_uv);
        float line = smoothstep(0.5, 0.51, noise);

        // Adicionar variação na espessura
        float thickness = 0.1 + 0.2 * value_noise(displaced_uv * 2.0);
        line *= smoothstep(thickness, 0.0, abs(rotated_uv.y));

        lines += line * amplitude;

        scale *= 2.0;
        amplitude *= 0.5;
    }

    return lines;
}

void main()
{
    // Obter cor original
    vec4 color = texture(u_texture, v_texcoord);

    // Calcular coordenadas normalizadas
    vec2 uv = (v_screen_pos * 0.5 + 0.5) * u_screen_size;
    uv = uv * 2.0 - 1.0;

    // Gerar linhas de velocidade
    float lines = speed_lines(uv * u_density,
                            normalize(u_direction),
                            u_time * u_length);

    // Aplicar intensidade e cor
    vec4 line_effect = u_line_color * lines * u_intensity;

    // Misturar com a cor original
    frag_color = mix(color, line_effect, line_effect.a * u_intensity);

    // Adicionar brilho nas intersecções
    float glow = lines * 0.5 * u_intensity;
    frag_color.rgb += u_line_color.rgb * glow;

    // Garantir que alpha seja preservado
    frag_color.a = color.a;
}
#endif
