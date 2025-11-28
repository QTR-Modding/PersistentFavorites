#pragma once

namespace Hooks {
    void Install();

    inline std::array menu_open = {false, false, false, false}; // Container, Inventory, Favorites, Magic
    size_t GetMenuID(const RE::BSFixedString& menuName);
    bool IsAnyMenuOpen();
    inline bool IsMenuOpen(const size_t a_index) { return menu_open[a_index]; }

    template <typename MenuType>
    class MenuHook : public MenuType {
        using ProcessMessage_t = decltype(&MenuType::ProcessMessage);
        static inline REL::Relocation<ProcessMessage_t> _ProcessMessage;
        RE::UI_MESSAGE_RESULTS ProcessMessage_Hook(RE::UIMessage& a_message);

    public:
        static void InstallHook(const REL::VariantID& varID);
    };
}