#include "frontend/qt/qt_menu.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QSlider>
#include <QWidget>

#include <cstring>

// =============== QtMenuItem Implementation ================

QtMenuItem::QtMenuItem(QObject *parent)
    : QObject(parent), m_enabled(true), m_visible(true), m_action(nullptr),
      m_widget(nullptr) {

  // Inicializar union com zeros
  memset(&action, 0, sizeof(action));
}

QtMenuItem::~QtMenuItem() {
  // QAction e QWidget são gerenciados pelo Qt parent
  // Não precisam ser deletados explicitamente
}

void QtMenuItem::setId(const QString &id) { m_id = id; }

void QtMenuItem::setText(const QString &text) {
  m_text = text;

  // Atualizar widget/action se já existir
  if (m_action) {
    m_action->setText(text);
  }
}

void QtMenuItem::setType(qt_menu_item_type_t type) { m_type = type; }

void QtMenuItem::setEnabled(bool enabled) {
  m_enabled = enabled;

  // Atualizar widget/action se já existir
  if (m_action) {
    m_action->setEnabled(enabled);
  }
  if (m_widget) {
    m_widget->setEnabled(enabled);
  }
}

void QtMenuItem::setVisible(bool visible) {
  m_visible = visible;

  // Atualizar widget/action se já existir
  if (m_action) {
    m_action->setVisible(visible);
  }
  if (m_widget) {
    m_widget->setVisible(visible);
  }
}

QString QtMenuItem::getId() const { return m_id; }

QString QtMenuItem::getText() const { return m_text; }

qt_menu_item_type_t QtMenuItem::getType() const { return m_type; }

bool QtMenuItem::isEnabled() const { return m_enabled; }

bool QtMenuItem::isVisible() const { return m_visible; }

void QtMenuItem::setActionCallback(std::function<void(void *)> callback) {
  action.callback = callback;
}

void QtMenuItem::setToggleValue(bool value) {
  toggle.value = value;

  // Atualizar widget/action se já existir
  if (m_action && m_action->isCheckable()) {
    m_action->setChecked(value);
  }
}

void QtMenuItem::setToggleCallback(std::function<void(bool, void *)> callback) {
  toggle.callback = callback;
}

void QtMenuItem::setSliderRange(int min, int max, int value, int step) {
  slider.min_value = min;
  slider.max_value = max;
  slider.value = value;
  slider.step = step;

  // Atualizar widget se for um slider
  if (QSlider *slider = qobject_cast<QSlider *>(m_widget)) {
    slider->setMinimum(min);
    slider->setMaximum(max);
    slider->setValue(value);
    slider->setSingleStep(step);
  }
}

void QtMenuItem::setSliderCallback(std::function<void(int, void *)> callback) {
  slider.callback = callback;
}

void QtMenuItem::setChoiceOptions(
    const QVector<qt_menu_choice_option_t> &options, int selectedIndex) {
  choice.options = options;
  choice.selected_index = selectedIndex;

  // Atualizar widget se for um combobox
  if (QComboBox *combo = qobject_cast<QComboBox *>(m_widget)) {
    combo->clear();
    for (const auto &option : options) {
      combo->addItem(QString(option.text));
    }
    if (selectedIndex >= 0 && selectedIndex < options.size()) {
      combo->setCurrentIndex(selectedIndex);
    }
  }
}

void QtMenuItem::setChoiceCallback(std::function<void(int, void *)> callback) {
  choice.callback = callback;
}

void QtMenuItem::setSubmenu(QtMenu *submenu) { submenu.submenu = submenu; }

