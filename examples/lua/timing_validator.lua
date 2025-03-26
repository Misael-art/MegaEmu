-- Validador de Timing
-- Este script compara o timing do emulador com hardware real

-- Configurações
local WINDOW_WIDTH = 800
local WINDOW_HEIGHT = 600

-- Cria a janela principal
local window_id = create_window("Validador de Timing", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT)

-- Estado
local test_results = {}
local current_test = nil
local is_testing = false

-- Testes de timing
local TIMING_TESTS = {
    -- Mega Drive
    {
        name = "68000 Instruções Básicas",
        setup = function()
            -- Configura ambiente de teste
            write_memory(0x1000, 0x4E71) -- NOP
            write_memory(0x1002, 0x4E71) -- NOP
            write_memory(0x1004, 0x4E75) -- RTS
        end,
        run = function()
            local cycles = measure_cycles(0x1000, 0x1004)
            return cycles == 8 -- 2 NOPs = 8 ciclos
        end
    },
    {
        name = "VDP H-Blank",
        setup = function()
            -- Espera pelo próximo H-Blank
            while not is_hblank() do end
        end,
        run = function()
            local start = get_cycles()
            while is_hblank() do end
            while not is_hblank() do end
            local end_cycles = get_cycles()
            return (end_cycles - start) == 228 -- ciclos por linha
        end
    },

    -- Master System
    {
        name = "Z80 Instruções Básicas",
        setup = function()
            write_memory(0x1000, 0x00) -- NOP
            write_memory(0x1001, 0x00) -- NOP
            write_memory(0x1002, 0xC9) -- RET
        end,
        run = function()
            local cycles = measure_cycles(0x1000, 0x1002)
            return cycles == 8 -- 2 NOPs = 8 ciclos
        end
    },

    -- NES
    {
        name = "6502 Instruções Básicas",
        setup = function()
            write_memory(0x1000, 0xEA) -- NOP
            write_memory(0x1001, 0xEA) -- NOP
            write_memory(0x1002, 0x60) -- RTS
        end,
        run = function()
            local cycles = measure_cycles(0x1000, 0x1002)
            return cycles == 4 -- 2 NOPs = 4 ciclos
        end
    },
    {
        name = "PPU Timing",
        setup = function()
            -- Espera pelo próximo VBlank
            while not is_vblank() do end
        end,
        run = function()
            local start = get_cycles()
            while is_vblank() do end
            while not is_vblank() do end
            local end_cycles = get_cycles()
            return (end_cycles - start) == 29780 -- ciclos por frame
        end
    }
}

-- Funções auxiliares
function measure_cycles(start_addr, end_addr)
    local start_cycles = get_cycles()

    -- Executa código
    local pc = start_addr
    while pc <= end_addr do
        pc = execute_instruction(pc)
    end

    return get_cycles() - start_cycles
end

function run_test(test)
    current_test = test
    test_results[test.name] = {
        status = "Executando...",
        cycles = 0,
        expected = 0
    }

    -- Configura teste
    test.setup()

    -- Executa teste
    local success = test.run()

    test_results[test.name] = {
        status = success and "OK" or "Falha",
        cycles = get_cycles(),
        expected = test.expected_cycles or 0
    }

    current_test = nil
    update_view()
end

-- Interface do usuário
function update_view()
    clear_window(window_id)

    if is_testing then
        add_text(window_id, "Executando testes... Por favor aguarde.")
        if current_test then
            add_text(window_id, "Teste atual: " .. current_test.name)
        end
        return
    end

    add_text(window_id, "Resultados dos Testes de Timing:")
    add_text(window_id, "")

    for name, result in pairs(test_results) do
        add_text(window_id, string.format("%s: %s", name, result.status))
        if result.cycles > 0 then
            add_text(window_id, string.format("  Ciclos: %d (Esperado: %d)",
                result.cycles, result.expected))
        end
    end
end

-- Botões
add_button(window_id, "Executar Todos", function()
    is_testing = true
    test_results = {}
    update_view()

    for _, test in ipairs(TIMING_TESTS) do
        run_test(test)
    end

    is_testing = false
    update_view()
end)

add_button(window_id, "Exportar Resultados", function()
    local filename = input_dialog("Nome do arquivo:", "timing_results.txt")
    if filename then
        local file = io.open(filename, "w")
        if file then
            file:write("Resultados dos Testes de Timing\n")
            file:write("===========================\n\n")

            for name, result in pairs(test_results) do
                file:write(string.format("%s: %s\n", name, result.status))
                if result.cycles > 0 then
                    file:write(string.format("  Ciclos: %d (Esperado: %d)\n",
                        result.cycles, result.expected))
                end
                file:write("\n")
            end

            file:close()
        end
    end
end)

add_button(window_id, "Limpar", function()
    test_results = {}
    update_view()
end)

-- Inicialização
update_view()
