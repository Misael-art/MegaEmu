#ifndef QT_FRONTEND_ADAPTER_H
#define QT_FRONTEND_ADAPTER_H

#include <QObject>
#include <QMainWindow>
#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>
#include <QPixmap>
#include <QKeyEvent>
#include <QGamepad>

#include <stdbool.h>
#include <stdint.h>

#include "frontend/common/frontend_config.h"
#include "frontend/qt/qt_game_renderer.h"
#include "frontend/qt/qt_menu.h"

// Declaração antecipada
class QtGameRenderer;
class QtAudioSystem;
class EmulationThread;

// Estado do frontend Qt
class QtFrontendState : public QObject {
    Q_OBJECT

public:
    explicit QtFrontendState(QObject* parent = nullptr);
    ~QtFrontendState();

    // Getters/Setters
    bool isRunning() const;
    void setRunning(bool running);

    bool isPaused() const;
    void setPaused(bool paused);

    bool isShowMenu() const;
    void setShowMenu(bool showMenu);

    bool isShowFps() const;
    void setShowFps(bool showFps);

    float getFps() const;
    void setFps(float fps);

    uint8_t getControllerState(int index) const;
    void setControllerState(int index, uint8_t state);

    void updateFpsCounter();

    // Acesso às configurações
    emu_frontend_config_t* getConfig();

    // Acesso direto ao renderer e sistema de áudio
    QtGameRenderer* getRenderer();
    QtAudioSystem* getAudioSystem();

    // Acesso ao contexto de menu
    qt_menu_context_t* getMenuContext();

private:
    // Configuração
    emu_frontend_config_t m_config;

    // Renderização
    QtGameRenderer* m_renderer;

    // Áudio
    QtAudioSystem* m_audioSystem;

    // Estado do jogo
    bool m_running;
    bool m_paused;
    bool m_showMenu;
    bool m_showFps;

    // Métricas
    float m_fps;
    uint32_t m_framesSinceLastFps;
    uint32_t m_lastFpsUpdate;

    // Controles
    QGamepad* m_gamepad;
    uint8_t m_controllerStates[4];

    // Sistema de menu
    qt_menu_context_t m_menuContext;
};

// Sistema de áudio Qt
class QtAudioSystem : public QObject {
    Q_OBJECT

public:
    explicit QtAudioSystem(QObject* parent = nullptr);
    ~QtAudioSystem();

    // Inicialização/Finalização
    bool init(int sampleRate, int channels, int bufferSize);
    void shutdown();

    // Processamento de áudio
    bool processAudio(const int16_t* samples, int numSamples);

    // Configuração
    void setVolume(float volume);
    float getVolume() const;
    void setEnabled(bool enabled);
    bool isEnabled() const;

    // Tamanho do buffer
    int getBufferSize() const;
    void setBufferSize(int size);

private slots:
    void handleStateChanged(QAudio::State state);

private:
    // Parâmetros de áudio
    int m_sampleRate;
    int m_channels;
    int m_bufferSize;
    float m_volume;
    bool m_enabled;

    // Componentes Qt de áudio
    QAudioFormat m_format;
    QAudioOutput* m_audioOutput;
    QIODevice* m_audioDevice;

    // Buffer de áudio
    QByteArray m_buffer;
    int m_bufferUsed;

    // Sincronização
    QMutex m_mutex;
};

// Thread de emulação para manter o loop principal separado da UI
class EmulationThread : public QThread {
    Q_OBJECT

public:
    explicit EmulationThread(QtFrontendState* state, QObject* parent = nullptr);
    ~EmulationThread();

    // Controle da emulação
    void startEmulation();
    void stopEmulation();
    void pauseEmulation(bool pause);
    bool isPaused() const;

    // Configuração
    void setFrameLimit(int fps);
    int getFrameLimit() const;

    // Callback de frame e áudio
    using FrameCallback = std::function<void(const uint32_t*, int, int)>;
    using AudioCallback = std::function<void(const int16_t*, int)>;

