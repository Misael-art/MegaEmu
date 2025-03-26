#ifndef QT_GAME_RENDERER_H
#define QT_GAME_RENDERER_H

#include <QElapsedTimer>
#include <QImage>
#include <QMap>
#include <QMutex>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QPainter>
#include <QSurfaceFormat>
#include <QTimer>

#include <memory>
#include <string>
#include <vector>

// Tamanho máximo da paleta de cores (compatível com o SDL)
#define COLOR_PALETTE_SIZE 64

// Configuração do renderizador Qt (compatível com SDL)
typedef struct {
  int32_t window_width;
  int32_t window_height;
  int32_t game_width;
  int32_t game_height;
  float scale_factor;
  bool vsync_enabled;
  bool fullscreen;
  bool smooth_scaling;
  bool integer_scaling;
  bool scanlines_enabled;
  bool crt_effect;
  char system_name[32]; // Nome do sistema: "NES", "MEGA_DRIVE", etc.
} qt_renderer_config_t;

// Cache de texturas
class TextureCacheEntry {
public:
  TextureCacheEntry(const std::string &key, QOpenGLTexture *texture, int width,
                    int height);
  ~TextureCacheEntry();

  // Getters
  std::string getKey() const;
  QOpenGLTexture *getTexture() const;
  int getWidth() const;
  int getHeight() const;
  qint64 getLastUseTime() const;
  qint64 getCreationTime() const;

  // Atualizar tempo de uso
  void updateLastUseTime();

private:
  std::string m_key;
  QOpenGLTexture *m_texture;
  int m_width;
  int m_height;
  qint64 m_lastUseTime;
  qint64 m_creationTime;
};

// Classe do renderizador de jogos com OpenGL
class QtGameRenderer : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

public:
  explicit QtGameRenderer(QWidget *parent = nullptr);
  ~QtGameRenderer();

  // Inicialização com configuração
  bool init(const qt_renderer_config_t *config);
  void shutdown();

  // Configuração
  bool setConfig(const qt_renderer_config_t *config);
  void getConfig(qt_renderer_config_t *config) const;

  // Funções de renderização
  bool beginFrame();
  bool endFrame();
  bool updateGameTexture(const uint32_t *pixels);
  bool drawFrame();
  bool drawOverlay(const uint32_t *pixels);

  // Efeitos visuais
  bool applyFilter(const QString &filterName);

  // Funções de configuração
  bool toggleFullscreen();
  bool setScale(float scale);
  bool setSmoothScaling(bool smooth);
  bool setIntegerScaling(bool integer);
  bool setScanlines(bool enabled);
  bool setCrtEffect(bool enabled);
  bool setColorPalette(const uint32_t *palette, int paletteSize);

  // Tamanho da janela
  void getOutputSize(int *width, int *height) const;
  void getGameRect(QRect *rect) const;

  // Event handlers
  bool handleResize(int width, int height);

  // Obter informações
  QImage captureFrame() const;

  // Debug
  bool isInitialized() const;
  QString getGLInfo() const;

signals:
  void frameRendered(float fps);
  void resized(int width, int height);

protected:
  // Sobrescritas do QOpenGLWidget
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

private:
  // Funções auxiliares
  void setupShaders();
  void setupGeometry();
  void setupTextures();
  void calculateGameRect();
  void clearTextureCache();

  // Helpers para shaders
  QOpenGLShaderProgram *
  createShaderProgram(const QString &vertexShaderSource,
                      const QString &fragmentShaderSource);
  bool compileShader(QOpenGLShader *shader, const QString &source);
  void updateShaderUniforms();

  // Cache de texturas
  QOpenGLTexture *getCachedTexture(const QString &key, int width, int height,
                                   QOpenGLTexture::Target target,
                                   QOpenGLTexture::PixelFormat format);

  // Setup das paletas de cor específicas para sistemas
  void setupColorPaletteForSystem(const QString &system);

  // Desenho de scanlines e efeitos CRT
  void renderScanlines();
  void renderCrtEffect();

  // Propriedades privadas
  bool m_initialized;
  qt_renderer_config_t m_config;

  // Objetos OpenGL
  QOpenGLShaderProgram *m_mainShader;
  QOpenGLShaderProgram *m_scanlinesShader;
  QOpenGLShaderProgram *m_crtShader;
  QOpenGLVertexArrayObject m_vao;
  QOpenGLBuffer m_vbo;
  QOpenGLTexture *m_gameTexture;
  QOpenGLTexture *m_overlayTexture;
  QOpenGLTexture *m_scanlinesTexture;

  // Buffer temporário para a textura
  uint32_t *m_frameBuffer;
  QMutex m_bufferMutex;

  // Frame timing
  QElapsedTimer m_frameTimer;
  float m_fps;

  // Geometria e layout
  QRect m_gameRect;
  int m_viewportWidth;
  int m_viewportHeight;

  // Paleta de cores para sistemas específicos
  uint32_t m_colorPalette[COLOR_PALETTE_SIZE];
  bool m_usingColorPalette;

  // Cache de texturas
  QMap<QString, TextureCacheEntry *> m_textureCache;
  QTimer m_textureCacheCleanupTimer;

  // Flags de estado
  bool m_frameStarted;
  bool m_textureUpdated;
  bool m_fullUpdateRequired;
};

#endif // QT_GAME_RENDERER_H
