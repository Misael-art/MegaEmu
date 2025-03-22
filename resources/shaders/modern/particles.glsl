#version 460

// Vertex Shader
#ifdef VERTEX_SHADER
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texcoord;
layout(location = 2) in vec4 a_particle_data;  // x,y = posição, z = tamanho, w = vida
layout(location = 3) in vec4 a_particle_color;

layout(location = 0) out vec2 v_texcoord;
layout(location = 1) out vec4 v_color;
layout(location = 2) out float v_life;

uniform mat4 u_projection;
uniform float u_time;
uniform float u_size_start;
uniform float u_size_end;
uniform vec4 u_color_start;
uniform vec4 u_color_end;

void main()
{
    // Calcular posição da partícula
    vec2 particle_pos = a_particle_data.xy;
    float life = a_particle_data.w;

    // Interpolar tamanho
    float size = mix(u_size_start, u_size_end, 1.0 - life);

    // Interpolar cor
    v_color = mix(u_color_end, u_color_start, life);

    // Passar vida para fragment shader
    v_life = life;

    // Calcular posição final
    vec4 world_pos = vec4(particle_pos + a_position.xy * size, 0.0, 1.0);
    gl_Position = u_projection * world_pos;

    // Passar coordenadas de textura
    v_texcoord = a_texcoord;
}
#endif

// Fragment Shader
#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 v_texcoord;
layout(location = 1) in vec4 v_color;
layout(location = 2) in float v_life;

layout(location = 0) out vec4 frag_color;

uniform sampler2D u_scene_texture;    // Cena renderizada
uniform sampler2D u_particle_texture; // Textura da partícula
uniform bool u_emit_light;           // Emitir luz
uniform float u_time;                // Tempo para animação

// Função para calcular forma da partícula
float calculate_particle_shape(vec2 uv)
{
    // Distância do centro
    vec2 center = uv - vec2(0.5);
    float dist = length(center);

    // Forma circular suave
    float radius = 0.5;
    float softness = 0.1;
    return 1.0 - smoothstep(radius - softness, radius, dist);
}

// Função para adicionar variação à partícula
vec4 add_variation(vec4 color, vec2 uv)
{
    // Variação baseada na vida
    float flicker = sin(u_time * 10.0 + v_life * 20.0) * 0.1 + 0.9;

    // Variação baseada na posição
    float edge_glow = 1.0 - length(uv - vec2(0.5));
    edge_glow = pow(edge_glow, 3.0);

    // Combinar efeitos
    color.rgb *= flicker;
    color.rgb += edge_glow * 0.2;

    return color;
}

// Função para calcular iluminação
vec4 calculate_lighting(vec4 color)
{
    if (!u_emit_light)
        return color;

    // Adicionar brilho baseado na vida
    float glow = v_life * v_life;
    vec4 light = color * glow * 2.0;

    // Adicionar variação de cor ao brilho
    light.rgb += vec3(0.2, 0.1, 0.0) * glow;

    return light;
}

void main()
{
    // Calcular forma base da partícula
    float shape = calculate_particle_shape(v_texcoord);

    // Amostrar textura da partícula
    vec4 particle = texture(u_particle_texture, v_texcoord);

    // Combinar com cor da partícula
    vec4 color = particle * v_color * shape;

    // Adicionar variação
    color = add_variation(color, v_texcoord);

    // Calcular iluminação
    vec4 light = calculate_lighting(color);

    // Resultado final
    frag_color = color + light;
}
#endif

// Compute Shader para Simulação de Partículas
#ifdef COMPUTE_SHADER
layout(local_size_x = 256) in;

struct Particle {
    vec2 position;
    vec2 velocity;
    float life;
    float size;
    vec4 color;
};

layout(std430, binding = 0) buffer ParticleBuffer {
    Particle particles[];
};

uniform float u_delta_time;
uniform float u_spawn_rate;
uniform float u_lifetime;
uniform vec2 u_gravity;
uniform float u_velocity;
uniform bool u_collide;
uniform vec2 u_screen_size;
uniform sampler2D u_collision_map;

// Função para gerar número aleatório
float random(vec2 st)
{
    return fract(sin(dot(st, vec2(12.9898, 78.233))) * 43758.5453);
}

// Função para inicializar partícula
void init_particle(uint index)
{
    vec2 seed = vec2(float(index), u_delta_time);

    // Posição inicial aleatória
    particles[index].position = vec2(
        random(seed) * u_screen_size.x,
        random(seed.yx) * u_screen_size.y
    );

    // Velocidade inicial aleatória
    float angle = random(seed * 2.0) * 6.28318;
    particles[index].velocity = vec2(
        cos(angle),
        sin(angle)
    ) * u_velocity;

    // Vida inicial
    particles[index].life = u_lifetime;

    // Tamanho inicial
    particles[index].size = 1.0;

    // Cor inicial
    particles[index].color = vec4(1.0);
}

// Função para atualizar partícula
void update_particle(uint index)
{
    Particle p = particles[index];

    // Atualizar vida
    p.life -= u_delta_time;

    // Reiniciar se morta
    if (p.life <= 0.0)
    {
        init_particle(index);
        return;
    }

    // Atualizar velocidade (gravidade)
    p.velocity += u_gravity * u_delta_time;

    // Atualizar posição
    vec2 new_pos = p.position + p.velocity * u_delta_time;

    // Verificar colisões
    if (u_collide)
    {
        vec2 uv = new_pos / u_screen_size;
        float collision = texture(u_collision_map, uv).r;

        if (collision > 0.5)
        {
            // Reflexão simples
            p.velocity *= -0.5;
            new_pos = p.position;
        }
    }

    // Manter dentro da tela
    new_pos = clamp(new_pos, vec2(0.0), u_screen_size);

    // Atualizar posição
    p.position = new_pos;

    // Salvar partícula atualizada
    particles[index] = p;
}

void main()
{
    uint index = gl_GlobalInvocationID.x;
    if (index >= particles.length())
        return;

    // Atualizar partícula
    update_particle(index);
}
#endif
