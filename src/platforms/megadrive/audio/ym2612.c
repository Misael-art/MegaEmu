/**
 * @file ym2612.c
 * @brief Implementação da emulação do chip de som YM2612 (OPN2) do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 */

#include "ym2612.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../../../utils/log_utils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constantes
#define YM2612_ENV_ATTACK  0
#define YM2612_ENV_DECAY   1
#define YM2612_ENV_SUSTAIN 2
#define YM2612_ENV_RELEASE 3

// Tabelas pré-calculadas
static uint32_t attack_rate_table[64][8]; // Tabela de rates de attack
static uint32_t decay_rate_table[64][8];  // Tabela de rates de decay
static int32_t sin_table[1024];           // Tabela de senos
static uint32_t pow_table[2048];          // Tabela de potência para envelopes
static bool tables_initialized = false;

/**
 * @brief Inicializa tabelas pré-calculadas
 */
static void init_tables(void)
{
    if (tables_initialized)
    {
        return;
    }
    
    LOG_INFO("Inicializando tabelas para YM2612");
    
    // Inicializar tabela de seno
    for (int i = 0; i < 1024; i++)
    {
        // Seno com 13-bit de precisão (-4096 a 4095)
        double sine = sin(((double)i * 2.0 * M_PI) / 1024.0);
        sin_table[i] = (int32_t)(sine * 4096.0);
    }
    
    // Inicializar tabela de potência (envelope logarítmico)
    for (int i = 0; i < 2048; i++)
    {
        double db = (double)i * (48.0 / 2048.0); // 0 a 48 dB
        pow_table[i] = (uint32_t)(pow(10.0, db / 20.0) * 1024.0);
    }
    
    // Tabelas de rate para envelopes
    for (int rate = 0; rate < 64; rate++)
    {
        for (int key_scaling = 0; key_scaling < 8; key_scaling++)
        {
            int effective_rate = rate;
            
            // Ajustar rate com base no key scaling
            if (key_scaling > 0)
            {
                effective_rate += key_scaling;
                if (effective_rate > 63)
                {
                    effective_rate = 63;
                }
            }
            
            // Calcular valores para attack e decay
            if (effective_rate < 4)
            {
                attack_rate_table[rate][key_scaling] = 0;
                decay_rate_table[rate][key_scaling] = 0;
            }
            else
            {
                // Valores aproximados - na emulação real seriam mais complexos
                attack_rate_table[rate][key_scaling] = (1 << (effective_rate - 4)) / 8;
                decay_rate_table[rate][key_scaling] = (1 << (effective_rate - 4)) / 16;
            }
        }
    }
    
    tables_initialized = true;
    LOG_INFO("Tabelas para YM2612 inicializadas com sucesso");
}

/**
 * @brief Inicializa um operador
 * @param op Ponteiro para o operador
 */
static void init_operator(ym2612_operator_t *op)
{
    if (!op) return;
    
    op->dt = 0;
    op->mul = 0;
    op->tl = 0;
    op->ks = 0;
    op->ar = 0;
    op->am = 0;
    op->dr = 0;
    op->sr = 0;
    op->sl = 0;
    op->rr = 0;
    op->ssg_eg = 0;
    
    op->state = YM2612_ENV_RELEASE;
    op->env_level = 0x3FF; // Máximo (silêncio)
    op->output = 0;
}

/**
 * @brief Inicializa um canal
 * @param channel Ponteiro para o canal
 */
static void init_channel(ym2612_channel_t *channel)
{
    if (!channel) return;
    
    for (int i = 0; i < YM2612_NUM_OPERATORS; i++)
    {
        init_operator(&channel->operators[i]);
    }
    
    channel->freq_num = 0;
    channel->block = 0;
    channel->feedback = 0;
    channel->algorithm = 0;
    channel->ams = 0;
    channel->pms = 0;
    channel->key_on = false;
    
    channel->output[0] = 0;
    channel->output[1] = 0;
}

/**
 * @brief Inicializa o chip YM2612
 */
