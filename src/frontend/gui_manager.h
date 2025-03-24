#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <memory>
#include <string>
#include "gui/gui_types.h" // Include gui_types.h

namespace MegaEmu::Frontend
{

    class GuiBackend; // Forward declaration

    enum class GuiType
    {
        PLAYER,
        DESIGNER,
        SIMPLE
    };

    class GuiManager
    {
    public:
        GuiManager();
        ~GuiManager();

        bool initialize(GuiType type);
        void shutdown();
        void update();
        void render();

    private:
        std::unique_ptr<GuiBackend> guiBackend_;
        GuiType currentGuiType_;
    };

} // namespace MegaEmu::Frontend

#endif // GUI_MANAGER_H
