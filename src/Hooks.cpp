#include "Hooks.h"
#include "Manager.h"

void Hooks::Install() {
    MenuHook<RE::ContainerMenu>::InstallHook(RE::VTABLE_ContainerMenu[0]);
    MenuHook<RE::InventoryMenu>::InstallHook(RE::VTABLE_InventoryMenu[0]);
    MenuHook<RE::FavoritesMenu>::InstallHook(RE::VTABLE_FavoritesMenu[0]);
    MenuHook<RE::MagicMenu>::InstallHook(RE::VTABLE_MagicMenu[0]);

    MoveItemHooks<RE::PlayerCharacter>::install();
}

template <typename MenuType>
RE::UI_MESSAGE_RESULTS Hooks::MenuHook<MenuType>::ProcessMessage_Hook(RE::UIMessage& a_message) {
    const auto msg_type = static_cast<int>(a_message.type.get());

    if (msg_type == 1) {
        is_menu_open = true;
        Manager::GetSingleton()->AddFavorites();
    }
    if (msg_type == 3) {
        is_menu_open = false;
        Manager::GetSingleton()->SyncFavorites();
    }

    return _ProcessMessage(this, a_message);
}

template <typename MenuType>
void Hooks::MenuHook<MenuType>::InstallHook(const REL::VariantID& varID) {
    REL::Relocation<std::uintptr_t> vTable(varID);
    _ProcessMessage = vTable.write_vfunc(0x4, &MenuHook<MenuType>::ProcessMessage_Hook);
}

template <typename RefType>
RE::ObjectRefHandle* Hooks::MoveItemHooks<RefType>::RemoveItem(
    RefType* a_this, RE::ObjectRefHandle& a_hidden_return_argument, RE::TESBoundObject* a_item, std::int32_t a_count,
    RE::ITEM_REMOVE_REASON a_reason, RE::ExtraDataList* a_extra_list, RE::TESObjectREFR* a_move_to_ref,
    const RE::NiPoint3* a_drop_loc, const RE::NiPoint3* a_rotate) {
    if (!is_menu_open || !a_item || a_count <= 0) {
        return remove_item_(a_this, a_hidden_return_argument, a_item, a_count, a_reason, a_extra_list, a_move_to_ref,
                            a_drop_loc, a_rotate);
    }

    Manager::GetSingleton()->UpdateFavorite(a_item);

    return remove_item_(a_this, a_hidden_return_argument, a_item, a_count, a_reason, a_extra_list, a_move_to_ref,
                        a_drop_loc, a_rotate);
}