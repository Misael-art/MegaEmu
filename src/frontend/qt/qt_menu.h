#ifndef QT_MENU_H
#define QT_MENU_H

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QSlider>
#include <QString>
#include <QVariant>
#include <QVector>
#include <functional>
#include <memory>

// Definições compatíveis com SDL_MENU_H
#define QT_MENU_MAX_TEXT_LENGTH 64
#define QT_MENU_MAX_ITEMS 32
#define QT_MENU_MAX_DEPTH 8

// Tipos de itens de menu (compatíveis com SDL)
typedef enum {
  QT_MENU_ITEM_ACTION,   // Item que executa uma ação
  QT_MENU_ITEM_TOGGLE,   // Item de alternância (on/off)
  QT_MENU_ITEM_SLIDER,   // Item com valor ajustável (barra deslizante)
  QT_MENU_ITEM_CHOICE,   // Item de escolha entre opções
  QT_MENU_ITEM_SUBMENU,  // Item que abre um submenu
  QT_MENU_ITEM_SEPARATOR // Separador visual (não selecionável)
} qt_menu_item_type_t;

// Estrutura para opções de escolha
typedef struct {
  char text[QT_MENU_MAX_TEXT_LENGTH];
  int value;
} qt_menu_choice_option_t;

// Declaração antecipada para referência circular
class QtMenuSystem;
class QtMenuItem;
class QtMenu;

// Classe para gerenciar um item de menu individual
class QtMenuItem : public QObject {
  Q_OBJECT

public:
  explicit QtMenuItem(QObject *parent = nullptr);
  ~QtMenuItem();

  // Métodos de configuração
  void setId(const QString &id);
  void setText(const QString &text);
  void setType(qt_menu_item_type_t type);
  void setEnabled(bool enabled);
  void setVisible(bool visible);

  // Getters
  QString getId() const;
  QString getText() const;
  qt_menu_item_type_t getType() const;
  bool isEnabled() const;
  bool isVisible() const;

  // Configurações específicas por tipo
  void setActionCallback(std::function<void(void *)> callback);
  void setToggleValue(bool value);
  void setToggleCallback(std::function<void(bool, void *)> callback);
  void setSliderRange(int min, int max, int value, int step);
  void setSliderCallback(std::function<void(int, void *)> callback);
  void setChoiceOptions(const QVector<qt_menu_choice_option_t> &options,
                        int selectedIndex);
  void setChoiceCallback(std::function<void(int, void *)> callback);
  void setSubmenu(QtMenu *submenu);

  // Obter widget Qt representando este item
  QWidget *getWidget();
  QAction *getAction();

  // Executar callbacks
  void triggerAction(void *userdata);
  void setToggleState(bool state, void *userdata);
  void setSliderValue(int value, void *userdata);
  void setChoiceIndex(int index, void *userdata);

private:
  QString m_id;
  QString m_text;
  qt_menu_item_type_t m_type;
  bool m_enabled;
  bool m_visible;

  // Ponteiros para widgets Qt
  QAction *m_action;
  QWidget *m_widget;

  // Dados específicos por tipo
  union {
    struct {
      std::function<void(void *)> callback;
    } action;

    struct {
      bool value;
      std::function<void(bool, void *)> callback;
    } toggle;

    struct {
      int min_value;
      int max_value;
      int value;
      int step;
      std::function<void(int, void *)> callback;
    } slider;

    struct {
      QVector<qt_menu_choice_option_t> options;
      int selected_index;
      std::function<void(int, void *)> callback;
    } choice;

    struct {
      QtMenu *submenu;
    } submenu;
  };
};

// Classe para gerenciar um menu
class QtMenu : public QObject {
  Q_OBJECT

public:
  explicit QtMenu(const QString &title, QtMenu *parent = nullptr,
                  QObject *qparent = nullptr);
  ~QtMenu();

  // Métodos de configuração
  void setTitle(const QString &title);
  void setParent(QtMenu *parent);
  void setUserData(void *userdata);
  void setVisible(bool visible);

  // Getters
  QString getTitle() const;
  QtMenu *getParent() const;
  void *getUserData() const;
  bool isVisible() const;
  int getItemCount() const;
  int getSelectedIndex() const;

  // Adicionar itens
  int addItem(const QString &id, const QString &text, qt_menu_item_type_t type);
  int addAction(const QString &id, const QString &text,
                std::function<void(void *)> callback);
  int addToggle(const QString &id, const QString &text, bool initialValue,
                std::function<void(bool, void *)> callback);
  int addSlider(const QString &id, const QString &text, int minValue,
                int maxValue, int initialValue, int step,
                std::function<void(int, void *)> callback);
  int addChoice(const QString &id, const QString &text,
                const QVector<qt_menu_choice_option_t> &options,
                int initialIndex, std::function<void(int, void *)> callback);
  int addSubmenu(const QString &id, const QString &text, QtMenu *submenu);
  int addSeparator();

  // Manipular itens
  bool setItemEnabled(const QString &id, bool enabled);
  bool setItemVisible(const QString &id, bool visible);
  bool setToggleValue(const QString &id, bool value);
  bool setSliderValue(const QString &id, int value);
  bool setChoiceIndex(const QString &id, int index);
  QtMenuItem *getItem(const QString &id);

  // Obter o menu Qt nativo
  QMenu *getQMenu();

  // Criar menu de barra (para janela principal)
  QMenuBar *createMenuBar();

private:
  // Encontrar índice de um item pelo ID
  int findItemIndex(const QString &id) const;

