#include "Hooks.h"
#include "Manager.h"

void Hooks::Install() {
    MenuHook<RE::ContainerMenu>::InstallHook(RE::VTABLE_ContainerMenu[0]);
    MenuHook<RE::InventoryMenu>::InstallHook(RE::VTABLE_InventoryMenu[0]);
    MenuHook<RE::FavoritesMenu>::InstallHook(RE::VTABLE_FavoritesMenu[0]);
    MenuHook<RE::MagicMenu>::InstallHook(RE::VTABLE_MagicMenu[0]);
}

bool Hooks::IsAnyMenuOpen() {
    return std::ranges::any_of(menu_open, [](const bool isOpen) { return isOpen; });
}

size_t Hooks::GetMenuID(const RE::BSFixedString& menuName) {
    if (menuName == RE::ContainerMenu::MENU_NAME) {
        return 0;
    }
    if (menuName == RE::InventoryMenu::MENU_NAME) {
        return 1;
    }
    if (menuName == RE::FavoritesMenu::MENU_NAME) {
        return 2;
    }
    if (menuName == RE::MagicMenu::MENU_NAME) {
        return 3;
    }

    return 0;
}

template <typename MenuType>
RE::UI_MESSAGE_RESULTS Hooks::MenuHook<MenuType>::ProcessMessage_Hook(RE::UIMessage& a_message) {
    const auto msg_type = static_cast<int>(a_message.type.get());
    if (msg_type != 3 && msg_type != 1) {
        return _ProcessMessage(this, a_message);
    }

    auto menu_id = GetMenuID(MenuType::MENU_NAME);
    menu_open[menu_id] = msg_type == 1;

    Manager::GetSingleton()->AddFavorites();

    return _ProcessMessage(this, a_message);
}

template <typename MenuType>
void Hooks::MenuHook<MenuType>::InstallHook(const REL::VariantID& varID) {
    REL::Relocation<std::uintptr_t> vTable(varID);
    _ProcessMessage = vTable.write_vfunc(0x4, &MenuHook<MenuType>::ProcessMessage_Hook);
}