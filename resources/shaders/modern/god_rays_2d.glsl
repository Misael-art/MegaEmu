#version 450 core

// Shader de Volumetric Lighting (God Rays) otimizado para jogos 2D
// Este shader implementa ray-marching 2D para simular luz volumétrica
// através de obstáculos em sprites 2D

// Vertex Shader
#ifdef VERTEX_SHADER
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texcoord;

layout(location = 0) out vec2 v_texcoord;

void main() {
    gl_Position = vec4(a_position, 1.0);
    v_texcoord = a_texcoord;
}
#endif

// Fragment Shader
#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 v_texcoord;
layout(location = 0) out vec4 frag_color;

// Texturas
layout(binding = 0) uniform sampler2D u_scene_texture;  // Textura da cena renderizada
layout(binding = 1) uniform sampler2D u_mask_texture;   // Máscara de obstáculos (objetos opacos)
layout(binding = 2) uniform sampler2D u_noise_texture;  // Textura de ruído para variação

// Parâmetros do efeito
layout(std140, binding = 0) uniform VolumetricParams {
    vec2 u_light_pos;           // Posição da fonte de luz (normalizada 0-1)
    vec4 u_light_color;         // Cor da luz (RGBA)
    float u_density;            // Densidade da luz
    float u_weight;             // Peso do efeito
    float u_decay;              // Decaimento da luz com a distância
    float u_exposure;           // Exposição da luz
    int u_num_samples;          // Número de amostras para ray-marching
    float u_scatter;            // Dispersão da luz
    float u_noise_scale;        // Escala do ruído
    float u_time;               // Tempo para animação
    vec2 u_resolution;          // Resolução da tela
} params;

// Função para aplicar ruído à amostra
float get_noise(vec2 uv) {
    vec2 noise_uv = uv * params.u_noise_scale + vec2(params.u_time * 0.01, params.u_time * 0.02);
    return texture(u_noise_texture, noise_uv).r * 0.3 + 0.7;
}

// Função principal para cálculo de god rays usando ray-marching
vec4 calculate_god_rays() {
    // Definir vetor do pixel atual para a fonte de luz
    vec2 delta_tex = v_texcoord - params.u_light_pos;

    // Distância até a luz
    float dist = length(delta_tex);

    // Normalizar o vetor de direção
    delta_tex /= dist;

    // Determinar o tamanho do passo para ray-marching
    // Número de amostras é ajustado com base na distância
    float step_size = 1.0 / float(params.u_num_samples);

    // Ajustar tamanho do passo para melhor performance em hardware mais lento
    step_size = min(step_size, 0.1);

    // Calcular vetor de passo para cada amostra
    vec2 step_vec = -delta_tex * step_size;

    // Posição inicial para ray-marching (pixel atual)
    vec2 current_pos = v_texcoord;

    // Acumulador para god rays
    vec4 accumulated_light = vec4(0.0);

    // Valor de decaimento inicial
    float illumination_decay = 1.0;

    // Executar ray-marching do pixel atual até a fonte de luz
    for (int i = 0; i < params.u_num_samples; i++) {
        // Avançar para próxima posição
        current_pos += step_vec;

        // Verificar se está dentro dos limites da textura
        if (current_pos.x < 0.0 || current_pos.x > 1.0 ||
            current_pos.y < 0.0 || current_pos.y > 1.0)
            break;

        // Ler valor da máscara de obstáculos na posição atual
        // 0 = objeto opaco que bloqueia a luz, 1 = transparente
        float mask_value = texture(u_mask_texture, current_pos).r;

        // Se não for um obstáculo, adicionar contribuição à luz
        if (mask_value > 0.1) {
            // Aplicar ruído à amostra para quebrar uniformidade
            float noise = get_noise(current_pos);

            // Acumular luz com decaimento e ruído
            vec4 sample_contrib = texture(u_scene_texture, current_pos) * params.u_light_color *
                                 illumination_decay * step_size * noise * mask_value;

            accumulated_light += sample_contrib;
        }

        // Aplicar decaimento para cada passo (luz diminui com a distância)
        illumination_decay *= params.u_decay;

        // Otimização: parar se o decaimento for muito pequeno
        if (illumination_decay < 0.01)
            break;
    }

    // Aplicar densidade e exposição finais
    accumulated_light *= params.u_density * params.u_exposure;

    return accumulated_light;
}

// Função para calcular reflexo da luz em superfícies
vec4 calculate_specular_highlight(vec4 base_color) {
    // Ler textura da cena
    vec4 scene_color = texture(u_scene_texture, v_texcoord);

    // Verificar se o pixel atual é "brilhante" (potencial reflexo)
    float brightness = dot(scene_color.rgb, vec3(0.299, 0.587, 0.114));

    // Se for um pixel brilhante, adicionar um brilho suave ao redor
    if (brightness > 0.7) {
        // Calcular distância até a fonte de luz
        float dist_to_light = distance(v_texcoord, params.u_light_pos);

        // Adicionar um brilho que diminui com a distância da luz
        float glow = 0.15 * max(0.0, 1.0 - dist_to_light * 1.5);

        // Combinar com a cor da luz
        return base_color + glow * params.u_light_color;
    }

    return base_color;
}

void main() {
    // Obter cor original da cena
    vec4 scene_color = texture(u_scene_texture, v_texcoord);

    // Calcular efeito de god rays
    vec4 god_rays = calculate_god_rays();

    // Combinar cena original com god rays
    vec4 combined = scene_color + god_rays * params.u_weight;

    // Adicionar highlights especulares
    combined = calculate_specular_highlight(combined);

    // Garantir que não ultrapasse o valor máximo
    combined = min(combined, vec4(1.0));

    // Ajustar gama para evitar oversaturation
    combined.rgb = pow(combined.rgb, vec3(0.9));

    frag_color = combined;
}
#endif
