/**
 * @file ppu_2c02_adapter.h
 * @brief Adaptador para a PPU 2C02 usar a interface genérica
 */
#ifndef PPU_2C02_ADAPTER_H
#define PPU_2C02_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../ppu_interface.h"
#include "platforms/nes/ppu/nes_ppu.h"

/**
 * @brief Cria uma interface genérica para a PPU 2C02
 *
 * @return ppu_interface_t* Interface genérica inicializada
 */
ppu_interface_t *ppu_2c02_create_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* PPU_2C02_ADAPTER_H */