emu_error_t ym2612_init(ym2612_t *chip, uint32_t clock, uint32_t rate)
{
    if (!chip)
    {
        LOG_ERROR("YM2612: Ponteiro nulo passado para inicialização");
        return EMU_ERROR_INVALID_PARAM;
    }
    
    // Inicializar tabelas se necessário
    init_tables();
    
    // Limpar estrutura
    memset(chip, 0, sizeof(ym2612_t));
    
    // Inicializar canais
    for (int i = 0; i < YM2612_NUM_CHANNELS; i++)
    {
        init_channel(&chip->channels[i]);
    }
    
    // Configurar clock e taxa de amostragem
    chip->clock = clock;
    chip->rate = rate;
    chip->clock_ratio = (float)clock / (float)rate;
    
    LOG_INFO("YM2612 inicializado: clock=%u Hz, sample_rate=%u Hz", clock, rate);
    
    return EMU_ERROR_NONE;
}

/**
 * @brief Reseta o estado do chip YM2612
 */
emu_error_t ym2612_reset(ym2612_t *chip)
{
    if (!chip)
    {
        LOG_ERROR("YM2612: Ponteiro nulo passado para reset");
        return EMU_ERROR_INVALID_PARAM;
    }
    
    // Salvar clock e taxa de amostragem
    uint32_t clock = chip->clock;
    uint32_t rate = chip->rate;
    float clock_ratio = chip->clock_ratio;
    
    // Limpar registradores
    memset(chip->registers, 0, sizeof(chip->registers));
    
    // Reinicializar canais
    for (int i = 0; i < YM2612_NUM_CHANNELS; i++)
    {
        init_channel(&chip->channels[i]);
    }
    
    // Restaurar clock e taxa de amostragem
    chip->clock = clock;
    chip->rate = rate;
    chip->clock_ratio = clock_ratio;
    
    // Resetar contadores
    chip->cycles = 0;
    chip->samples_generated = 0;
    
    // Resetar timers e LFO
    chip->lfo_enable = 0;
    chip->lfo_freq = 0;
    chip->timer_a_val = 0;
    chip->timer_b_val = 0;
    chip->timer_a_enable = false;
    chip->timer_b_enable = false;
    
    LOG_INFO("YM2612 resetado");
    
    return EMU_ERROR_NONE;
}

/**
 * @brief Libera recursos utilizados pelo chip YM2612
 */
void ym2612_shutdown(ym2612_t *chip)
{
    // Não há recursos alocados dinamicamente para liberar
    if (chip)
    {
        LOG_INFO("YM2612 desligado");
    }
}

/**
 * @brief Escreve um valor em um registrador
 */
void ym2612_write(ym2612_t *chip, uint8_t port, uint8_t reg, uint8_t value)
{
    if (!chip)
    {
        LOG_ERROR("YM2612: Ponteiro nulo passado para escrita");
        return;
    }
    
    // Calcular endereço do registrador
    uint16_t addr = (port << 8) | reg;
    
    // Verificar se o endereço é válido
    if (addr >= YM2612_NUM_REGISTERS)
    {
        LOG_WARNING("YM2612: Tentativa de escrita em registrador inválido: port=%u, reg=0x%02X", port, reg);
        return;
    }
    
    // Armazenar valor no registrador
    chip->registers[addr] = value;
    
    LOG_DEBUG("YM2612: Escrita em registrador: port=%u, reg=0x%02X, valor=0x%02X", port, reg, value);
    
    // Processar registrador específico
    // Nota: Esta é uma implementação simplificada. Uma implementação completa
    // processaria todos os registradores do YM2612.
    
    // Registradores globais
    if (port == 0)
    {
        switch (reg)
        {
            case 0x22: // LFO
                chip->lfo_enable = (value >> 3) & 0x01;
                chip->lfo_freq = value & 0x07;
                LOG_DEBUG("YM2612: LFO configurado: enable=%u, freq=%u", chip->lfo_enable, chip->lfo_freq);
                break;
                
            case 0x24: // Timer A (MSB)
                chip->timer_a_val = (chip->timer_a_val & 0x03) | ((value & 0xFF) << 2);
                LOG_DEBUG("YM2612: Timer A MSB configurado: valor=%u", chip->timer_a_val);
                break;
                
            case 0x25: // Timer A (LSB)
                chip->timer_a_val = (chip->timer_a_val & 0x3FC) | (value & 0x03);
                LOG_DEBUG("YM2612: Timer A LSB configurado: valor=%u", chip->timer_a_val);
                break;
                
            case 0x26: // Timer B
                chip->timer_b_val = value & 0xFF;
                LOG_DEBUG("YM2612: Timer B configurado: valor=%u", chip->timer_b_val);
                break;
                
            case 0x27: // Timer Control
                chip->timer_a_enable = (value & 0x01) != 0;
                chip->timer_b_enable = (value & 0x02) != 0;
                LOG_DEBUG("YM2612: Controle de timer: A=%u, B=%u", 
                         chip->timer_a_enable, chip->timer_b_enable);
                break;
                
            case 0x28: // Key On/Off
                {
                    uint8_t channel = value & 0x07;
                    bool key_on = (value & 0xF0) != 0;
                    
                    // Ajustar índice do canal
                    if (channel >= 4)
                    {
                        channel = channel - 4 + 3; // Canais 4-6 -> índices 3-5
                    }
                    
                    if (channel < YM2612_NUM_CHANNELS)
                    {
                        chip->channels[channel].key_on = key_on;
                        LOG_DEBUG("YM2612: Key %s para canal %u", 
                                 key_on ? "ON" : "OFF", channel);
                    }
                }
                break;
        }
    }
    
    // Processar registradores de canal/operador
    // Esta é uma implementação simplificada que não processa todos os registradores
    // Uma implementação completa processaria todos os parâmetros de operador e canal
}