QAction *QtMenuItem::getAction() {
  if (!m_action) {
    // Criar QAction com base no tipo
    switch (m_type) {
    case QT_MENU_ITEM_ACTION:
      m_action = new QAction(m_text);
      connect(m_action, &QAction::triggered, this, [this]() {
        if (action.callback) {
          action.callback(nullptr); // Userdata será passado depois
        }
      });
      break;

    case QT_MENU_ITEM_TOGGLE:
      m_action = new QAction(m_text);
      m_action->setCheckable(true);
      m_action->setChecked(toggle.value);
      connect(m_action, &QAction::toggled, this, [this](bool checked) {
        if (toggle.callback) {
          toggle.callback(checked, nullptr); // Userdata será passado depois
        }
      });
      break;

    case QT_MENU_ITEM_SUBMENU:
      if (submenu.submenu) {
        m_action = new QAction(m_text);
      }
      break;

    case QT_MENU_ITEM_SEPARATOR:
      m_action = new QAction();
      m_action->setSeparator(true);
      break;

    default:
      // Slider e Choice não tem action, apenas widget
      break;
    }

    if (m_action) {
      m_action->setEnabled(m_enabled);
      m_action->setVisible(m_visible);
    }
  }

  return m_action;
}

QWidget *QtMenuItem::getWidget() {
  if (!m_widget) {
    // Criar widget com base no tipo
    switch (m_type) {
    case QT_MENU_ITEM_SLIDER: {
      QSlider *slider = new QSlider(Qt::Horizontal);
      slider->setMinimum(this->slider.min_value);
      slider->setMaximum(this->slider.max_value);
      slider->setValue(this->slider.value);
      slider->setSingleStep(this->slider.step);

      connect(slider, &QSlider::valueChanged, this, [this](int value) {
        if (this->slider.callback) {
          this->slider.callback(value, nullptr); // Userdata será passado depois
        }
      });

      m_widget = slider;
      break;
    }

    case QT_MENU_ITEM_CHOICE: {
      QComboBox *combo = new QComboBox();
      for (const auto &option : choice.options) {
        combo->addItem(QString(option.text));
      }
      if (choice.selected_index >= 0 &&
          choice.selected_index < choice.options.size()) {
        combo->setCurrentIndex(choice.selected_index);
      }

      connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
              [this](int index) {
                if (this->choice.callback) {
                  int value = 0;
                  if (index >= 0 && index < this->choice.options.size()) {
                    value = this->choice.options[index].value;
                  }
                  this->choice.callback(
                      value, nullptr); // Userdata será passado depois
                }
              });

      m_widget = combo;
      break;
    }

    default:
      // Outros tipos não têm widgets específicos
      break;
    }

    if (m_widget) {
      m_widget->setEnabled(m_enabled);
      m_widget->setVisible(m_visible);
    }
  }

  return m_widget;
}

void QtMenuItem::triggerAction(void *userdata) {
  if (m_type == QT_MENU_ITEM_ACTION && action.callback) {
    action.callback(userdata);
  }
}

void QtMenuItem::setToggleState(bool state, void *userdata) {
  if (m_type == QT_MENU_ITEM_TOGGLE) {
    toggle.value = state;
    if (m_action && m_action->isCheckable()) {
      m_action->setChecked(state);
    }
    if (toggle.callback) {
      toggle.callback(state, userdata);
    }
  }
}

void QtMenuItem::setSliderValue(int value, void *userdata) {
  if (m_type == QT_MENU_ITEM_SLIDER) {
    slider.value = value;
    if (QSlider *sliderWidget = qobject_cast<QSlider *>(m_widget)) {
      sliderWidget->setValue(value);
    }
    if (slider.callback) {
      slider.callback(value, userdata);
    }
  }
}

void QtMenuItem::setChoiceIndex(int index, void *userdata) {
  if (m_type == QT_MENU_ITEM_CHOICE) {
    if (index >= 0 && index < choice.options.size()) {
      choice.selected_index = index;
      if (QComboBox *combo = qobject_cast<QComboBox *>(m_widget)) {
        combo->setCurrentIndex(index);
      }
      if (choice.callback) {
        int value = choice.options[index].value;
        choice.callback(value, userdata);
      }
    }
  }
}

// =============== QtMenu Implementation ================

QtMenu::QtMenu(const QString &title, QtMenu *parent, QObject *qparent)
    : QObject(qparent), m_title(title), m_parent(parent), m_selectedIndex(0),
      m_userdata(nullptr), m_visible(true), m_qmenu(new QMenu(title)) {}

