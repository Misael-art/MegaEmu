#version 460

// Vertex Shader
#ifdef VERTEX_SHADER
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoord;

layout(location = 0) out vec2 v_texcoord;
layout(location = 1) out vec2 v_screen_pos;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

void main()
{
    vec4 pos = u_projection * u_view * u_model * vec4(a_position, 1.0);
    gl_Position = pos;
    v_texcoord = a_texcoord;
    v_screen_pos = pos.xy / pos.w;
}
#endif

// Fragment Shader
#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 v_texcoord;
layout(location = 1) in vec2 v_screen_pos;

layout(location = 0) out vec4 frag_color;

uniform sampler2D u_texture;
uniform sampler2D u_depth_texture;
uniform vec2 u_screen_size;
uniform float u_thickness;
uniform vec4 u_outline_color;
uniform float u_edge_threshold;
uniform bool u_color_based;
uniform bool u_depth_based;

// Função para detectar bordas usando Sobel
vec4 detect_edges(sampler2D tex, vec2 uv, vec2 pixel_size)
{
    // Kernels Sobel
    float kernel_x[9] = float[](
        -1.0, 0.0, 1.0,
        -2.0, 0.0, 2.0,
        -1.0, 0.0, 1.0
    );

    float kernel_y[9] = float[](
        -1.0, -2.0, -1.0,
        0.0, 0.0, 0.0,
        1.0, 2.0, 1.0
    );

    vec4 color_sum_x = vec4(0.0);
    vec4 color_sum_y = vec4(0.0);

    int index = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            vec2 offset = vec2(float(j), float(i)) * pixel_size;
            vec4 color = texture(tex, uv + offset);

            color_sum_x += color * kernel_x[index];
            color_sum_y += color * kernel_y[index];

            index++;
        }
    }

    return sqrt(color_sum_x * color_sum_x + color_sum_y * color_sum_y);
}

// Função para detectar bordas usando profundidade
float detect_depth_edges(sampler2D depth_tex, vec2 uv, vec2 pixel_size)
{
    float depth = texture(depth_tex, uv).r;
    float depth_up = texture(depth_tex, uv + vec2(0.0, pixel_size.y)).r;
    float depth_down = texture(depth_tex, uv + vec2(0.0, -pixel_size.y)).r;
    float depth_right = texture(depth_tex, uv + vec2(pixel_size.x, 0.0)).r;
    float depth_left = texture(depth_tex, uv + vec2(-pixel_size.x, 0.0)).r;

    float edge_h = abs(depth_right - depth_left);
    float edge_v = abs(depth_up - depth_down);

    return max(edge_h, edge_v);
}

void main()
{
    vec2 pixel_size = vec2(1.0) / u_screen_size;
    vec4 color = texture(u_texture, v_texcoord);
    float edge = 0.0;

    // Detecção de borda baseada em cor
    if (u_color_based)
    {
        vec4 edges = detect_edges(u_texture, v_texcoord, pixel_size * u_thickness);
        edge = max(max(edges.r, edges.g), edges.b);
    }

    // Detecção de borda baseada em profundidade
    if (u_depth_based)
    {
        float depth_edge = detect_depth_edges(u_depth_texture, v_texcoord, pixel_size * u_thickness);
        edge = max(edge, depth_edge);
    }

    // Aplicar threshold e cor do contorno
    if (edge > u_edge_threshold)
    {
        // Misturar cor do contorno com a cor original baseado na intensidade da borda
        float blend = smoothstep(u_edge_threshold, u_edge_threshold + 0.1, edge);
        frag_color = mix(color, u_outline_color, blend);
    }
    else
    {
        frag_color = color;
    }
}
#endif
