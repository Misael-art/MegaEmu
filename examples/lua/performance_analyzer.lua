-- Analisador de Performance
-- Este script analisa e otimiza a performance do emulador

-- Configurações
local WINDOW_WIDTH = 1024
local WINDOW_HEIGHT = 768
local SAMPLE_FRAMES = 60

-- Cria a janela principal
local window_id = create_window("Analisador de Performance", 150, 150, WINDOW_WIDTH, WINDOW_HEIGHT)

-- Estado
local performance_data = {
    cpu_usage = {},
    frame_times = {},
    audio_buffer = {},
    vram_usage = {}
}

local current_profile = {
    start_time = 0,
    samples = 0,
    component = ""
}

local is_profiling = false
local show_graphs = true

-- Funções de profiling
function start_profile(component)
    current_profile.start_time = get_cycles()
    current_profile.component = component
    current_profile.samples = 0
end

function end_profile()
    local end_time = get_cycles()
    local duration = end_time - current_profile.start_time

    if current_profile.component == "cpu" then
        table.insert(performance_data.cpu_usage, duration)
    elseif current_profile.component == "frame" then
        table.insert(performance_data.frame_times, duration)
    end

    current_profile.samples = current_profile.samples + 1
end

-- Coleta de dados
function collect_performance_data()
    -- CPU
    start_profile("cpu")
    execute_frame()
    end_profile()

    -- Frame time
    start_profile("frame")
    render_frame()
    end_profile()

    -- Áudio
    local buffer_usage = get_audio_buffer_usage()
    table.insert(performance_data.audio_buffer, buffer_usage)

    -- VRAM
    local vram_usage = get_vram_usage()
    table.insert(performance_data.vram_usage, vram_usage)

    -- Mantém apenas os últimos N samples
    if #performance_data.cpu_usage > SAMPLE_FRAMES then
        table.remove(performance_data.cpu_usage, 1)
        table.remove(performance_data.frame_times, 1)
        table.remove(performance_data.audio_buffer, 1)
        table.remove(performance_data.vram_usage, 1)
    end
end

-- Análise de dados
function analyze_performance()
    local results = {
        cpu = {
            avg = 0,
            max = 0,
            min = math.huge
        },
        frame = {
            avg = 0,
            max = 0,
            min = math.huge
        },
        audio = {
            avg = 0,
            underruns = 0
        },
        vram = {
            avg = 0,
            peak = 0
        }
    }

    -- CPU
    for _, usage in ipairs(performance_data.cpu_usage) do
        results.cpu.avg = results.cpu.avg + usage
        results.cpu.max = math.max(results.cpu.max, usage)
        results.cpu.min = math.min(results.cpu.min, usage)
    end
    results.cpu.avg = results.cpu.avg / #performance_data.cpu_usage

    -- Frame times
    for _, time in ipairs(performance_data.frame_times) do
        results.frame.avg = results.frame.avg + time
        results.frame.max = math.max(results.frame.max, time)
        results.frame.min = math.min(results.frame.min, time)
    end
    results.frame.avg = results.frame.avg / #performance_data.frame_times

    -- Áudio
    for _, usage in ipairs(performance_data.audio_buffer) do
        results.audio.avg = results.audio.avg + usage
        if usage < 0.25 then -- buffer menor que 25%
            results.audio.underruns = results.audio.underruns + 1
        end
    end
    results.audio.avg = results.audio.avg / #performance_data.audio_buffer

    -- VRAM
    for _, usage in ipairs(performance_data.vram_usage) do
        results.vram.avg = results.vram.avg + usage
        results.vram.peak = math.max(results.vram.peak, usage)
    end
    results.vram.avg = results.vram.avg / #performance_data.vram_usage

    return results
end

-- Otimizações
function suggest_optimizations(results)
    local suggestions = {}

    -- CPU
    if results.cpu.avg > 16667 then -- mais de 16.67ms por frame (60fps)
        table.insert(suggestions, "CPU: Considere habilitar recompilação dinâmica")
    end

    -- Frame time
    if results.frame.max > 20000 then -- mais de 20ms
        table.insert(suggestions, "Vídeo: Considere habilitar GPU acceleration")
    end

    -- Áudio
    if results.audio.underruns > 0 then
        table.insert(suggestions, "Áudio: Aumente o tamanho do buffer de áudio")
    end

    -- VRAM
    if results.vram.peak > 0.9 then -- mais de 90% usado
        table.insert(suggestions, "VRAM: Considere habilitar texture streaming")
    end

    return suggestions
