-- Visualizador de Memória Avançado
-- Este script cria uma janela para visualizar e editar a memória em tempo real

-- Configurações
local WINDOW_WIDTH = 800
local WINDOW_HEIGHT = 600
local BYTES_PER_ROW = 16

-- Cria a janela principal
local window_id = create_window("Visualizador de Memória", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT)

-- Estado
local current_address = 0
local current_bank = 0
local follow_pc = false

-- Funções auxiliares
function format_byte(byte)
    return string.format("%02X", byte)
end

function format_ascii(byte)
    if byte >= 32 and byte <= 126 then
        return string.char(byte)
    end
    return "."
end

-- Desenha uma linha de memória
function draw_memory_line(address)
    local line = string.format("%04X: ", address)
    local ascii = ""

    for i = 0, BYTES_PER_ROW - 1 do
        local byte = read_memory(address + i)
        line = line .. format_byte(byte) .. " "
        ascii = ascii .. format_ascii(byte)
    end

    return line .. "  " .. ascii
end

-- Atualiza a visualização
function update_view()
    -- Limpa a janela
    clear_window(window_id)

    -- Mostra informações do banco atual
    add_text(window_id, string.format("Banco: %02X  PC: %04X", current_bank, get_pc()))

    -- Mostra a memória
    for i = 0, 32 do
        local address = current_address + (i * BYTES_PER_ROW)
        add_text(window_id, draw_memory_line(address))
    end
end

-- Adiciona controles
add_button(window_id, "Anterior", function()
    current_address = math.max(0, current_address - BYTES_PER_ROW * 16)
    update_view()
end)

add_button(window_id, "Próximo", function()
    current_address = current_address + BYTES_PER_ROW * 16
    update_view()
end)

add_button(window_id, "Ir para...", function()
    local addr = input_dialog("Endereço (hex):", "")
    if addr then
        current_address = tonumber(addr, 16) or current_address
        update_view()
    end
end)

add_checkbox(window_id, "Seguir PC", function(checked)
    follow_pc = checked
end)

-- Registra handlers de eventos
register_event("frame", function()
    if follow_pc then
        current_address = get_pc()
        update_view()
    end
end)

register_event("memory_write", function(addr, value)
    if addr >= current_address and addr < current_address + (BYTES_PER_ROW * 32) then
        update_view()
    end
end)

-- Atualização inicial
update_view()
