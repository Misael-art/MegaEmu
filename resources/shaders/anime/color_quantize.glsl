#version 460

// Vertex Shader
#ifdef VERTEX_SHADER
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texcoord;

layout(location = 0) out vec2 v_texcoord;

void main()
{
    gl_Position = vec4(a_position, 1.0);
    v_texcoord = a_texcoord;
}
#endif

// Fragment Shader
#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 v_texcoord;

layout(location = 0) out vec4 frag_color;

uniform sampler2D u_texture;
uniform int u_color_levels;
uniform float u_saturation;
uniform float u_contrast;
uniform bool u_posterize;

// Função para converter RGB para HSV
vec3 rgb_to_hsv(vec3 rgb)
{
    float cmax = max(max(rgb.r, rgb.g), rgb.b);
    float cmin = min(min(rgb.r, rgb.g), rgb.b);
    float delta = cmax - cmin;

    vec3 hsv = vec3(0.0, 0.0, cmax);

    if (delta != 0.0)
    {
        if (cmax == rgb.r)
            hsv.x = 60.0 * mod((rgb.g - rgb.b) / delta, 6.0);
        else if (cmax == rgb.g)
            hsv.x = 60.0 * ((rgb.b - rgb.r) / delta + 2.0);
        else
            hsv.x = 60.0 * ((rgb.r - rgb.g) / delta + 4.0);

        hsv.y = (cmax == 0.0) ? 0.0 : delta / cmax;
    }

    return hsv;
}

// Função para converter HSV para RGB
vec3 hsv_to_rgb(vec3 hsv)
{
    float h = hsv.x;
    float s = hsv.y;
    float v = hsv.z;

    float c = v * s;
    float x = c * (1.0 - abs(mod(h / 60.0, 2.0) - 1.0));
    float m = v - c;

    vec3 rgb;

    if (h < 60.0)
        rgb = vec3(c, x, 0.0);
    else if (h < 120.0)
        rgb = vec3(x, c, 0.0);
    else if (h < 180.0)
        rgb = vec3(0.0, c, x);
    else if (h < 240.0)
        rgb = vec3(0.0, x, c);
    else if (h < 300.0)
        rgb = vec3(x, 0.0, c);
    else
        rgb = vec3(c, 0.0, x);

    return rgb + vec3(m);
}

// Função para quantizar valor em níveis
float quantize(float value, int levels)
{
    float step = 1.0 / float(levels - 1);
    return floor(value * float(levels - 1) + 0.5) * step;
}

void main()
{
    // Obter cor original
    vec4 color = texture(u_texture, v_texcoord);

    // Converter para HSV
    vec3 hsv = rgb_to_hsv(color.rgb);

    // Ajustar saturação
    hsv.y *= u_saturation;
    hsv.y = clamp(hsv.y, 0.0, 1.0);

    // Ajustar valor (contraste)
    hsv.z = (hsv.z - 0.5) * u_contrast + 0.5;
    hsv.z = clamp(hsv.z, 0.0, 1.0);

    // Quantizar cores se posterização estiver ativa
    if (u_posterize)
    {
        hsv.x = quantize(hsv.x / 360.0, u_color_levels) * 360.0;
        hsv.y = quantize(hsv.y, u_color_levels);
        hsv.z = quantize(hsv.z, u_color_levels);
    }

    // Converter de volta para RGB
    vec3 rgb = hsv_to_rgb(hsv);

    // Resultado final
    frag_color = vec4(rgb, color.a);
}
#endif