QtMenu::~QtMenu() {
  // Limpar itens de menu
  for (QtMenuItem *item : m_items) {
    delete item;
  }
  m_items.clear();

  // QMenu é gerenciado pelo Qt parent
  // Não precisa ser deletado explicitamente
}

void QtMenu::setTitle(const QString &title) {
  m_title = title;
  m_qmenu->setTitle(title);
}

void QtMenu::setParent(QtMenu *parent) { m_parent = parent; }

void QtMenu::setUserData(void *userdata) { m_userdata = userdata; }

void QtMenu::setVisible(bool visible) {
  m_visible = visible;
  m_qmenu->menuAction()->setVisible(visible);
}

QString QtMenu::getTitle() const { return m_title; }

QtMenu *QtMenu::getParent() const { return m_parent; }

void *QtMenu::getUserData() const { return m_userdata; }

bool QtMenu::isVisible() const { return m_visible; }

int QtMenu::getItemCount() const { return m_items.size(); }

int QtMenu::getSelectedIndex() const { return m_selectedIndex; }

int QtMenu::findItemIndex(const QString &id) const {
  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items[i]->getId() == id) {
      return i;
    }
  }
  return -1;
}

int QtMenu::addItem(const QString &id, const QString &text,
                    qt_menu_item_type_t type) {
  if (m_items.size() >= QT_MENU_MAX_ITEMS) {
    qWarning() << "Menu atingiu o número máximo de itens:" << QT_MENU_MAX_ITEMS;
    return -1;
  }

  // Verificar se o ID já existe
  if (findItemIndex(id) >= 0) {
    qWarning() << "Item com ID" << id << "já existe no menu" << m_title;
    return -1;
  }

  // Criar novo item
  QtMenuItem *item = new QtMenuItem(this);
  item->setId(id);
  item->setText(text);
  item->setType(type);

  // Adicionar ao vetor de itens
  m_items.append(item);

  // Para tipos específicos, adicionar ao QMenu
  switch (type) {
  case QT_MENU_ITEM_ACTION:
  case QT_MENU_ITEM_TOGGLE:
    m_qmenu->addAction(item->getAction());
    break;

  case QT_MENU_ITEM_SEPARATOR:
    m_qmenu->addSeparator();
    break;

  case QT_MENU_ITEM_SUBMENU:
    // Será adicionado depois em addSubmenu
    break;

  default:
    // Slider e Choice precisam de tratamento especial
    break;
  }

  return m_items.size() - 1;
}

int QtMenu::addAction(const QString &id, const QString &text,
                      std::function<void(void *)> callback) {
  int index = addItem(id, text, QT_MENU_ITEM_ACTION);
  if (index >= 0) {
    m_items[index]->setActionCallback(callback);
  }
  return index;
}

int QtMenu::addToggle(const QString &id, const QString &text, bool initialValue,
                      std::function<void(bool, void *)> callback) {
  int index = addItem(id, text, QT_MENU_ITEM_TOGGLE);
  if (index >= 0) {
    m_items[index]->setToggleValue(initialValue);
    m_items[index]->setToggleCallback(callback);
  }
  return index;
}

int QtMenu::addSlider(const QString &id, const QString &text, int minValue,
                      int maxValue, int initialValue, int step,
                      std::function<void(int, void *)> callback) {
  int index = addItem(id, text, QT_MENU_ITEM_SLIDER);
  if (index >= 0) {
    m_items[index]->setSliderRange(minValue, maxValue, initialValue, step);
    m_items[index]->setSliderCallback(callback);
  }
  return index;
}

int QtMenu::addChoice(const QString &id, const QString &text,
                      const QVector<qt_menu_choice_option_t> &options,
                      int initialIndex,
                      std::function<void(int, void *)> callback) {
  int index = addItem(id, text, QT_MENU_ITEM_CHOICE);
  if (index >= 0) {
    m_items[index]->setChoiceOptions(options, initialIndex);
    m_items[index]->setChoiceCallback(callback);
  }
  return index;
}

