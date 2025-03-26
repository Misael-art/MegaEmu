/**
 * @file rp2a03_adapter.h
 * @brief Adaptador para a CPU RP2A03 usar a interface genérica
 */
#ifndef RP2A03_ADAPTER_H
#define RP2A03_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../cpu_interface.h"
#include "platforms/nes/cpu/rp2a03.h"

/**
 * @brief Cria uma interface genérica para a CPU RP2A03
 *
 * @return cpu_interface_t* Interface genérica inicializada
 */
cpu_interface_t *rp2a03_create_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* RP2A03_ADAPTER_H */
