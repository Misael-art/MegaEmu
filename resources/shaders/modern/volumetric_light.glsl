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
uniform sampler2D u_mask_texture;     // Máscara de obstáculos
uniform sampler2D u_noise_texture;    // Ruído para variação
uniform vec2 u_light_pos;            // Posição da luz em pixels
uniform vec4 u_light_color;          // Cor da luz
uniform float u_density;             // Densidade dos raios
uniform float u_scatter;            // Dispersão da luz
uniform int u_num_samples;          // Amostras de ray marching
uniform float u_falloff;           // Atenuação da luz
uniform float u_noise_scale;       // Escala do ruído
uniform bool u_use_noise;          // Usar ruído
uniform vec2 u_screen_size;        // Tamanho da tela
uniform float u_time;              // Tempo para animação

// Função para ruído 2D
float noise2D(vec2 st)
{
    if (u_use_noise)
    {
        vec2 noise_uv = st * u_noise_scale + vec2(u_time * 0.1);
        return texture(u_noise_texture, noise_uv).r;
    }
    return 1.0;
}

// Função para calcular atenuação da luz
float calculate_attenuation(float distance)
{
    return 1.0 / (1.0 + u_falloff * distance * distance);
}

// Função para calcular dispersão da luz
float calculate_scatter(vec2 ray_dir, vec2 light_dir)
{
    float cos_angle = dot(normalize(ray_dir), normalize(light_dir));
    return pow(max(0.0, cos_angle), u_scatter);
}

// Função para ray marching
vec4 ray_march(vec2 uv)
{
    // Converter posição da luz para coordenadas UV
    vec2 light_uv = u_light_pos / u_screen_size;

    // Direção do raio
    vec2 ray_dir = light_uv - uv;
    float ray_length = length(ray_dir);
    ray_dir = ray_dir / ray_length;

    // Configurar ray marching
    float step_size = ray_length / float(u_num_samples);
    vec2 step_vector = ray_dir * step_size;
    vec2 current_pos = uv;

    // Acumular luz
    vec4 accumulated_light = vec4(0.0);
    float transmittance = 1.0;

    for (int i = 0; i < u_num_samples; i++)
    {
        // Amostrar máscara de obstáculos
        float mask = texture(u_mask_texture, current_pos).r;

        // Se não houver obstáculo
        if (mask > 0.0)
        {
            // Calcular atenuação baseada na distância
            float distance = length(current_pos - light_uv);
            float attenuation = calculate_attenuation(distance);

            // Calcular dispersão
            float scatter = calculate_scatter(ray_dir, light_uv - current_pos);

            // Aplicar ruído para variação
            float noise = noise2D(current_pos);

            // Acumular luz
            vec4 sample_light = u_light_color * attenuation * scatter * noise * mask;
            accumulated_light += sample_light * transmittance * u_density * step_size;

            // Atualizar transmitância
            transmittance *= exp(-u_density * step_size);

            // Otimização: parar se a transmitância for muito baixa
            if (transmittance < 0.01)
                break;
        }

        // Avançar para próxima posição
        current_pos += step_vector;
    }

    return accumulated_light;
}

void main()
{
    // Obter cor da cena
    vec4 scene_color = texture(u_scene_texture, v_texcoord);

    // Calcular luz volumétrica
    vec4 volumetric_light = ray_march(v_texcoord);

    // Combinar com a cena
    vec4 final_color = scene_color + volumetric_light;

    // Ajustar exposição para evitar oversaturation
    final_color.rgb = final_color.rgb / (final_color.rgb + vec3(1.0));

    // Preservar alpha original
    final_color.a = scene_color.a;

    frag_color = final_color;
}
#endif