int QtMenu::addSubmenu(const QString &id, const QString &text,
                       QtMenu *submenu) {
  int index = addItem(id, text, QT_MENU_ITEM_SUBMENU);
  if (index >= 0 && submenu) {
    m_items[index]->setSubmenu(submenu);
    m_qmenu->addMenu(submenu->getQMenu());
  }
  return index;
}

int QtMenu::addSeparator() {
  // Gerar um ID único para o separador
  QString id = QString("separator_%1").arg(m_items.size());
  return addItem(id, "", QT_MENU_ITEM_SEPARATOR);
}

bool QtMenu::setItemEnabled(const QString &id, bool enabled) {
  int index = findItemIndex(id);
  if (index >= 0) {
    m_items[index]->setEnabled(enabled);
    return true;
  }
  return false;
}

bool QtMenu::setItemVisible(const QString &id, bool visible) {
  int index = findItemIndex(id);
  if (index >= 0) {
    m_items[index]->setVisible(visible);
    return true;
  }
  return false;
}

bool QtMenu::setToggleValue(const QString &id, bool value) {
  int index = findItemIndex(id);
  if (index >= 0 && m_items[index]->getType() == QT_MENU_ITEM_TOGGLE) {
    m_items[index]->setToggleValue(value);
    return true;
  }
  return false;
}

bool QtMenu::setSliderValue(const QString &id, int value) {
  int index = findItemIndex(id);
  if (index >= 0 && m_items[index]->getType() == QT_MENU_ITEM_SLIDER) {
    m_items[index]->setSliderValue(value, m_userdata);
    return true;
  }
  return false;
}

bool QtMenu::setChoiceIndex(const QString &id, int index) {
  int itemIndex = findItemIndex(id);
  if (itemIndex >= 0 && m_items[itemIndex]->getType() == QT_MENU_ITEM_CHOICE) {
    m_items[itemIndex]->setChoiceIndex(index, m_userdata);
    return true;
  }
  return false;
}

QtMenuItem *QtMenu::getItem(const QString &id) {
  int index = findItemIndex(id);
  return (index >= 0) ? m_items[index] : nullptr;
}

QMenu *QtMenu::getQMenu() { return m_qmenu; }

QMenuBar *QtMenu::createMenuBar() {
  QMenuBar *menuBar = new QMenuBar();
  menuBar->addMenu(m_qmenu);
  return menuBar;
}

// =============== QtMenuSystem Implementation ================

QtMenuSystem::QtMenuSystem(QWidget *parentWindow, QObject *parent)
    : QObject(parent), m_parentWindow(parentWindow), m_activeMenu(nullptr),
      m_menuStackDepth(0), m_initialized(false), m_userdata(nullptr),
      m_menuBar(nullptr) {}

QtMenuSystem::~QtMenuSystem() { shutdown(); }

bool QtMenuSystem::init() {
  if (m_initialized) {
    return true;
  }

  // Criar barra de menu principal se tivermos uma janela pai
  if (m_parentWindow) {
    m_menuBar = new QMenuBar(m_parentWindow);
  }

  m_initialized = true;
  return true;
}

void QtMenuSystem::shutdown() {
  // Limpar todos os menus
  for (QtMenu *menu : m_allMenus) {
    delete menu;
  }
  m_allMenus.clear();

  // Resetar estado
  m_activeMenu = nullptr;
  m_menuStackDepth = 0;
  m_menuStack.clear();

  // QMenuBar é propriedade da janela pai
  m_menuBar = nullptr;

  m_initialized = false;
}

QtMenu *QtMenuSystem::createMenu(const QString &title, QtMenu *parent) {
  QtMenu *menu = new QtMenu(title, parent);
  m_allMenus.append(menu);

  // Se for um menu raiz e tivermos uma barra de menu, adicionar à barra
  if (!parent && m_menuBar) {
    m_menuBar->addMenu(menu->getQMenu());
  }

  return menu;
}