/**
 * @brief Lê um valor de um registrador
 */
uint8_t ym2612_read(ym2612_t *chip, uint8_t port, uint8_t reg)
{
    if (!chip)
    {
        LOG_ERROR("YM2612: Ponteiro nulo passado para leitura");
        return 0;
    }
    
    // Calcular endereço do registrador
    uint16_t addr = (port << 8) | reg;
    
    // Verificar se o endereço é válido
    if (addr >= YM2612_NUM_REGISTERS)
    {
        LOG_WARNING("YM2612: Tentativa de leitura de registrador inválido: port=%u, reg=0x%02X", port, reg);
        return 0;
    }
    
    // Retornar valor do registrador
    return chip->registers[addr];
}

/**
 * @brief Define a taxa de amostragem
 */
void ym2612_set_sample_rate(ym2612_t *chip, uint32_t rate)
{
    if (!chip || rate == 0)
    {
        LOG_ERROR("YM2612: Parâmetros inválidos para definição de taxa de amostragem");
        return;
    }
    
    chip->rate = rate;
    chip->clock_ratio = (float)chip->clock / (float)rate;
    
    LOG_INFO("YM2612: Taxa de amostragem alterada para %u Hz", rate);
}

/**
 * @brief Define a frequência do clock
 */
void ym2612_set_clock(ym2612_t *chip, uint32_t clock)
{
    if (!chip || clock == 0)
    {
        LOG_ERROR("YM2612: Parâmetros inválidos para definição de clock");
        return;
    }
    
    chip->clock = clock;
    chip->clock_ratio = (float)clock / (float)chip->rate;
    
    LOG_INFO("YM2612: Clock alterado para %u Hz", clock);
}

/**
 * @brief Avança o YM2612 pelo número especificado de ciclos
 */
void ym2612_advance(ym2612_t *chip, uint32_t cycles)
{
    if (!chip)
    {
        return;
    }
    
    chip->cycles += cycles;
    
    // Calcular quantas amostras deveriam ter sido geradas
    uint32_t expected_samples = (uint32_t)((float)chip->cycles / chip->clock_ratio);
    
    // Atualizar contador de ciclos
    if (expected_samples > chip->samples_generated)
    {
        chip->samples_generated = expected_samples;
    }
}

/**
 * @brief Calcula a saída de um operador
 * @param op Ponteiro para o operador
 * @param input Valor de entrada
 * @param freq_num Número de frequência
 * @param block Bloco (oitava)
 * @return Valor de saída do operador
 */
