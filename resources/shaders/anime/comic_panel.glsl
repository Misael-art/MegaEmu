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
uniform int u_layout;
uniform float u_border_thickness;
uniform vec4 u_border_color;
uniform float u_panel_gap;
uniform bool u_dynamic;
uniform vec2 u_screen_size;
uniform float u_time;

// Função para desenhar borda
float draw_border(vec2 uv, float thickness)
{
    vec2 border = smoothstep(0.0, thickness, uv) *
                  smoothstep(0.0, thickness, 1.0 - uv);
    return border.x * border.y;
}

// Função para calcular layout dinâmico
vec4 calculate_dynamic_panel(vec2 uv, float time)
{
    // Calcular ângulo baseado no tempo
    float angle = time * 0.5;
    mat2 rotation = mat2(cos(angle), -sin(angle),
                        sin(angle), cos(angle));

    // Aplicar rotação ao UV
    vec2 rotated_uv = (rotation * (uv - 0.5)) + 0.5;

    // Criar padrão de divisão dinâmico
    float diagonal = sin(rotated_uv.x * 10.0 + time) * 0.5 + 0.5;
    float panel = step(rotated_uv.y, diagonal);

    return vec4(panel);
}

// Função para aplicar layout de painéis
vec4 apply_panel_layout(vec2 uv, int layout_type)
{
    vec4 panel = vec4(1.0);
    float gap = u_panel_gap / 100.0;

    switch (layout_type)
    {
        case 1: // Layout 2x2
            vec2 grid = step(vec2(0.5 + gap), uv) * step(vec2(0.5 + gap), 1.0 - uv);
            panel = vec4(grid.x * grid.y);
            break;

        case 2: // Layout diagonal
            float diagonal = step(uv.x + gap, uv.y) * step(uv.y, uv.x + 1.0 - gap);
            panel = vec4(diagonal);
            break;

        case 3: // Layout triplo vertical
            float thirds = step(1.0/3.0 + gap, fract(uv.x * 3.0));
            panel = vec4(thirds);
            break;

        case 4: // Layout manga dinâmico
            if (u_dynamic)
            {
                panel = calculate_dynamic_panel(uv, u_time);
            }
            break;

        case 5: // Layout espiral
            vec2 center = uv - 0.5;
            float angle = atan(center.y, center.x);
            float radius = length(center);
            float spiral = step(0.1, mod(angle + radius * 5.0 + u_time * 0.5, 3.14159 * 2.0));
            panel = vec4(spiral);
            break;
    }

    return panel;
}

void main()
{
    // Normalizar coordenadas
    vec2 uv = v_texcoord;
    vec2 aspect_ratio = vec2(u_screen_size.x / u_screen_size.y, 1.0);
    uv = uv * aspect_ratio;

    // Obter cor original
    vec4 color = texture(u_texture, v_texcoord);

    // Aplicar layout de painéis
    vec4 panel = apply_panel_layout(uv, u_layout);

    // Desenhar bordas
    float border = 1.0 - draw_border(uv, u_border_thickness / 100.0);

    // Combinar efeitos
    vec4 result = mix(color, u_border_color, border * panel.a);

    // Adicionar efeitos de sombra nas bordas
    float shadow = smoothstep(0.0, 0.1, border) * 0.5;
    result = mix(result, vec4(0.0, 0.0, 0.0, 1.0), shadow * panel.a);

    // Adicionar efeito de papel
    float paper = fract(sin(dot(uv * 100.0, vec2(12.9898, 78.233))) * 43758.5453);
    result += vec4(paper * 0.03);

    // Resultado final
    frag_color = result;
}
#endif
