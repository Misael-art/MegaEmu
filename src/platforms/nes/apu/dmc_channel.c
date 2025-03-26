/**
 * @file dmc_channel.c
 * @brief Implementação do canal DMC (Delta Modulation Channel) do APU do NES
 */

#include "dmc_channel.h"
#include "../../../utils/logging.h"

// Tabela de taxas do DMC (em ciclos de CPU)
static const uint16_t dmc_rate_table[DMC_RATE_TABLE_SIZE] = {
    428, 380, 340, 320, 286, 254, 226, 214,
    190, 160, 142, 128, 106,  84,  72,  54
};

void dmc_init(nes_dmc_channel_t *dmc) {
    if (!dmc) {
        LOG_ERROR("DMC: Ponteiro nulo passado para inicialização");
        return;
    }

    dmc_reset(dmc);
}

void dmc_reset(nes_dmc_channel_t *dmc) {
    if (!dmc) return;

    // Limpar todos os registradores e estado
    dmc->irq_enable = false;
    dmc->loop_flag = false;
    dmc->rate_index = 0;
    dmc->direct_load = 0;
    dmc->sample_addr = 0xC000;  // Endereço inicial padrão
    dmc->sample_length = 0;

    dmc->timer_period = dmc_rate_table[0];
    dmc->timer_counter = dmc->timer_period;

    dmc->current_addr = 0xC000;
    dmc->bytes_remaining = 0;
    dmc->sample_buffer = 0;
    dmc->sample_buffer_empty = true;
    dmc->shift_register = 0;
    dmc->bits_remaining = 8;
    dmc->output_level = 0;

    dmc->dma_pending = false;
    dmc->dma_addr = 0;
    dmc->dma_buffer = 0;

    dmc->enabled = false;
    dmc->irq_flag = false;
    dmc->silence_flag = true;
}

void dmc_write_register(nes_dmc_channel_t *dmc, uint16_t addr, uint8_t value) {
    if (!dmc) return;

    switch (addr & 0x03) {
        case 0x00:  // $4010 - Flags e taxa
            dmc->irq_enable = (value & 0x80) != 0;
            dmc->loop_flag = (value & 0x40) != 0;
            dmc->rate_index = value & 0x0F;
            dmc->timer_period = dmc_rate_table[dmc->rate_index];

            // Limpar flag de IRQ se IRQ foi desabilitado
            if (!dmc->irq_enable) {
                dmc->irq_flag = false;
            }
            break;

        case 0x01:  // $4011 - Load counter
            // Os 7 bits mais baixos são carregados diretamente no nível de saída
            dmc->output_level = value & 0x7F;
            break;

        case 0x02:  // $4012 - Sample address
            dmc->sample_addr = 0xC000 | (value << 6);
            break;

        case 0x03:  // $4013 - Sample length
            dmc->sample_length = (value << 4) | 0x0001;
            break;
    }
}

uint8_t dmc_read_status(nes_dmc_channel_t *dmc) {
    if (!dmc) return 0;

    uint8_t status = 0;

    // Bit 7: IRQ pending
    if (dmc->irq_flag) status |= 0x80;

    // Bit 0: Bytes remaining > 0
    if (dmc->bytes_remaining > 0) status |= 0x01;

    return status;
}

void dmc_clock(nes_dmc_channel_t *dmc) {
    if (!dmc || !dmc->enabled) return;

    // Decrementar timer
    if (dmc->timer_counter > 0) {
        dmc->timer_counter--;
    }

    // Quando timer chega a 0
    if (dmc->timer_counter == 0) {
        // Recarregar timer
        dmc->timer_counter = dmc->timer_period;

        if (!dmc->silence_flag) {
            // Processar bit atual do shift register
            if ((dmc->shift_register & 1) == 1) {
                // Incrementar nível de saída se não estiver no máximo
                if (dmc->output_level <= 125) {
                    dmc->output_level += 2;
                }
            } else {
                // Decrementar nível de saída se não estiver no mínimo
                if (dmc->output_level >= 2) {
                    dmc->output_level -= 2;
                }
            }
            dmc->shift_register >>= 1;
        }

        // Decrementar contador de bits
        if (--dmc->bits_remaining == 0) {
            // Recarregar shift register
            dmc->bits_remaining = 8;
            if (dmc->sample_buffer_empty) {
                dmc->silence_flag = true;
            } else {
                dmc->silence_flag = false;
                dmc->shift_register = dmc->sample_buffer;
                dmc->sample_buffer_empty = true;

                // Solicitar próxima amostra se necessário
                if (dmc->bytes_remaining > 0) {
                    dmc->dma_pending = true;
                    dmc->dma_addr = dmc->current_addr;
                }
            }
        }
    }

    // Verificar se precisa carregar nova amostra
    if (dmc->sample_buffer_empty && dmc->bytes_remaining > 0 && !dmc->dma_pending) {
        dmc->dma_pending = true;
        dmc->dma_addr = dmc->current_addr;
    }
}

int16_t dmc_output(nes_dmc_channel_t *dmc) {
    if (!dmc) return 0;
    return dmc->output_level;
}

bool dmc_irq_pending(nes_dmc_channel_t *dmc) {
    return dmc ? dmc->irq_flag : false;
}

void dmc_acknowledge_irq(nes_dmc_channel_t *dmc) {
    if (dmc) dmc->irq_flag = false;
}

bool dmc_dma_needed(nes_dmc_channel_t *dmc) {
    return dmc ? dmc->dma_pending : false;
}

void dmc_dma_complete(nes_dmc_channel_t *dmc, uint8_t data) {
    if (!dmc) return;

    // Armazenar dado do DMA
    dmc->sample_buffer = data;
    dmc->sample_buffer_empty = false;
    dmc->dma_pending = false;

    // Atualizar endereço e contador
    dmc->current_addr++;
    if (dmc->current_addr > 0xFFFF) {
        dmc->current_addr = 0x8000;
    }

    // Decrementar bytes restantes
    if (dmc->bytes_remaining > 0) {
        dmc->bytes_remaining--;

        // Verificar fim da amostra
        if (dmc->bytes_remaining == 0) {
            if (dmc->loop_flag) {
                // Reiniciar amostra
                dmc->current_addr = dmc->sample_addr;
                dmc->bytes_remaining = dmc->sample_length;
            } else if (dmc->irq_enable) {
                // Gerar IRQ se habilitado
                dmc->irq_flag = true;
            }
        }
    }
}