    void setFrameCallback(FrameCallback callback);
    void setAudioCallback(AudioCallback callback);

signals:
    void frameReady(const uint32_t* framebuffer, int width, int height);
    void audioReady(const int16_t* samples, int numSamples);
    void emulationStopped();

protected:
    // Implementação do QThread
    void run() override;

private:
    QtFrontendState* m_state;
    bool m_running;
    bool m_paused;
    int m_frameLimit;
    FrameCallback m_frameCallback;
    AudioCallback m_audioCallback;

    // Sincronização
    QMutex m_mutex;
    QWaitCondition m_pauseCondition;

    // Timer para limitar FPS
    QElapsedTimer m_frameTimer;
};

// Classe principal do adaptador Qt
class QtFrontendAdapter : public QObject {
    Q_OBJECT

public:
    explicit QtFrontendAdapter(QObject* parent = nullptr);
    ~QtFrontendAdapter();

    // Inicialização e finalização
    bool init(const emu_frontend_config_t* config);
    void shutdown();

    // Processamento de eventos e renderização
    bool processEvents();
    void renderFrame(const uint32_t* framebuffer, const int16_t* audioSamples, int numSamples);
    bool processAudio(const int16_t* audioSamples, int numSamples);

    // Estado do frontend
    bool isRunning() const;
    uint8_t getControllerState(int controller);
    float getFps() const;

    // Controle da janela
    void toggleFullscreen();
    void setTitle(const char* title);

    // Acesso ao estado Qt
    QtFrontendState* getState();

    // Acesso à janela principal
    QMainWindow* getMainWindow();

    // Criar menus para a aplicação
    bool createMenus();

signals:
    void windowClosed();
    void keyPressed(int key);
    void keyReleased(int key);
    void controllerChanged(int controller, uint8_t state);

private slots:
    void handleFrameReady(const uint32_t* framebuffer, int width, int height);
    void handleAudioReady(const int16_t* samples, int numSamples);
    void handleEmulationStopped();

    // Event handlers
    void handleKeyPressed(QKeyEvent* event);
    void handleKeyReleased(QKeyEvent* event);
    void handleGamepadButtonPressed(int button);
    void handleGamepadButtonReleased(int button);
    void handleGamepadAxisChanged(int axis, double value);

private:
    // Janela principal
    QMainWindow* m_mainWindow;
    QWidget* m_centralWidget;

    // Widget de render
    QtGameRenderer* m_renderer;

    // Estado do frontend
    QtFrontendState* m_state;

    // Thread de emulação
    EmulationThread* m_emulationThread;

    // Timer para limitar UI FPS
    QTimer* m_uiUpdateTimer;

    // Auxiliares
    void mapKeyboardToController(QKeyEvent* event, bool pressed);
    void setupGamepad();
    void updateGamepadState();

    // Criar itens de menu
    bool createMainMenu();
    bool createVideoMenu();
    bool createAudioMenu();
    bool createInputMenu();
    bool createDebugMenu();
};

// Funções de API em C para compatibilidade com o frontend comum
extern "C" {
    // Inicialização e finalização
    bool qt_frontend_init(const emu_frontend_config_t* config);
    void qt_frontend_shutdown(void);

    // Processamento de eventos e renderização
    bool qt_frontend_process_events(void);
    void qt_frontend_render_frame(const uint32_t* framebuffer, const int16_t* audioSamples, int numSamples);
    bool qt_frontend_process_audio(const int16_t* audioSamples, int numSamples);

    // Estado do frontend
    bool qt_frontend_is_running(void);
    uint8_t qt_frontend_get_controller_state(int controller);
    float qt_frontend_get_fps(void);

    // Controle da janela
    void qt_frontend_toggle_fullscreen(void);
    void qt_frontend_set_title(const char* title);
}

#endif // QT_FRONTEND_ADAPTER_H
