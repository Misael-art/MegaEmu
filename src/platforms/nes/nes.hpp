#include <stdint.h>/** * @file nes.hpp * @brief Definição da classe NES para emulação do Nintendo Entertainment System * @author Mega_Emu Team * @version 1.0.0 * @date 2024-03-13 */#pragma once#include <array>#include <memory>#include <string>#include <vector>#include "../../core/platform.hpp"#include "../../core/rom_system.hpp"// Forward declarationsextern "C"{#include "nes.h"}// Forward declarations C++namespace MegaEmu{    namespace Platforms    {        namespace NES        {            class NESCPU;            class NESPPU;            class NESAPU;            class NESCartridge;        }    }}namespace MegaEmu{    namespace Platforms    {        /**         * @class NES         * @brief Implementação da plataforma Nintendo Entertainment System         *         * Esta classe encapsula a funcionalidade específica do NES,         * delegando para o código C existente quando necessário enquanto         * fornece uma interface C++ moderna.         */        class NES : public Core::Platform        {        public:            /**             * @brief Construtor da classe NES             */            NES();            /**             * @brief Destrutor da classe NES             */            ~NES() override;            /**             * @brief Inicializa o emulador do NES             * @return true se a inicialização foi bem-sucedida, false caso contrário             */            bool initialize() override;            /**             * @brief Retorna o nome da plataforma             * @return Nome da plataforma ("NES")             */            std::string getPlatformName() const override;            /**             * @brief Carrega uma ROM             * @param filename Caminho para o arquivo ROM             * @return true se o carregamento foi bem-sucedido, false caso contrário             */            bool loadROM(const std::string &filename) override;            /**             * @brief Executa um quadro de emulação             * @return true se a execução foi bem-sucedida, false caso contrário             */            bool runFrame() override;            /**             * @brief Retorna o buffer de vídeo             * @return Ponteiro para o buffer de vídeo             */            const uint32_t *getVideoBuffer() const override;            /**             * @brief Retorna a largura da tela             * @return Largura da tela em pixels             */            int32_t getScreenWidth() const override;            /**             * @brief Retorna a altura da tela             * @return Altura da tela em pixels             */            int32_t getScreenHeight() const override;            /**             * @brief Atualiza o estado do controle             * @param index Índice do controle (0-3)             * @param state Estado do controle             */            void updateControllerState(int32_t index, const Core::ControllerState &state) override;            /**             * @brief Salva o estado do emulador             * @param filename Nome do arquivo para salvar o estado             * @return true se o salvamento foi bem-sucedido, false caso contrário             */            bool saveState(const std::string &filename) override;            /**             * @brief Carrega o estado do emulador             * @param filename Nome do arquivo para carregar o estado             * @return true se o carregamento foi bem-sucedido, false caso contrário             */            bool loadState(const std::string &filename) override;        private:            // Estado interno do emulador NES (interface C)            nes_t *m_nesState;            // Componentes C++ do NES            std::unique_ptr<NES::NESCPU> m_cpu;            std::unique_ptr<NES::NESPPU> m_ppu;            std::unique_ptr<NES::NESAPU> m_apu;            std::unique_ptr<NES::NESCartridge> m_cartridge;            // Buffer de vídeo            std::vector<uint32_t> m_videoBuffer;            // Dimensões da tela            int32_t m_screenWidth;            int32_t m_screenHeight;            // ROM carregada            std::vector<uint8_t> m_romData;            // Estado do controle            std::array<Core::ControllerState, 2> m_controllerStates;            // Configuração            bool m_isInitialized;            bool m_isRomLoaded;            /**             * @brief Inicializa os componentes C++ do NES             * @return true se a inicialização foi bem-sucedida, false caso contrário             */            bool initializeComponents();            /**             * @brief Conecta os componentes entre si             * @return true se a conexão foi bem-sucedida, false caso contrário             */            bool connectComponents();        };    } // namespace Platforms} // namespace MegaEmu