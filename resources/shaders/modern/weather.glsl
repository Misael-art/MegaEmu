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
uniform sampler2D u_depth_texture;    // Buffer de profundidade
uniform sampler2D u_noise_texture;    // Ruído para variação
uniform int u_weather_type;          // Tipo de clima
uniform float u_intensity;           // Intensidade do efeito
uniform float u_wind_speed;          // Velocidade do vento
uniform float u_wind_direction;      // Direção do vento (radianos)
uniform float u_accumulation;        // Acúmulo de neve
uniform float u_splash_size;         // Tamanho dos respingos
uniform vec4 u_weather_color;        // Cor do efeito
uniform bool u_use_lighting;         // Aplicar iluminação
uniform vec2 u_screen_size;          // Tamanho da tela
uniform float u_time;                // Tempo para animação

// Constantes
const float PI = 3.14159265359;
const int MAX_DROPS = 100;
const int MAX_FLAKES = 50;

// Função para ruído 2D
float noise2D(vec2 st)
{
    return texture(u_noise_texture, st).r;
}

// Função para gerar gota de chuva
float rain_drop(vec2 uv, vec2 pos, float size)
{
    vec2 delta = uv - pos;
    float drop = 1.0 - smoothstep(0.0, size, length(delta));

    // Alongar gota na direção do vento
    vec2 wind_dir = vec2(cos(u_wind_direction), sin(u_wind_direction));
    float stretch = dot(normalize(delta), wind_dir) * u_wind_speed;
    drop *= 1.0 + stretch;

    return drop;
}

// Função para gerar floco de neve
float snow_flake(vec2 uv, vec2 pos, float size)
{
    vec2 delta = uv - pos;

    // Rotação baseada no tempo
    float angle = u_time * u_wind_speed;
    mat2 rotation = mat2(cos(angle), -sin(angle),
                        sin(angle), cos(angle));
    delta = rotation * delta;

    // Forma de cristal
    float flake = 0.0;
    for (int i = 0; i < 6; i++)
    {
        float angle = float(i) * PI / 3.0;
        vec2 arm = vec2(cos(angle), sin(angle)) * size;
        flake += 1.0 - smoothstep(0.0, 0.1 * size, length(delta - arm));
    }

    return clamp(flake, 0.0, 1.0);
}

// Função para gerar respingo
float splash(vec2 uv, vec2 pos)
{
    vec2 delta = uv - pos;
    float ring = abs(length(delta) - u_splash_size);
    return 1.0 - smoothstep(0.0, 0.02, ring);
}

// Função para simular chuva
vec4 render_rain(vec2 uv)
{
    vec4 rain = vec4(0.0);

    // Direção do vento
    vec2 wind_dir = vec2(cos(u_wind_direction), sin(u_wind_direction));

    // Gerar gotas
    for (int i = 0; i < MAX_DROPS; i++)
    {
        // Posição baseada no ruído e tempo
        float noise = noise2D(vec2(float(i) * 0.1, u_time));
        vec2 pos = vec2(
            mod(noise * 1234.5 + u_time * u_wind_speed * wind_dir.x, 1.0),
            mod(noise * 5678.9 + u_time * (u_wind_speed + 2.0) * wind_dir.y, 1.0)
        );

        // Gerar gota
        float drop = rain_drop(uv, pos, 0.002);
        rain += vec4(u_weather_color.rgb, drop * u_intensity);

        // Adicionar respingo quando a gota atinge uma superfície
        if (pos.y < 0.1)
        {
            float splash_effect = splash(uv, vec2(pos.x, 0.1));
            rain += vec4(u_weather_color.rgb * 0.5, splash_effect * u_intensity * 0.5);
        }
    }

    return rain;
}

// Função para simular neve
vec4 render_snow(vec2 uv)
{
    vec4 snow = vec4(0.0);

    // Direção do vento
    vec2 wind_dir = vec2(cos(u_wind_direction), sin(u_wind_direction));

    // Gerar flocos
    for (int i = 0; i < MAX_FLAKES; i++)
    {
        // Posição baseada no ruído e tempo
        float noise = noise2D(vec2(float(i) * 0.2, u_time));
        vec2 pos = vec2(
            mod(noise * 2345.6 + u_time * u_wind_speed * wind_dir.x, 1.0),
            mod(noise * 7890.1 + u_time * (u_wind_speed * 0.5) * wind_dir.y, 1.0)
        );

        // Movimento sinusoidal
        pos.x += sin(u_time * 2.0 + float(i)) * 0.02;

        // Gerar floco
        float flake = snow_flake(uv, pos, 0.005);
        snow += vec4(u_weather_color.rgb, flake * u_intensity);
    }

    // Adicionar acúmulo de neve em superfícies
    float depth = texture(u_depth_texture, uv).r;
    if (depth > 0.9) // Superfície próxima
    {
        float accumulation = noise2D(uv * 10.0) * u_accumulation;
        snow += vec4(1.0) * accumulation;
    }

    return snow;
}

// Função para simular neblina
vec4 render_fog(vec2 uv)
{
    // Ruído para variação da neblina
    vec2 fog_uv = uv * 2.0 + vec2(u_time * u_wind_speed * 0.1);
    float noise = noise2D(fog_uv);

    // Densidade baseada na profundidade
    float depth = texture(u_depth_texture, uv).r;
    float density = mix(0.0, u_intensity, depth);

    // Adicionar movimento baseado no vento
    density *= 1.0 + sin(uv.x * 10.0 + u_time) * 0.1;

    return vec4(u_weather_color.rgb, density * noise);
}

void main()
{
    // Obter cor da cena
    vec4 scene_color = texture(u_scene_texture, v_texcoord);

    // Renderizar efeito climático apropriado
    vec4 weather_effect = vec4(0.0);

    switch (u_weather_type)
    {
        case 1: // Chuva
            weather_effect = render_rain(v_texcoord);
            break;

        case 2: // Neve
            weather_effect = render_snow(v_texcoord);
            break;

        case 3: // Tempestade (chuva + efeitos extras)
            weather_effect = render_rain(v_texcoord) * 1.5;
            // Adicionar flash de relâmpago ocasional
            float lightning = step(0.99, sin(u_time * 10.0));
            weather_effect += vec4(1.0) * lightning * 0.5;
            break;

        case 4: // Neblina
            weather_effect = render_fog(v_texcoord);
            break;

        case 5: // Ondas de calor
            vec2 distortion = vec2(
                sin(v_texcoord.y * 10.0 + u_time) * 0.01,
                cos(v_texcoord.x * 10.0 + u_time) * 0.01
            );
            scene_color = texture(u_scene_texture, v_texcoord + distortion);
            break;
    }

    // Aplicar iluminação se necessário
    if (u_use_lighting && weather_effect.a > 0.0)
    {
        float depth = texture(u_depth_texture, v_texcoord).r;
        weather_effect.rgb *= (1.0 - depth) * 2.0;
    }

    // Combinar com a cena
    vec4 final_color = mix(scene_color, weather_effect, weather_effect.a);

    frag_color = final_color;
}
#endif