static int32_t calculate_operator_output(ym2612_operator_t *op, int32_t input, uint16_t freq_num, uint8_t block)
{
    // Implementação simplificada - uma implementação completa seria mais complexa
    
    // Calcular fase baseada na frequência
    uint32_t phase = (freq_num << block) * op->mul;
    
    // Aplicar detune
    if (op->dt != 0)
    {
        // Simplificação - detune real seria mais complexo
        phase += op->dt * 4;
    }
    
    // Calcular índice na tabela de seno (simplificado)
    uint32_t sin_idx = (phase >> 2) & 0x3FF;
    
    // Obter valor da tabela de seno
    int32_t sin_val = sin_table[sin_idx];
    
    // Aplicar feedback se necessário
    if (input != 0)
    {
        sin_val = (sin_val + input) >> 1;
    }
    
    // Aplicar envelope
    uint32_t env = op->env_level;
    
    // Calcular atenuação total (simplificado)
    uint32_t att = env + (op->tl << 3);
    if (att > 0x3FF)
    {
        att = 0x3FF;
    }
    
    // Aplicar atenuação
    int32_t output = sin_val;
    if (att > 0)
    {
        // Simplificação - a implementação real usaria tabelas de atenuação
        output = (output * (0x3FF - att)) >> 10;
    }
    
    return output;
}

/**
 * @brief Atualiza o envelope de um operador
 * @param op Ponteiro para o operador
 * @param key_on Estado da tecla (on/off)
 */
static void update_envelope(ym2612_operator_t *op, bool key_on)
{
    // Implementação simplificada do envelope ADSR
    
    // Verificar mudança de estado key on/off
    if (key_on && op->state == YM2612_ENV_RELEASE)
    {
        // Key on - iniciar attack
        op->state = YM2612_ENV_ATTACK;
        op->env_level = 0x3FF; // Começar do nível máximo (silêncio)
    }
    else if (!key_on && op->state != YM2612_ENV_RELEASE)
    {
        // Key off - ir para release
        op->state = YM2612_ENV_RELEASE;
    }
    
    // Processar envelope baseado no estado atual
    switch (op->state)
    {
        case YM2612_ENV_ATTACK:
            {
                // Simplificação - attack rate real seria baseado em tabelas
                uint32_t rate = attack_rate_table[op->ar][0];
                if (rate > 0)
                {
                    // Decremento exponencial (simplificado)
                    op->env_level -= ((0x3FF - op->env_level) * rate) >> 8;
                    
                    // Verificar se chegou ao mínimo
                    if (op->env_level <= 0)
                    {
                        op->env_level = 0;
                        op->state = YM2612_ENV_DECAY;
                    }
                }
            }
            break;
            
        case YM2612_ENV_DECAY:
            {
                // Simplificação - decay rate real seria baseado em tabelas
                uint32_t rate = decay_rate_table[op->dr][0];
                if (rate > 0)
                {
                    // Incremento linear (simplificado)
                    op->env_level += rate;
                    
                    // Verificar se atingiu o nível de sustain
                    uint32_t sl = op->sl << 5; // Converter para escala 0-1023
                    if (op->env_level >= sl)
                    {
                        op->env_level = sl;
                        op->state = YM2612_ENV_SUSTAIN;
                    }
                }
            }
            break;
            
        case YM2612_ENV_SUSTAIN:
            {
                // Simplificação - sustain rate real seria baseado em tabelas
                uint32_t rate = decay_rate_table[op->sr][0];
                if (rate > 0)
                {
                    // Incremento linear (simplificado)
                    op->env_level += rate;
                    
                    // Limitar ao máximo
                    if (op->env_level >= 0x3FF)
                    {
                        op->env_level = 0x3FF;
                    }
                }
            }
            break;
            
        case YM2612_ENV_RELEASE:
            {
                // Simplificação - release rate real seria baseado em tabelas
                uint32_t rate = decay_rate_table[op->rr][0];
                if (rate > 0)
                {
                    // Incremento linear (simplificado)
                    op->env_level += rate;
                    
                    // Limitar ao máximo
                    if (op->env_level >= 0x3FF)
                    {
                        op->env_level = 0x3FF;
                    }
                }
            }
            break;
    }
}

/**
 * @brief Atualiza o estado do chip e gera amostras
 */
