-- Analisador de ROMs
-- Este script analisa ROMs para identificar padrões comuns e funções

-- Configurações
local WINDOW_WIDTH = 1024
local WINDOW_HEIGHT = 768

-- Cria a janela principal
local window_id = create_window("Analisador de ROMs", 50, 50, WINDOW_WIDTH, WINDOW_HEIGHT)

-- Padrões comuns em jogos
local COMMON_PATTERNS = {
    -- Mega Drive
    {
        name = "VDP Init",
        pattern = {0x13, 0xFC, 0x00, 0xC0, 0x00, 0x00},
        description = "Inicialização do VDP"
    },
    {
        name = "Sound Init",
        pattern = {0x2A, 0x78, 0x00, 0xA0, 0x40, 0x00},
        description = "Inicialização do YM2612"
    },
    -- NES
    {
        name = "NMI Handler",
        pattern = {0x48, 0x8A, 0x48, 0x98, 0x48},
        description = "Handler de NMI padrão"
    },
    -- Master System
    {
        name = "VDP Write",
        pattern = {0x3E, 0xBF, 0xD3, 0xBF},
        description = "Escrita no VDP"
    }
}

-- Estado da análise
local analysis_results = {}
local current_address = 0
local is_analyzing = false

-- Funções auxiliares
function check_pattern(address, pattern)
    for i, byte in ipairs(pattern) do
        if read_memory(address + i - 1) ~= byte then
            return false
        end
    end
    return true
end

function find_patterns(start_addr, end_addr)
    local results = {}

    for addr = start_addr, end_addr do
        for _, pattern in ipairs(COMMON_PATTERNS) do
            if check_pattern(addr, pattern.pattern) then
                table.insert(results, {
                    address = addr,
                    name = pattern.name,
                    description = pattern.description
                })
            end
        end
    end

    return results
end

function analyze_functions()
    local functions = {}
    local current_addr = 0

    while current_addr < get_rom_size() do
        -- Procura por padrões comuns de início de função
        if is_function_start(current_addr) then
            local size = analyze_function_size(current_addr)
            table.insert(functions, {
                address = current_addr,
                size = size,
                name = string.format("func_%04X", current_addr)
            })
            current_addr = current_addr + size
        else
            current_addr = current_addr + 1
        end
    end

    return functions
end

function is_function_start(addr)
    -- Verifica padrões comuns de início de função
    local byte1 = read_memory(addr)
    local byte2 = read_memory(addr + 1)

    -- Push/link no 68000
    if byte1 == 0x4E and byte2 == 0x56 then
        return true
    end

    -- Push no Z80
    if byte1 == 0xC5 or byte1 == 0xF5 then
        return true
    end

    -- Push no 6502
    if byte1 == 0x48 or byte1 == 0x08 then
        return true
    end

    return false
end

function analyze_function_size(start_addr)
    local size = 0
    local max_size = 1024 -- Limite de segurança

    while size < max_size do
        local byte = read_memory(start_addr + size)

        -- Retorno no 68000
        if byte == 0x4E and read_memory(start_addr + size + 1) == 0x75 then
            return size + 2
        end

        -- Retorno no Z80
        if byte == 0xC9 then
            return size + 1
        end

        -- Retorno no 6502
        if byte == 0x60 then
            return size + 1
        end

        size = size + 1
    end

    return size
end

-- Interface do usuário
function update_view()
    clear_window(window_id)

    if is_analyzing then
        add_text(window_id, "Analisando ROM... Por favor aguarde.")
        return
    end

    add_text(window_id, string.format("ROM Size: %d bytes", get_rom_size()))
    add_text(window_id, string.format("Padrões encontrados: %d", #analysis_results))
    add_text(window_id, "")

    for i, result in ipairs(analysis_results) do
        add_text(window_id, string.format("%04X: %s - %s",
            result.address,
            result.name,
            result.description))
    end
end

-- Botões
add_button(window_id, "Analisar Padrões", function()
    is_analyzing = true
    update_view()

    analysis_results = find_patterns(0, get_rom_size() - 1)

    is_analyzing = false
    update_view()
end)

add_button(window_id, "Analisar Funções", function()
    is_analyzing = true
    update_view()

    local functions = analyze_functions()
    for _, func in ipairs(functions) do
        table.insert(analysis_results, {
            address = func.address,
            name = func.name,
            description = string.format("Função (tamanho: %d bytes)", func.size)
        })
    end

    is_analyzing = false
    update_view()
end)

add_button(window_id, "Exportar", function()
    local filename = input_dialog("Nome do arquivo:", "analysis.txt")
    if filename then
        local file = io.open(filename, "w")
        if file then
            for _, result in ipairs(analysis_results) do
                file:write(string.format("%04X: %s - %s\n",
                    result.address,
                    result.name,
                    result.description))
            end
            file:close()
        end
    end
end)

add_button(window_id, "Limpar", function()
    analysis_results = {}
    update_view()
end)

-- Inicialização
update_view()
