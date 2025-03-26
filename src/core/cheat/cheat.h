/**
 * @file cheat.h
 * @brief Interface principal do sistema de cheats
 */

#ifndef MEGA_EMU_CHEAT_H
#define MEGA_EMU_CHEAT_H

#include <stdint.h>
#include <stdbool.h>
#include "cheat_types.h"
#include "../memory/memory_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa o sistema de cheats
 *
 * @param memory Interface de memória do emulador
 * @param platform Plataforma alvo
 * @return true se inicializado com sucesso, false caso contrário
 */
bool mega_emu_cheat_init(void* memory, mega_emu_cheat_platform_t platform);

/**
 * @brief Finaliza o sistema de cheats e libera recursos
 */
void mega_emu_cheat_shutdown(void);

/**
 * @brief Adiciona um novo cheat à lista
 *
 * @param cheat Estrutura do cheat a ser adicionado
 * @return Índice do cheat na lista ou -1 em caso de erro
 */
int mega_emu_cheat_add(const mega_emu_cheat_t* cheat);

/**
 * @brief Remove um cheat da lista
 *
 * @param index Índice do cheat a ser removido
 * @return true se removido com sucesso, false caso contrário
 */
bool mega_emu_cheat_remove(uint32_t index);

/**
 * @brief Habilita ou desabilita um cheat
 *
 * @param index Índice do cheat
 * @param enabled Estado desejado (true para habilitado, false para desabilitado)
 * @return true se a operação foi bem-sucedida, false caso contrário
 */
bool mega_emu_cheat_enable(uint32_t index, bool enabled);

/**
 * @brief Aplica todos os cheats habilitados em um único frame
 *
 * Esta função deve ser chamada uma vez por frame, após a execução do CPU
 * e antes da renderização
 *
 * @return Número de cheats aplicados
 */
uint32_t mega_emu_cheat_apply_all(void);

/**
 * @brief Carrega uma lista de cheats de um arquivo
 *
 * @param filename Caminho para o arquivo
 * @return true se carregado com sucesso, false caso contrário
 */
bool mega_emu_cheat_load_from_file(const char* filename);

/**
 * @brief Salva a lista de cheats atual em um arquivo
 *
 * @param filename Caminho para o arquivo
 * @return true se salvo com sucesso, false caso contrário
 */
bool mega_emu_cheat_save_to_file(const char* filename);

/**
 * @brief Decodifica um código Game Genie
 *
 * @param code String com o código Game Genie
 * @param platform Plataforma alvo
 * @param result Ponteiro para estrutura que receberá as informações decodificadas
 * @return true se decodificado com sucesso, false caso contrário
 */
bool mega_emu_cheat_decode_game_genie(const char* code, mega_emu_cheat_platform_t platform, mega_emu_cheat_t* result);

/**
 * @brief Decodifica um código Pro Action Replay
 *
 * @param code String com o código Pro Action Replay
 * @param platform Plataforma alvo
 * @param result Ponteiro para estrutura que receberá as informações decodificadas
 * @return true se decodificado com sucesso, false caso contrário
 */
bool mega_emu_cheat_decode_pro_action_replay(const char* code, mega_emu_cheat_platform_t platform, mega_emu_cheat_t* result);

/**
 * @brief Inicializa o Cheat Finder
 *
 * @return Ponteiro para o contexto do Cheat Finder ou NULL em caso de erro
 */
mega_emu_cheat_finder_t* mega_emu_cheat_finder_create(void);

/**
 * @brief Destrói o contexto do Cheat Finder
 *
 * @param finder Ponteiro para o contexto do Cheat Finder
 */
void mega_emu_cheat_finder_destroy(mega_emu_cheat_finder_t* finder);

/**
 * @brief Inicia uma nova busca no Cheat Finder
 *
 * @param finder Ponteiro para o contexto do Cheat Finder
 * @param size Tamanho dos valores a serem buscados
 * @return true se iniciado com sucesso, false caso contrário
 */
bool mega_emu_cheat_finder_init_search(mega_emu_cheat_finder_t* finder, mega_emu_cheat_size_t size);

/**
 * @brief Refina a busca no Cheat Finder
 *
 * @param finder Ponteiro para o contexto do Cheat Finder
 * @param comparator Operador de comparação
 * @param value Valor para comparação (se use_previous_value for false)
 * @param use_previous_value Se deve comparar com o valor da busca anterior
 * @return Número de resultados encontrados
 */
uint32_t mega_emu_cheat_finder_search(mega_emu_cheat_finder_t* finder,
                                    mega_emu_cheat_comparator_t comparator,
                                    uint32_t value,
                                    bool use_previous_value);

/**
 * @brief Obtém um resultado da busca
 *
 * @param finder Ponteiro para o contexto do Cheat Finder
 * @param index Índice do resultado
 * @param result Ponteiro para estrutura que receberá as informações do resultado
 * @return true se obtido com sucesso, false caso contrário
 */
bool mega_emu_cheat_finder_get_result(const mega_emu_cheat_finder_t* finder,
                                    uint32_t index,
                                    mega_emu_cheat_search_result_t* result);

/**
 * @brief Cria um cheat a partir de um resultado do Cheat Finder
 *
 * @param finder Ponteiro para o contexto do Cheat Finder
 * @param index Índice do resultado
 * @param name Nome para o cheat
 * @param description Descrição para o cheat
 * @param value Valor para o cheat (se NULL, usa o valor atual)
 * @return Ponteiro para o novo cheat ou NULL em caso de erro
 */
mega_emu_cheat_t* mega_emu_cheat_finder_create_cheat(const mega_emu_cheat_finder_t* finder,
                                                  uint32_t index,
                                                  const char* name,
                                                  const char* description,
                                                  uint32_t* value);

#ifdef __cplusplus
}
#endif

#endif /* MEGA_EMU_CHEAT_H */
