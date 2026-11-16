#include "Utils.h"

#undef GetObject

namespace Utils {
    std::string DecodeTypeCode(const std::uint32_t typeCode) {
        char buf[4];
        buf[3] = static_cast<char>(typeCode);
        buf[2] = static_cast<char>(typeCode >> 8);
        buf[1] = static_cast<char>(typeCode >> 16);
        buf[0] = static_cast<char>(typeCode >> 24);
        return std::string(buf, buf + 4);
    }

    namespace MsgBoxesNotifs {
        namespace Windows {
            int Po3ErrMsg() {
                MessageBoxA(nullptr, po3_err_msgbox.c_str(), "Error", MB_OK | MB_ICONERROR);
                return 1;
            }
        };
    };

    bool FavoriteItem(const RE::TESForm* a_item, const RE::TESObjectREFR::InventoryItemMap& inv) {
        const auto player = RE::PlayerCharacter::GetSingleton();
        if (const auto invChanges = player->GetInventoryChanges()) {
            for (const auto& [item, data] : inv) {
                const auto& [count, entry] = data;
                if (count > 0 && item == a_item) {
                    if (entry->IsFavorited()) {
                        return true;
                    }
                    const auto extralist = entry->extraLists && !entry->extraLists->empty()
                                               ? entry->extraLists->front()
                                               : nullptr;
                    invChanges->SetFavorite(entry.get(), extralist);
                    return true;
                }
            }
        }
        return false;
    }

    std::unordered_set<RE::TESForm*> FindUnregisteredMagicFavorites(const RE::MagicFavorites* magicFavorites) {
        std::unordered_set<RE::TESForm*> unregistered;
        unregistered.insert(magicFavorites->spells.begin(), magicFavorites->spells.end());
        unregistered.insert(magicFavorites->hotkeys.begin(), magicFavorites->hotkeys.end());
        unregistered.erase(nullptr);
        if (unregistered.empty()) {
            return unregistered;
        }

        const auto [forms, lock] = RE::TESForm::GetAllForms();
        const RE::BSReadLockGuard guard{ lock };
        if (forms) {
            for (const auto& entry : *forms) {
                if (unregistered.erase(entry.second) && unregistered.empty()) {
                    break;
                }
            }
        }
        return unregistered;
    }

    bool IsMagicMenuItem(const RE::TESForm* a_form) {
        const auto formType = a_form ? a_form->GetFormType() : RE::FormType::None;
        if (formType == RE::FormType::Spell || formType == RE::FormType::Shout) {
            return true;
        }
        return false;
    }
};