void QtMenuSystem::destroyMenu(QtMenu *menu) {
  if (!menu) {
    return;
  }

  // Remover da lista de todos os menus
  int index = m_allMenus.indexOf(menu);
  if (index >= 0) {
    m_allMenus.removeAt(index);
  }

  // Se for o menu ativo, limpar
  if (menu == m_activeMenu) {
    m_activeMenu = nullptr;
  }

  // Remover da pilha de navegação
  for (int i = 0; i < m_menuStack.size(); ++i) {
    if (m_menuStack[i] == menu) {
      m_menuStack.remove(i);
      --m_menuStackDepth;
      break;
    }
  }

  delete menu;
}

bool QtMenuSystem::navigateTo(QtMenu *menu) {
  if (!menu || !m_initialized) {
    return false;
  }

  // Verificar se já estamos no limite da pilha
  if (m_menuStackDepth >= QT_MENU_MAX_DEPTH) {
    qWarning() << "Profundidade máxima de menu atingida:" << QT_MENU_MAX_DEPTH;
    return false;
  }

  // Adicionar à pilha
  m_menuStack.append(menu);
  ++m_menuStackDepth;
  m_activeMenu = menu;

  return true;
}

bool QtMenuSystem::navigateBack() {
  if (m_menuStackDepth <= 1 || !m_initialized) {
    return false;
  }

  // Remover menu atual da pilha
  m_menuStack.removeLast();
  --m_menuStackDepth;

  // Atualizar menu ativo
  m_activeMenu = m_menuStack.last();

  return true;
}

bool QtMenuSystem::selectItem() {
  if (!m_activeMenu || !m_initialized) {
    return false;
  }

  int selectedIndex = m_activeMenu->getSelectedIndex();
  if (selectedIndex < 0 || selectedIndex >= m_activeMenu->getItemCount()) {
    return false;
  }

  QtMenuItem *item = m_activeMenu->getItem(QString::number(selectedIndex));
  if (!item || !item->isEnabled()) {
    return false;
  }

  // Executar ação com base no tipo
  switch (item->getType()) {
  case QT_MENU_ITEM_ACTION:
    item->triggerAction(m_userdata);
    return true;

  case QT_MENU_ITEM_TOGGLE:
    // Inverter estado
    item->setToggleState(!item->isEnabled(), m_userdata);
    return true;

  case QT_MENU_ITEM_SUBMENU: {
    // Navegar para submenu
    QtMenu *submenu =
        item->getAction()->menu()->property("qtmenu").value<QtMenu *>();
    if (submenu) {
      return navigateTo(submenu);
    }
    return false;
  }

  default:
    // Outros tipos são manipulados de forma diferente
    return false;
  }
}

void QtMenuSystem::setVisible(bool visible) {
  if (m_menuBar) {
    m_menuBar->setVisible(visible);
  }
}

bool QtMenuSystem::isVisible() const {
  return m_menuBar ? m_menuBar->isVisible() : false;
}

QtMenu *QtMenuSystem::getActiveMenu() const { return m_activeMenu; }

bool QtMenuSystem::handleEvent(QEvent *event) {
  if (!m_initialized || !m_activeMenu) {
    return false;
  }

  // Processar apenas eventos de teclado por enquanto
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

    switch (keyEvent->key()) {
    case Qt::Key_Up:
      // Mover seleção para cima
      if (m_activeMenu->getSelectedIndex() > 0) {
        m_activeMenu->getSelectedIndex() - 1;
        return true;
      }
      break;

    case Qt::Key_Down:
      // Mover seleção para baixo
      if (m_activeMenu->getSelectedIndex() < m_activeMenu->getItemCount() - 1) {
        m_activeMenu->getSelectedIndex() + 1;
        return true;
      }
      break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
      // Selecionar item atual
      return selectItem();

    case Qt::Key_Escape:
      // Voltar para menu anterior
      return navigateBack();

    default:
      break;
    }
  }

  return false;
}

QMenuBar *QtMenuSystem::getMenuBar() { return m_menuBar; }