end

-- Interface do usuário
function draw_graph(data, x, y, width, height, max_value, color)
    if not show_graphs then return end

    -- Normaliza dados
    local points = {}
    for i, value in ipairs(data) do
        local px = x + (i-1) * (width / #data)
        local py = y + height - (value / max_value) * height
        table.insert(points, {px, py})
    end

    -- Desenha gráfico
    draw_lines(window_id, points, color)
end

function update_view()
    clear_window(window_id)

    if is_profiling then
        add_text(window_id, "Coletando dados de performance...")
        return
    end

    local results = analyze_performance()

    -- Estatísticas
    add_text(window_id, "Estatísticas de Performance:")
    add_text(window_id, string.format("CPU: %.2fms (min: %.2f, max: %.2f)",
        results.cpu.avg / 1000,
        results.cpu.min / 1000,
        results.cpu.max / 1000))
    add_text(window_id, string.format("Frame: %.2fms (min: %.2f, max: %.2f)",
        results.frame.avg / 1000,
        results.frame.min / 1000,
        results.frame.max / 1000))
    add_text(window_id, string.format("Áudio: %.1f%% buffer (underruns: %d)",
        results.audio.avg * 100,
        results.audio.underruns))
    add_text(window_id, string.format("VRAM: %.1f%% usado (pico: %.1f%%)",
        results.vram.avg * 100,
        results.vram.peak * 100))

    -- Gráficos
    if show_graphs then
        draw_graph(performance_data.cpu_usage,
            10, 150, 400, 100,
            20000, -- 20ms max
            {1, 0, 0, 1}) -- vermelho

        draw_graph(performance_data.frame_times,
            10, 300, 400, 100,
            20000,
            {0, 1, 0, 1}) -- verde

        draw_graph(performance_data.audio_buffer,
            10, 450, 400, 100,
            1.0,
            {0, 0, 1, 1}) -- azul
    end

    -- Sugestões
    add_text(window_id, "\nSugestões de Otimização:")
    for _, suggestion in ipairs(suggest_optimizations(results)) do
        add_text(window_id, "- " .. suggestion)
    end
end

-- Botões
add_button(window_id, "Iniciar Profiling", function()
    is_profiling = true
    performance_data = {
        cpu_usage = {},
        frame_times = {},
        audio_buffer = {},
        vram_usage = {}
    }
    update_view()

    -- Coleta dados por 60 frames
    for i = 1, SAMPLE_FRAMES do
        collect_performance_data()
    end

    is_profiling = false
    update_view()
end)

add_button(window_id, "Mostrar/Ocultar Gráficos", function()
    show_graphs = not show_graphs
    update_view()
end)

add_button(window_id, "Exportar Relatório", function()
    local filename = input_dialog("Nome do arquivo:", "performance_report.txt")
    if filename then
        local file = io.open(filename, "w")
        if file then
            local results = analyze_performance()

            file:write("Relatório de Performance\n")
            file:write("=====================\n\n")

            file:write(string.format("CPU:\n  Média: %.2fms\n  Min: %.2fms\n  Max: %.2fms\n\n",
                results.cpu.avg / 1000,
                results.cpu.min / 1000,
                results.cpu.max / 1000))

            file:write(string.format("Frame:\n  Média: %.2fms\n  Min: %.2fms\n  Max: %.2fms\n\n",
                results.frame.avg / 1000,
                results.frame.min / 1000,
                results.frame.max / 1000))

            file:write(string.format("Áudio:\n  Buffer médio: %.1f%%\n  Underruns: %d\n\n",
                results.audio.avg * 100,
                results.audio.underruns))

            file:write(string.format("VRAM:\n  Uso médio: %.1f%%\n  Pico: %.1f%%\n\n",
                results.vram.avg * 100,
                results.vram.peak * 100))

            file:write("Sugestões de Otimização:\n")
            for _, suggestion in ipairs(suggest_optimizations(results)) do
                file:write("- " .. suggestion .. "\n")
            end

            file:close()
        end
    end
end)

-- Inicialização
update_view()
