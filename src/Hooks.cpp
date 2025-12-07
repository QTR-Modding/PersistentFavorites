#include "Hooks.h"
#include "Manager.h"

void Hooks::Install() {
    MenuHook<RE::ContainerMenu>::InstallHook(RE::VTABLE_ContainerMenu[0]);
    MenuHook<RE::InventoryMenu>::InstallHook(RE::VTABLE_InventoryMenu[0]);
    MenuHook<RE::FavoritesMenu>::InstallHook(RE::VTABLE_FavoritesMenu[0]);
    MenuHook<RE::MagicMenu>::InstallHook(RE::VTABLE_MagicMenu[0]);
}

template <typename MenuType>
RE::UI_MESSAGE_RESULTS Hooks::MenuHook<MenuType>::ProcessMessage_Hook(RE::UIMessage& a_message) {
    const auto msg_type = static_cast<int>(a_message.type.get());
    if (msg_type != 3 && msg_type != 1) {
        return _ProcessMessage(this, a_message);
    }

    if (msg_type == 1) {
        Manager::GetSingleton()->AddFavorites();
    } else {
        Manager::GetSingleton()->SyncFavorites();
    }

    return _ProcessMessage(this, a_message);
}

template <typename MenuType>
void Hooks::MenuHook<MenuType>::InstallHook(const REL::VariantID& varID) {
    REL::Relocation<std::uintptr_t> vTable(varID);
    _ProcessMessage = vTable.write_vfunc(0x4, &MenuHook<MenuType>::ProcessMessage_Hook);
}