  QString m_title;
  QtMenu *m_parent;
  QVector<QtMenuItem *> m_items;
  int m_selectedIndex;
  void *m_userdata;
  bool m_visible;

  // Referência para o objeto Qt nativo
  QMenu *m_qmenu;
};

// Classe principal do sistema de menu
class QtMenuSystem : public QObject {
  Q_OBJECT

public:
  explicit QtMenuSystem(QWidget *parentWindow, QObject *parent = nullptr);
  ~QtMenuSystem();

  // Inicialização e finalização
  bool init();
  void shutdown();

  // Funções de navegação
  bool navigateTo(QtMenu *menu);
  bool navigateBack();
  bool selectItem();

  // Funções de estado
  void setVisible(bool visible);
  bool isVisible() const;
  QtMenu *getActiveMenu() const;

  // Criar menu
  QtMenu *createMenu(const QString &title, QtMenu *parent = nullptr);
  void destroyMenu(QtMenu *menu);

  // Gerenciamento de eventos Qt
  bool handleEvent(QEvent *event);

  // Obter a barra de menu principal
  QMenuBar *getMenuBar();

  // Definir dados para callbacks
  void setUserData(void *userdata);

private slots:
  void onMenuTriggered();

private:
  QWidget *m_parentWindow;
  QtMenu *m_activeMenu;
  QVector<QtMenu *> m_menuStack;
  int m_menuStackDepth;
  bool m_initialized;
  void *m_userdata;
  QMenuBar *m_menuBar;
  QVector<QtMenu *> m_allMenus; // Para gerenciamento de memória
};

// Funções de compatibilidade com a API SDL do menu
extern "C" {
// Estrutura para opções de escolha
typedef struct {
  char text[QT_MENU_MAX_TEXT_LENGTH];
  int value;
} qt_menu_choice_option_t;

// Estrutura para um item de menu
typedef struct qt_menu_item {
  char id[QT_MENU_MAX_TEXT_LENGTH];   // ID único do item
  char text[QT_MENU_MAX_TEXT_LENGTH]; // Texto exibido
  qt_menu_item_type_t type;           // Tipo do item
  bool enabled;                       // Se o item está habilitado
  bool visible;                       // Se o item está visível
  void *native_item;                  // Ponteiro para QtMenuItem
} qt_menu_item_t;

// Estrutura para um menu
typedef struct qt_menu {
  char title[QT_MENU_MAX_TEXT_LENGTH]; // Título do menu
  struct qt_menu *parent;              // Menu pai (NULL se for menu raiz)
  void *userdata;                      // Dados do usuário
  bool visible;                        // Se o menu está visível
  void *native_menu;                   // Ponteiro para QtMenu
} qt_menu_t;

// Contexto global do menu
typedef struct {
  void *native_context; // Ponteiro para QtMenuSystem
  bool initialized;     // Se o sistema de menu está inicializado
  void *userdata;       // Dados do usuário para callbacks
} qt_menu_context_t;

// Funções de inicialização e finalização
bool qt_menu_init(qt_menu_context_t *context, QWidget *parent_window);
void qt_menu_shutdown(qt_menu_context_t *context);

// Funções de criação e manipulação de menus
qt_menu_t *qt_menu_create(const char *title, qt_menu_t *parent);
void qt_menu_destroy(qt_menu_t *menu);
int qt_menu_add_item(qt_menu_t *menu, const char *id, const char *text,
                     qt_menu_item_type_t type);
int qt_menu_add_action(qt_menu_t *menu, const char *id, const char *text,
                       void (*callback)(void *userdata));
int qt_menu_add_toggle(qt_menu_t *menu, const char *id, const char *text,
                       bool initial_value,
                       void (*callback)(bool value, void *userdata));
int qt_menu_add_slider(qt_menu_t *menu, const char *id, const char *text,
                       int min_value, int max_value, int initial_value,
                       int step, void (*callback)(int value, void *userdata));
int qt_menu_add_choice(qt_menu_t *menu, const char *id, const char *text,
                       qt_menu_choice_option_t *options, int option_count,
                       int initial_index,
                       void (*callback)(int value, void *userdata));
int qt_menu_add_submenu(qt_menu_t *menu, const char *id, const char *text,
                        qt_menu_t *submenu);
int qt_menu_add_separator(qt_menu_t *menu);

// Funções de acesso e modificação
bool qt_menu_set_item_enabled(qt_menu_t *menu, const char *id, bool enabled);
bool qt_menu_set_item_visible(qt_menu_t *menu, const char *id, bool visible);
bool qt_menu_set_toggle_value(qt_menu_t *menu, const char *id, bool value);
bool qt_menu_set_slider_value(qt_menu_t *menu, const char *id, int value);
bool qt_menu_set_choice_index(qt_menu_t *menu, const char *id, int index);
qt_menu_item_t *qt_menu_get_item(qt_menu_t *menu, const char *id);

// Funções de entrada e navegação
bool qt_menu_process_event(qt_menu_context_t *context, void *event);
bool qt_menu_navigate_to(qt_menu_context_t *context, qt_menu_t *menu);
bool qt_menu_navigate_back(qt_menu_context_t *context);
bool qt_menu_select_item(qt_menu_context_t *context);

// Funções de renderização e visibilidade
void qt_menu_set_visible(qt_menu_context_t *context, bool visible);
bool qt_menu_is_visible(const qt_menu_context_t *context);
QMenuBar *qt_menu_get_menu_bar(qt_menu_context_t *context);
}

#endif // QT_MENU_H