void QtMenuSystem::setUserData(void *userdata) { m_userdata = userdata; }

void QtMenuSystem::onMenuTriggered() {
  // Este slot é conectado aos sinais dos menus
  // para facilitar a comunicação entre Qt e nosso sistema
}

// =============== Implementação da API em C ================

bool qt_menu_init(qt_menu_context_t *context, QWidget *parent_window) {
  if (!context) {
    return false;
  }

  // Criar contexto nativo
  QtMenuSystem *nativeContext = new QtMenuSystem(parent_window);
  context->native_context = nativeContext;

  // Inicializar
  bool result = nativeContext->init();
  context->initialized = result;

  return result;
}

void qt_menu_shutdown(qt_menu_context_t *context) {
  if (!context || !context->native_context) {
    return;
  }

  // Finalizar contexto nativo
  QtMenuSystem *nativeContext =
      static_cast<QtMenuSystem *>(context->native_context);
  nativeContext->shutdown();

  // Limpar
  delete nativeContext;
  context->native_context = nullptr;
  context->initialized = false;
}

qt_menu_t *qt_menu_create(const char *title, qt_menu_t *parent) {
  // Criar estrutura para novo menu
  qt_menu_t *menu = new qt_menu_t();
  strncpy(menu->title, title, QT_MENU_MAX_TEXT_LENGTH - 1);
  menu->title[QT_MENU_MAX_TEXT_LENGTH - 1] = '\0';
  menu->parent = parent;
  menu->userdata = nullptr;
  menu->visible = true;

  // Criar menu nativo
  QtMenuSystem *system = nullptr;
  if (parent && parent->native_menu) {
    // Usar sistema do menu pai
    QtMenu *parentMenu = static_cast<QtMenu *>(parent->native_menu);
    system = qobject_cast<QtMenuSystem *>(parentMenu->parent());
  }

  if (system) {
    QtMenu *parentMenu = static_cast<QtMenu *>(parent->native_menu);
    QtMenu *nativeMenu = system->createMenu(QString(title), parentMenu);
    menu->native_menu = nativeMenu;
  } else {
    // Menu sem sistema ainda, criar apenas o QtMenu
    QtMenu *nativeMenu = new QtMenu(QString(title), nullptr);
    menu->native_menu = nativeMenu;
  }

  return menu;
}

void qt_menu_destroy(qt_menu_t *menu) {
  if (!menu) {
    return;
  }

  // Verificar se temos um sistema
  QtMenu *nativeMenu = static_cast<QtMenu *>(menu->native_menu);
  QtMenuSystem *system =
      nativeMenu ? qobject_cast<QtMenuSystem *>(nativeMenu->parent()) : nullptr;

  if (system) {
    // Deixar o sistema gerenciar a destruição
    system->destroyMenu(nativeMenu);
  } else {
    // Destruir diretamente
    delete nativeMenu;
  }

  // Limpar e liberar estrutura
  menu->native_menu = nullptr;
  delete menu;
}

int qt_menu_add_item(qt_menu_t *menu, const char *id, const char *text,
                     qt_menu_item_type_t type) {
  if (!menu || !menu->native_menu) {
    return -1;
  }

  QtMenu *nativeMenu = static_cast<QtMenu *>(menu->native_menu);
  return nativeMenu->addItem(QString(id), QString(text), type);
}

int qt_menu_add_action(qt_menu_t *menu, const char *id, const char *text,
                       void (*callback)(void *userdata)) {
  if (!menu || !menu->native_menu) {
    return -1;
  }

  QtMenu *nativeMenu = static_cast<QtMenu *>(menu->native_menu);

  // Converter callback C para std::function
  std::function<void(void *)> stdCallback = nullptr;
  if (callback) {
    stdCallback = [callback](void *data) { callback(data); };
  }

  return nativeMenu->addAction(QString(id), QString(text), stdCallback);
}

// Implementações restantes das funções da API C...
// (O restante segue o mesmo padrão, convertendo entre as APIs)
