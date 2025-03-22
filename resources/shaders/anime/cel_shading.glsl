#version 460

// Vertex Shader
#ifdef VERTEX_SHADER
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoord;

layout(location = 0) out vec3 v_normal;
layout(location = 1) out vec2 v_texcoord;
layout(location = 2) out vec3 v_view_pos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    v_normal = mat3(transpose(inverse(u_model))) * a_normal;
    v_texcoord = a_texcoord;
    vec4 world_pos = u_model * vec4(a_position, 1.0);
    v_view_pos = (u_view * world_pos).xyz;
    gl_Position = u_projection * u_view * world_pos;
}
#endif

// Fragment Shader
#ifdef FRAGMENT_SHADER
layout(location = 0) in vec3 v_normal;
layout(location = 1) in vec2 v_texcoord;
layout(location = 2) in vec3 v_view_pos;

layout(location = 0) out vec4 frag_color;

uniform sampler2D u_texture;
uniform vec3 u_light_dir;
uniform vec3 u_light_color;
uniform vec3 u_ambient_color;
uniform int u_shade_levels;
uniform float u_shade_thresholds[4];
uniform float u_smoothness;
uniform bool u_specular;

// Função para quantizar valor em níveis
float quantize(float value, int levels)
{
    float step = 1.0 / float(levels - 1);
    return floor(value * float(levels - 1) + 0.5) * step;
}

// Função para suavizar transições
float smooth_step(float edge0, float edge1, float x)
{
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

void main()
{
    // Normalizar vetores
    vec3 N = normalize(v_normal);
    vec3 L = normalize(-u_light_dir);
    vec3 V = normalize(-v_view_pos);
    vec3 H = normalize(L + V);

    // Calcular iluminação difusa
    float NdotL = max(dot(N, L), 0.0);

    // Quantizar iluminação em níveis
    float diffuse = 0.0;
    float step_size = 1.0 / float(u_shade_levels - 1);

    for (int i = 0; i < u_shade_levels - 1; i++)
    {
        float threshold = u_shade_thresholds[i];
        float next_threshold = (i < u_shade_levels - 2) ? u_shade_thresholds[i + 1] : 1.0;

        if (NdotL >= threshold)
        {
            float t = u_smoothness > 0.0 ?
                      smooth_step(threshold, threshold + u_smoothness, NdotL) :
                      1.0;

            diffuse = mix(float(i) * step_size, float(i + 1) * step_size, t);
        }
    }

    // Calcular especular se habilitado
    float specular = 0.0;
    if (u_specular)
    {
        float NdotH = max(dot(N, H), 0.0);
        specular = pow(NdotH, 32.0);
        specular = quantize(specular, 2); // Apenas dois níveis para especular
    }

    // Combinar texturas e iluminação
    vec4 tex_color = texture(u_texture, v_texcoord);
    vec3 ambient = u_ambient_color * tex_color.rgb;
    vec3 diffuse_color = u_light_color * tex_color.rgb * diffuse;
    vec3 specular_color = u_light_color * specular;

    // Resultado final
    frag_color = vec4(ambient + diffuse_color + specular_color, tex_color.a);
}
#endif
