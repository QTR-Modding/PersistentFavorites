#pragma once
#include "Manager.h"

namespace Hooks {
    void Install();

    inline bool is_menu_open = false;

    template <typename MenuType>
    class MenuHook : public MenuType {
        using ProcessMessage_t = decltype(&MenuType::ProcessMessage);
        static inline REL::Relocation<ProcessMessage_t> _ProcessMessage;
        RE::UI_MESSAGE_RESULTS ProcessMessage_Hook(RE::UIMessage& a_message);

    public:
        static void InstallHook(const REL::VariantID& varID);
    };

    template <typename RefType>
    class MoveItemHooks {
    public:
        static void install() {
            REL::Relocation<std::uintptr_t> _vtbl{RefType::VTABLE[0]};
            remove_item_ = _vtbl.write_vfunc(0x56, RemoveItem);
        }

    private:
        static RE::ObjectRefHandle* RemoveItem(RefType* a_this, RE::ObjectRefHandle& a_hidden_return_argument,
                                               RE::TESBoundObject* a_item, std::int32_t a_count,
                                               RE::ITEM_REMOVE_REASON a_reason, RE::ExtraDataList* a_extra_list,
                                               RE::TESObjectREFR* a_move_to_ref, const RE::NiPoint3* a_drop_loc,
                                               const RE::NiPoint3* a_rotate);
        static inline REL::Relocation<decltype(RemoveItem)> remove_item_;
    };
}