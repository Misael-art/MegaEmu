/** * @file nes_input.h * @brief Definições para o subsistema de entrada (controles) do NES */#ifndef NES_INPUT_H#define NES_INPUT_H#include <stdint.h>#include <stdbool.h>#include "../../../core/interfaces/controller_interface.h"#ifdef __cplusplusextern "C"{#endif/** * @brief Definições dos botões do controle do NES */#define NES_BUTTON_A (1 << CONTROLLER_BUTTON_A)#define NES_BUTTON_B (1 << CONTROLLER_BUTTON_B)#define NES_BUTTON_SELECT (1 << CONTROLLER_BUTTON_SELECT)#define NES_BUTTON_START (1 << CONTROLLER_BUTTON_START)#define NES_BUTTON_UP (1 << CONTROLLER_BUTTON_UP)#define NES_BUTTON_DOWN (1 << CONTROLLER_BUTTON_DOWN)#define NES_BUTTON_LEFT (1 << CONTROLLER_BUTTON_LEFT)#define NES_BUTTON_RIGHT (1 << CONTROLLER_BUTTON_RIGHT)    /**     * @brief Enumeração dos botões do NES para mapeamento de teclas     */    typedef enum    {        NES_BUTTON_IDX_A,        NES_BUTTON_IDX_B,        NES_BUTTON_IDX_SELECT,        NES_BUTTON_IDX_START,        NES_BUTTON_IDX_UP,        NES_BUTTON_IDX_DOWN,        NES_BUTTON_IDX_LEFT,        NES_BUTTON_IDX_RIGHT,        NES_BUTTON_COUNT    } nes_button_t;    /**     * @brief Tipos de dispositivos de entrada     */    typedef enum    {        NES_INPUT_DEVICE_NONE,   /**< Nenhum dispositivo */        NES_INPUT_DEVICE_JOYPAD, /**< Controle padrão */        NES_INPUT_DEVICE_ZAPPER, /**< Zapper (pistola de luz) */    } nes_input_device_type_t;    /**     * @brief Estrutura do subsistema de entrada do NES     */    typedef struct    {        // Estado dos controles        uint8_t button_states[2];   /**< Estado dos botões para os dois controles */        uint8_t strobe;             /**< Estado do strobe */        uint8_t shift_registers[2]; /**< Registradores de deslocamento para leitura serial */        // Tipo de dispositivos conectados        nes_input_device_type_t device_type[2]; /**< Tipo de dispositivo em cada porta */        // Mapeamento de teclas para botões        int32_t key_mapping[2][NES_BUTTON_COUNT]; /**< Mapeamento de teclas para cada botão */        // Estado do Zapper (pistola de luz)        struct        {            int32_t x;              /**< Posição X do cursor */            int32_t y;              /**< Posição Y do cursor */            uint8_t trigger;        /**< Estado do gatilho (1: pressionado, 0: solto) */            uint8_t light_detected; /**< Detecção de luz (1: detectada, 0: não detectada) */        } zapper;    } nes_input_t;    /**     * @brief Inicializa o subsistema de entrada     *     * @return nes_input_t* Ponteiro para o estado do subsistema de entrada, ou NULL em caso de erro     */    bool nes_input_init(nes_input_t *input);    /**     * @brief Finaliza e libera recursos do subsistema de entrada     *     * @param input Ponteiro para o subsistema de entrada     */    void nes_input_shutdown(nes_input_t *input);    /**     * @brief Reseta o estado do subsistema de entrada     *     * @param input Ponteiro para o subsistema de entrada     */    void nes_input_reset(nes_input_t *input);    /**     * @brief Define o tipo de dispositivo conectado em uma porta     *     * @param input Ponteiro para o subsistema de entrada     * @param port Número da porta (0 ou 1)     * @param device_type Tipo de dispositivo     */    void nes_input_set_device(nes_input_t *input, int32_t port, nes_input_device_type_t device_type);    /**     * @brief Define o estado dos botões do controlador     *     * @param input Ponteiro para o subsistema de entrada     * @param port Porta do controlador (0 ou 1)     * @param button_state Estado dos botões (bit 0: A, bit 1: B, etc.)     */    void nes_input_set_buttons(nes_input_t *input, int port, uint8_t buttons);    /**     * @brief Atualiza a posição e estado do Zapper     *     * @param input Ponteiro para o subsistema de entrada     * @param x Coordenada X (0-255)     * @param y Coordenada Y (0-239)     * @param trigger Estado do gatilho (1: pressionado, 0: liberado)     */    void nes_input_set_zapper_state(nes_input_t *input, int32_t x, int32_t y, int32_t trigger);    /**     * @brief Lê o registrador de controle     *     * @param input Ponteiro para o subsistema de entrada     * @param address Endereço (0x4016 ou 0x4017)     * @return uint8_t Valor lido     */    uint8_t nes_input_read(nes_input_t *input, int port);    /**     * @brief Escreve no registrador de controle     *     * @param input Ponteiro para o subsistema de entrada     * @param address Endereço (0x4016)     * @param value Valor a escrever     */    void nes_input_write(nes_input_t *input, uint8_t value);    /**     * @brief Atualiza o estado do Zapper baseado no buffer de frame     *     * @param input Ponteiro para o subsistema de entrada     * @param frame_buffer Buffer de frame (pixels RGBA)     * @param width Largura do frame     * @param height Altura do frame     */    void nes_input_update_zapper(nes_input_t *input, const uint32_t *frame_buffer, int32_t width, int32_t height);    /**     * @brief Detecta se o zapper está apontando para uma área luminosa     * @return 1 se detectou luz, 0 caso contrário     */    int32_t nes_input_update_zapper_light_detection(nes_input_t *input, const uint32_t *frame_buffer, int32_t width, int32_t height);    /**     * @brief Inicializa o mapeamento de teclas padrão     */    void nes_input_init_key_mapping(nes_input_t *input);    /**     * @brief Processa a entrada de teclado e atualiza o estado dos controles     */    void nes_input_process_key(nes_input_t *input, int32_t key_code, int32_t is_pressed);    /**     * @brief Configura o mapeamento personalizado de um botão     * @return 0 se bem-sucedido, -1 em caso de erro     */    int32_t nes_input_set_key_mapping(nes_input_t *input, int32_t player, uint8_t nes_button, int32_t key_code);    /**     * @brief Reseta o mapeamento para o padrão     */    void nes_input_reset_key_mapping(nes_input_t *input, int32_t player);#ifdef __cplusplus}#endif#endif /* NES_INPUT_H */