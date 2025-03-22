#include <stdint.h>/** * @file input_system.hpp * @brief Define a classe base para o sistema de entrada * @author Mega_Emu Team * @version 1.0.0 * @date 2024-03-10 */#pragma once#include <SDL.h>#include <array>#include <map>#include <string>#include <vector>#include "../core/platform.hpp"namespace MegaEmu{    namespace Input    {        /**         * @brief Enumeração dos tipos de controle suportados         */        enum class ControllerType        {            Keyboard, ///< Teclado            Gamepad,  ///< Controle de jogo            None      ///< Nenhum controle        };        /**         * @brief Classe que gerencia o sistema de entrada         */        class InputSystem        {        public:            /**             * @brief Construtor             */            InputSystem()                : quit(false)            {                // Inicializar mapeamento padrão de teclas                keyboardMap = {                    {SDLK_z, &Core::ControllerState::a},                    {SDLK_x, &Core::ControllerState::b},                    {SDLK_a, &Core::ControllerState::x},                    {SDLK_s, &Core::ControllerState::y},                    {SDLK_RETURN, &Core::ControllerState::start},                    {SDLK_RSHIFT, &Core::ControllerState::select},                    {SDLK_UP, &Core::ControllerState::up},                    {SDLK_DOWN, &Core::ControllerState::down},                    {SDLK_LEFT, &Core::ControllerState::left},                    {SDLK_RIGHT, &Core::ControllerState::right}};                // Inicializar mapeamento padrão de botões do gamepad                gamepadMap = {                    {SDL_CONTROLLER_BUTTON_A, &Core::ControllerState::a},                    {SDL_CONTROLLER_BUTTON_B, &Core::ControllerState::b},                    {SDL_CONTROLLER_BUTTON_X, &Core::ControllerState::x},                    {SDL_CONTROLLER_BUTTON_Y, &Core::ControllerState::y},                    {SDL_CONTROLLER_BUTTON_START, &Core::ControllerState::start},                    {SDL_CONTROLLER_BUTTON_BACK, &Core::ControllerState::select},                    {SDL_CONTROLLER_BUTTON_DPAD_UP, &Core::ControllerState::up},                    {SDL_CONTROLLER_BUTTON_DPAD_DOWN, &Core::ControllerState::down},                    {SDL_CONTROLLER_BUTTON_DPAD_LEFT, &Core::ControllerState::left},                    {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, &Core::ControllerState::right}};                // Inicializar estado dos controles                controllerStates.fill(Core::ControllerState());            }            /**             * @brief Destrutor             */            ~InputSystem()            {                // Fechar todos os gamepads abertos                for (auto &gamepad : gamepads)                {                    if (gamepad)                    {                        SDL_GameControllerClose(gamepad);                    }                }            }            /**             * @brief Processa eventos de entrada             * @return true se o programa deve continuar, false para sair             */            bool processEvents()            {                SDL_Event event;                while (SDL_PollEvent(&event))                {                    switch (event.type)                    {                    case SDL_QUIT:                        quit = true;                        break;                    case SDL_KEYDOWN:                        if (event.key.keysym.sym == SDLK_ESCAPE)                        {                            quit = true;                        }                        handleKeyboardEvent(event.key, true);                        break;                    case SDL_KEYUP:                        handleKeyboardEvent(event.key, false);                        break;                    case SDL_CONTROLLERBUTTONDOWN:                        handleGamepadButton(event.cbutton, true);                        break;                    case SDL_CONTROLLERBUTTONUP:                        handleGamepadButton(event.cbutton, false);                        break;                    case SDL_CONTROLLERDEVICEADDED:                        handleGamepadConnection(event.cdevice.which);                        break;                    case SDL_CONTROLLERDEVICEREMOVED:                        handleGamepadDisconnection(event.cdevice.which);                        break;                    }                }                return !quit;            }            /**             * @brief Obtém o estado atual de um controle             * @param index Índice do controle (0-3)             * @return Estado do controle             */            Core::ControllerState getControllerState(int32_t index) const            {                if (index >= 0 && index < MAX_CONTROLLERS)                {                    return controllerStates[index];                }                return Core::ControllerState();            }            /**             * @brief Define o mapeamento de teclas             * @param keymap Novo mapeamento de teclas             */            void setKeyboardMap(const std::map<SDL_Keycode, Core::ButtonState Core::ControllerState::*> &keymap)            {                keyboardMap = keymap;            }            /**             * @brief Define o mapeamento de botões do gamepad             * @param buttonmap Novo mapeamento de botões             */            void setGamepadMap(const std::map<SDL_GameControllerButton, Core::ButtonState Core::ControllerState::*> &buttonmap)            {                gamepadMap = buttonmap;            }        private:            static const int32_t MAX_CONTROLLERS = 4; ///< Número máximo de controles suportados            /**             * @brief Processa eventos do teclado             * @param event Evento do teclado             * @param pressed true se a tecla foi pressionada, false se foi solta             */            void handleKeyboardEvent(const SDL_KeyboardEvent &event, bool pressed)            {                auto it = keyboardMap.find(event.keysym.sym);                if (it != keyboardMap.end())                {                    Core::ButtonState &buttonState = (controllerStates[0].*(it->second));                    buttonState.pressed = pressed;                    buttonState.held = pressed;                    buttonState.released = !pressed;                }            }            /**             * @brief Processa eventos de botão do gamepad             * @param event Evento do botão             * @param pressed true se o botão foi pressionado, false se foi solto             */            void handleGamepadButton(const SDL_ControllerButtonEvent &event, bool pressed)            {                if (event.which < MAX_CONTROLLERS)                {                    auto it = gamepadMap.find(static_cast<SDL_GameControllerButton>(event.button));                    if (it != gamepadMap.end())                    {                        Core::ButtonState &buttonState = (controllerStates[event.which].*(it->second));                        buttonState.pressed = pressed;                        buttonState.held = pressed;                        buttonState.released = !pressed;                    }                }            }            /**             * @brief Processa conexão de gamepad             * @param deviceIndex Índice do dispositivo             */            void handleGamepadConnection(int32_t deviceIndex)            {                if (SDL_IsGameController(deviceIndex))                {                    SDL_GameController *gamepad = SDL_GameControllerOpen(deviceIndex);                    if (gamepad)                    {                        int32_t playerIndex = SDL_GameControllerGetPlayerIndex(gamepad);                        if (playerIndex >= 0 && playerIndex < MAX_CONTROLLERS)                        {                            gamepads[playerIndex] = gamepad;                        }                    }                }            }            /**             * @brief Processa desconexão de gamepad             * @param deviceIndex Índice do dispositivo             */            void handleGamepadDisconnection(int32_t deviceIndex)            {                for (int32_t i = 0; i < MAX_CONTROLLERS; ++i)                {                    if (gamepads[i] && SDL_GameControllerGetAttached(gamepads[i]) == SDL_FALSE)                    {                        SDL_GameControllerClose(gamepads[i]);                        gamepads[i] = nullptr;                        controllerStates[i] = Core::ControllerState();                    }                }            }            bool quit;                                                                                 ///< Flag de saída            std::array<Core::ControllerState, MAX_CONTROLLERS> controllerStates;                       ///< Estado dos controles            std::array<SDL_GameController *, MAX_CONTROLLERS> gamepads;                                ///< Gamepads conectados            std::map<SDL_Keycode, Core::ButtonState Core::ControllerState::*> keyboardMap;             ///< Mapeamento de teclas            std::map<SDL_GameControllerButton, Core::ButtonState Core::ControllerState::*> gamepadMap; ///< Mapeamento de botões do gamepad        };    } // namespace Input} // namespace MegaEmu