int32_t ym2612_update(ym2612_t *chip, int16_t *buffer_left, int16_t *buffer_right, int32_t num_samples)
{
    if (!chip || !buffer_left || !buffer_right || num_samples <= 0)
    {
        LOG_ERROR("YM2612: Parâmetros inválidos para atualização");
        return 0;
    }
    
    // Processar cada amostra
    for (int32_t i = 0; i < num_samples; i++)
    {
        // Limpar buffers de saída
        int32_t output_left = 0;
        int32_t output_right = 0;
        
        // Processar cada canal
        for (int ch = 0; ch < YM2612_NUM_CHANNELS; ch++)
        {
            ym2612_channel_t *channel = &chip->channels[ch];
            
            // Atualizar envelopes dos operadores
            for (int op = 0; op < YM2612_NUM_OPERATORS; op++)
            {
                update_envelope(&channel->operators[op], channel->key_on);
            }
            
            // Calcular saída do canal baseado no algoritmo
            int32_t op_outputs[YM2612_NUM_OPERATORS] = {0};
            
            // Calcular saídas dos operadores baseado no algoritmo
            // Esta é uma implementação simplificada - o YM2612 real tem 8 algoritmos diferentes
            switch (channel->algorithm)
            {
                case 0: // Algoritmo 0: OP1->OP2->OP3->OP4->out
                    op_outputs[0] = calculate_operator_output(&channel->operators[0], 0, channel->freq_num, channel->block);
                    op_outputs[1] = calculate_operator_output(&channel->operators[1], op_outputs[0], channel->freq_num, channel->block);
                    op_outputs[2] = calculate_operator_output(&channel->operators[2], op_outputs[1], channel->freq_num, channel->block);
                    op_outputs[3] = calculate_operator_output(&channel->operators[3], op_outputs[2], channel->freq_num, channel->block);
                    
                    // Apenas OP4 vai para a saída
                    channel->output[0] = op_outputs[3];
                    channel->output[1] = op_outputs[3];
                    break;
                    
                case 7: // Algoritmo 7: Todos os operadores vão diretamente para a saída
                    op_outputs[0] = calculate_operator_output(&channel->operators[0], 0, channel->freq_num, channel->block);
                    op_outputs[1] = calculate_operator_output(&channel->operators[1], 0, channel->freq_num, channel->block);
                    op_outputs[2] = calculate_operator_output(&channel->operators[2], 0, channel->freq_num, channel->block);
                    op_outputs[3] = calculate_operator_output(&channel->operators[3], 0, channel->freq_num, channel->block);
                    
                    // Todos os operadores vão para a saída
                    channel->output[0] = op_outputs[0] + op_outputs[1] + op_outputs[2] + op_outputs[3];
                    channel->output[1] = channel->output[0];
                    break;
                    
                default: // Algoritmos 1-6 (simplificados)
                    op_outputs[0] = calculate_operator_output(&channel->operators[0], 0, channel->freq_num, channel->block);
                    op_outputs[1] = calculate_operator_output(&channel->operators[1], op_outputs[0], channel->freq_num, channel->block);
                    op_outputs[2] = calculate_operator_output(&channel->operators[2], 0, channel->freq_num, channel->block);
                    op_outputs[3] = calculate_operator_output(&channel->operators[3], op_outputs[2], channel->freq_num, channel->block);
                    
                    // Saída mista (simplificação)
                    channel->output[0] = op_outputs[1] + op_outputs[3];
                    channel->output[1] = channel->output[0];
                    break;
            }
            
            // Adicionar saída do canal ao buffer de saída
            // Canais 3-5 vão para a direita, 0-2 vão para a esquerda (simplificação)
            if (ch < 3)
            {
                output_left += channel->output[0];
            }
            else
            {
                output_right += channel->output[1];
            }
        }
        
        // Limitar e converter para 16-bit
        output_left = (output_left > 32767) ? 32767 : ((output_left < -32768) ? -32768 : output_left);
        output_right = (output_right > 32767) ? 32767 : ((output_right < -32768) ? -32768 : output_right);
        
        buffer_left[i] = (int16_t)output_left;
        buffer_right[i] = (int16_t)output_right;
    }
    
    // Atualizar contador de amostras
    chip->samples_generated += num_samples;
    
    return num_samples;
}