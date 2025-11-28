#include "Manager.h"
#include "Settings.h"
#include "Utils.h"
#include "ClibUtilsQTR/FormReader.hpp"

namespace {
    bool IsSpellFavorited(const FormID a_spell, const RE::BSTArray<RE::TESForm*>& favs) {
        for (auto& fav : favs) {
            if (!fav) continue;
            if (fav->GetFormID() == a_spell) return true;
        }
        return false;
    }

    void HotkeySpell(RE::TESForm* form, const int hotkey) {
        const auto magic_favs = RE::MagicFavorites::GetSingleton();
        if (!magic_favs) {
            logger::error("HotkeySpell: MagicFavorites is null.");
            return;
        }
        magic_favs->SetFavorite(form);
        auto& hotkeys = magic_favs->hotkeys;
        int index = 0;
        for (auto& hotkeyed_spell : hotkeys) {
            if (!hotkeyed_spell && index == hotkey) {
                hotkeyed_spell = form;
                return;
            }
            if (hotkeyed_spell && hotkeyed_spell->GetFormID() == form->GetFormID()) {
                return;
            }
            index++;
        }
        logger::error("HotkeySpell: Failed to set hotkey. FormID: {:x}, Hotkey: {}", form->GetFormID(), hotkey);
    }
}


bool Manager::RemoveFavorite(const FormID formid) {
    const auto removed = favorites.erase(formid) > 0;
    hotkey_map.erase(formid);
    return removed;
};

int Manager::GetHotkey(const RE::InventoryEntryData* a_entry) const {
    if (!a_entry) {
        logger::warn("GetHotkey: Entry is null.");
        return -1;
    }
    if (!a_entry->IsFavorited()) {
        logger::warn("GetHotkey: Entry is not favorited.");
        return -1;
    }
    if (!a_entry->extraLists || a_entry->extraLists->empty()) {
        logger::warn("GetHotkey: Entry has no extraLists.");
        return -1;
    }
    for (const auto& extraList : *a_entry->extraLists) {
        if (!extraList) continue;
        if (extraList->HasType(RE::ExtraDataType::kHotkey)) {
            const auto extra_hotkey = extraList->GetByType<RE::ExtraHotkey>();
            if (!extra_hotkey) continue;
            const auto hotkey = extraList->GetByType<RE::ExtraHotkey>()->hotkey.underlying();
            if (!IsHotkeyValid(hotkey)) continue;
            return hotkey;
        }
    }
    return -1;
}

inline bool Manager::IsHotkeyValid(const int hotkey) const {
    return allowed_hotkeys.contains(hotkey);
}

void Manager::UpdateHotkeyMap(const FormID item_formid, const RE::InventoryEntryData* a_entry) {
    const auto hotkey = GetHotkey(a_entry);
    if (IsHotkeyValid(hotkey)) {
        hotkey_map[item_formid] = hotkey;
    }
}

void Manager::UpdateHotkeyMap(const FormID spell_formid, const int a_hotkey) {
    if (IsHotkeyValid(a_hotkey)) {
        hotkey_map[spell_formid] = a_hotkey;
    }
}

std::map<int, FormID> Manager::GetInventoryHotkeys() const {
    std::map<int, FormID> hotkeys_in_use;
    const auto player_inventory = RE::PlayerCharacter::GetSingleton()->GetInventory();
    for (auto& item : player_inventory) {
        if (!item.first) continue;
        if (item.second.first <= 0) continue;
        if (std::strlen(item.first->GetName()) == 0) continue;
        if (!item.second.second) continue;
        if (!item.second.second->extraLists || item.second.second->extraLists->empty()) continue;
        if (!item.second.second->IsFavorited()) continue;
        const auto hotkey_temp = GetHotkey(item.second.second.get());
        if (IsHotkeyValid(hotkey_temp)) {
            hotkeys_in_use[hotkey_temp] = item.first->GetFormID();
        }
    }
    return hotkeys_in_use;
}

std::map<FormID, int> Manager::GetMagicHotkeys() const {
    std::map<FormID, int> hotkeys_in_use;
    const auto& mg_hotkeys = RE::MagicFavorites::GetSingleton()->hotkeys;
    int index = 0;
    for (auto& hotkeyed_spell : mg_hotkeys) {
        if (!hotkeyed_spell) {
            index++;
            continue;
        }
        if (IsHotkeyValid(index)) {
            hotkeys_in_use[hotkeyed_spell->GetFormID()] = index;
        }
        index++;
    }
    return hotkeys_in_use;
};

std::map<int, FormID> Manager::GetHotkeysInUse() const {
    const auto& inventory_hotkeys = GetInventoryHotkeys();
    const auto& magic_hotkeys = GetMagicHotkeys();
    std::map<int, FormID> hotkeys_in_use;
    for (const auto& [hotkey, form_id] : inventory_hotkeys) {
        if (IsHotkeyValid(hotkey)) hotkeys_in_use[hotkey] = form_id;
    }
    for (const auto& [form_id,hotkey] : magic_hotkeys) {
        if (IsHotkeyValid(hotkey)) hotkeys_in_use[hotkey] = form_id;
    }
    return hotkeys_in_use;
};

FormID Manager::HotkeyIsInUse(const FormID formid, const int a_hotkey) const {
    if (!IsHotkeyValid(a_hotkey)) {
        logger::error("Hotkey invalid. Hotkey: {}", a_hotkey);
        return formid;
    }
    const auto hotkeys_temp = GetHotkeysInUse();
    if (hotkeys_temp.contains(a_hotkey)) {
        const auto used_by = hotkeys_temp.at(a_hotkey);
        return used_by;
    }
    return formid;
}

void Manager::ApplyHotkey(const FormID formid) {
    if (!formid) return;
    if (!favorites.contains(formid)) {
        return;
    }
    if (!hotkey_map.contains(formid)) {
        return;
    }
    const auto hotkey = hotkey_map.at(formid);
    if (!IsHotkeyValid(hotkey)) {
        logger::error("Hotkey invalid. FormID: {:x}, Hotkey: {}", formid, hotkey);
        hotkey_map.erase(formid);
        return;
    }
    if (const auto used_by = HotkeyIsInUse(formid, hotkey); used_by != formid) {
        hotkey_map.erase(formid);
        hotkey_map[used_by] = hotkey;
        return;
    }
    const auto spell = FormReader::GetFormByID<RE::SpellItem>(formid);
    if (spell && RE::PlayerCharacter::GetSingleton()->HasSpell(spell)) {
        HotkeySpell(spell, hotkey);
        return;
    }

    // Spell ended

    // Now items
    const auto bound = FormReader::GetFormByID<RE::TESBoundObject>(formid);
    if (!bound) {
        logger::error("ApplyHotkey: Form not found. FormID: {:x}", formid);
        return;
    }
    const auto player_inventory = RE::PlayerCharacter::GetSingleton()->GetInventory();
    const auto item = player_inventory.find(bound);
    if (item == player_inventory.end()) {
        return;
    }
    if (!item->second.second || !item->second.second->extraLists || item->second.second->extraLists->empty()) {
        logger::error("ApplyHotkey: InventoryEntryData or extraLists invalid. FormID: {:x}", formid);
        return;
    }
    auto* xList = item->second.second->extraLists->front();
    if (!xList) {
        logger::error("ApplyHotkey: ExtraList is null. FormID: {:x}", formid);
        return;
    }
    if (xList->HasType(RE::ExtraDataType::kHotkey)) {
        if (const auto old_xHotkey = xList->GetByType<RE::ExtraHotkey>()) {
            old_xHotkey->hotkey = static_cast<RE::ExtraHotkey::Hotkey>(hotkey);
        }
    } else {
        RE::ExtraHotkey* xHotkey = RE::ExtraHotkey::Create<RE::ExtraHotkey>();
        if (!xHotkey) {
            logger::error("ApplyHotkey: Failed to create hotkey. FormID: {:x}", formid);
            return;
        }
        xHotkey->hotkey = static_cast<RE::ExtraHotkey::Hotkey>(hotkey);
        if (static_cast<uint8_t>(xHotkey->hotkey.get()) != hotkey) {
            logger::error("ApplyHotkey: Failed to set hotkey. FormID: {:x}, Hotkey: {}", formid, hotkey);
            delete xHotkey;
            return;
        }
        xList->Add(xHotkey);
    }
    const int added_hotkey = GetHotkey(item->second.second.get());
    if (added_hotkey != static_cast<int>(hotkey)) {
        logger::error("ApplyHotkey: Failed to add hotkey. FormID: {:x}, Hotkey: {}, added:{}", formid, hotkey,
                      added_hotkey);
    }
}

void Manager::SyncHotkeys_Item() {
    const auto player_inventory = RE::PlayerCharacter::GetSingleton()->GetInventory();
    for (auto& item : player_inventory) {
        if (!item.first) continue;
        if (item.second.first <= 0) continue;
        if (std::strlen(item.first->GetName()) == 0) continue;
        if (!item.second.second) continue;
        if (!item.second.second->extraLists || item.second.second->extraLists->empty()) continue;
        if (!item.second.second->IsFavorited()) continue;
        UpdateHotkeyMap(item.first->GetFormID(), item.second.second.get());
    }
}

void Manager::SyncHotkeys_Spell() {
    const auto& mg_favorites = RE::MagicFavorites::GetSingleton()->spells;
    const auto mg_hotkeys = GetMagicHotkeys();
    for (auto& spell : mg_favorites) {
        if (!spell) continue;
        if (std::strlen(spell->GetName()) == 0) continue;
        if (!mg_hotkeys.contains(spell->GetFormID())) continue;
        const auto spell_formid = spell->GetFormID();
        UpdateHotkeyMap(spell_formid, mg_hotkeys.at(spell_formid));
    }
}

void Manager::SyncHotkeys() {
    std::unique_lock lock(mutex_);
    SyncHotkeys_Item();
    SyncHotkeys_Spell();
}

RE::BSContainer::ForEachResult Manager::Visit(RE::SpellItem* a_spell) {
    if (!a_spell || !a_spell->GetPlayable()) return RE::BSContainer::ForEachResult::kContinue;
    if (std::strlen(a_spell->GetName()) == 0) return RE::BSContainer::ForEachResult::kContinue;
    temp_all_spells.insert(a_spell->GetFormID());
    return RE::BSContainer::ForEachResult::kContinue;
}

void Manager::CollectPlayerSpells() {
    temp_all_spells.clear();
    const auto player = RE::PlayerCharacter::GetSingleton();
    player->VisitSpells(*this);
}

bool Manager::AddFavorites_Item() {
    bool favorited_any = false;

    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto inv_changes = player->GetInventoryChanges();

    if (!inv_changes) {
        logger::error("Player Inventory Changes is null!");
        return false;
    }

    for (const auto player_inventory = player->GetInventory();
         const auto& [fst, snd] : player_inventory) {
        if (!fst) continue;
        if (snd.first <= 0) continue;
        if (!fst->GetPlayable()) continue;
        if (std::strlen(fst->GetName()) == 0) continue;
        if (!snd.second.get()) continue;
        const auto a_formid = fst->GetFormID();
        if (snd.second->IsFavorited()) {
            if (favorites.insert(a_formid).second) {
            }
            UpdateHotkeyMap(a_formid, snd.second.get());
        } else if (favorites.contains(a_formid)) {
            favorited_any |= Utils::FavoriteItem(inv_changes, a_formid);
            ApplyHotkey(a_formid);
        }
    }

    return favorited_any;
}

bool Manager::AddFavorites_Spell() {
    bool favorited_any = false;

    const auto mg_favorites = RE::MagicFavorites::GetSingleton();
    const auto& favorited_spells = mg_favorites->spells;
    const auto hotkeyed_spells = GetMagicHotkeys();

    CollectPlayerSpells();

    if (temp_all_spells.empty()) {
        return false;
    }
    for (auto& spell_formid : temp_all_spells) {
        const auto spell = FormReader::GetFormByID(spell_formid);
        if (!spell) continue;
        if (IsSpellFavorited(spell_formid, favorited_spells)) {
            if (favorites.insert(spell_formid).second) {
            }
            if (hotkeyed_spells.contains(spell_formid)) UpdateHotkeyMap(spell_formid, hotkeyed_spells.at(spell_formid));
        } else if (favorites.contains(spell_formid)) {
            mg_favorites->SetFavorite(spell);
            ApplyHotkey(spell_formid);
            favorited_any = true;
        }
    }
    temp_all_spells.clear();

    return favorited_any;
}

bool Manager::AddFavorites() {
    std::unique_lock lock(mutex_);
    const auto added_1 = AddFavorites_Item();
    const auto added_2 = AddFavorites_Spell();
    return added_1 || added_2;
}

void Manager::SyncFavorites_Item() {
    const auto player_inventory = RE::PlayerCharacter::GetSingleton()->GetInventory();
    for (const auto& [fst, snd] : player_inventory) {
        if (!fst) continue;
        if (snd.first <= 0) continue;
        if (!fst->GetPlayable()) continue;
        if (std::strlen(fst->GetName()) == 0) continue;
        if (!snd.second) continue;
        if (snd.second->IsFavorited()) {
            if (favorites.insert(fst->GetFormID()).second) {
            }
            UpdateHotkeyMap(fst->GetFormID(), snd.second.get());
        } else if (RemoveFavorite(fst->GetFormID())) {
        }
    }
}

void Manager::SyncFavorites_Spell() {
    const auto& favorited_spells = RE::MagicFavorites::GetSingleton()->spells;
    const auto hotkeyed_spells = GetMagicHotkeys();
    CollectPlayerSpells();
    if (temp_all_spells.empty()) {
        logger::warn("SyncFavorites: No spells found.");
        return;
    }
    for (auto& spell_formid : temp_all_spells) {
        const auto spell = FormReader::GetFormByID(spell_formid);
        if (!spell) continue;
        if (IsSpellFavorited(spell_formid, favorited_spells)) {
            if (favorites.insert(spell_formid).second) {
            }
            if (hotkeyed_spells.contains(spell_formid)) UpdateHotkeyMap(spell_formid, hotkeyed_spells.at(spell_formid));
        } else if (RemoveFavorite(spell_formid)) {
        }
    }
    temp_all_spells.clear();
};

void Manager::SyncFavorites() {
    std::unique_lock lock(mutex_);
    SyncFavorites_Item();
    SyncFavorites_Spell();
}

void Manager::FavoriteCheck_Item(const FormID formid) {
    std::unique_lock lock(mutex_);
    if (!favorites.contains(formid)) return;
    if (const auto inv_changes = RE::PlayerCharacter::GetSingleton()->GetInventoryChanges()) {
        Utils::FavoriteItem(inv_changes, formid);
        ApplyHotkey(formid);
    }
}

void Manager::FavoriteCheck_Spell(const FormID formid) {
    if (!favorites.contains(formid)) {
        return;
    }
    const auto spell = FormReader::GetFormByID(formid);
    if (!spell) {
        logger::warn("FavoriteCheck_Spell: Form not found. FormID: {}", formid);
        RemoveFavorite(formid);
        return;
    }
    RE::MagicFavorites::GetSingleton()->SetFavorite(spell);
    ApplyHotkey(formid);
}

void Manager::FavoriteCheck_Spell() {
    std::unique_lock lock(mutex_);
    CollectPlayerSpells();
    if (temp_all_spells.empty()) {
        logger::warn("FavoriteCheck_Spell: No spells found.");
        return;
    }
    for (auto& spell_formid : temp_all_spells) {
        FavoriteCheck_Spell(spell_formid);
    }
    temp_all_spells.clear();
}

void Manager::Reset() {
    logger::info("Resetting manager...");
    std::unique_lock lock(mutex_);
    favorites.clear();
    hotkey_map.clear();
    temp_all_spells.clear();
    Clear();
    logger::info("Manager reset.");
}

void Manager::SendData() {
    logger::info("--------Sending data---------");
    Clear();

    std::shared_lock lock(mutex_);
    int n_instances = 0;
    for (auto& fav_id : favorites) {
        const auto temp_form = FormReader::GetFormByID(fav_id);
        if (!temp_form) continue;
        const auto temp_editorid = clib_util::editorID::get_editorID(temp_form);
        if (temp_editorid.empty()) {
            logger::error("SendData: EditorID is empty. FormID: {:x}", fav_id);
            continue;
        }
        const SaveDataLHS lhs({fav_id, temp_editorid});
        const int rhs = hotkey_map.contains(fav_id) && IsHotkeyValid(hotkey_map.at(fav_id))
                            ? hotkey_map.at(fav_id)
                            : -1;
        SetData(lhs, rhs);
        n_instances++;
        if (n_instances >= Settings::instance_limit) {
            logger::warn("SendData: Instance limit reached. Number of instances: {}", n_instances);
            break;
        }
    }
    logger::info("Data sent. Number of instances: {}", n_instances);
}

void Manager::ReceiveData() {
    logger::info("--------Receiving data---------");
    if (m_Data.empty()) {
        logger::warn("ReceiveData: No data to receive.");
        return;
    }

    std::unique_lock lock(mutex_);
    int n_instances = 0;
    for (const auto& [lhs, rhs] : m_Data) {
        auto source_formid = lhs.first;
        auto source_editorid = lhs.second;

        if (source_editorid.empty()) {
            logger::error("ReceiveData: EditorID is empty.");
            continue;
        }
        const auto source_form = FormReader::GetFormByID(0, source_editorid);
        if (!source_form) {
            logger::critical("ReceiveData: Source form not found. Saved formid: {}, editorid: {}", source_formid,
                             source_editorid);
            continue;
        }
        if (source_form->GetFormID() != source_formid) {
            logger::warn("ReceiveData: Source formid does not match. Saved formid: {}, editorid: {}", source_formid,
                         source_editorid);
            source_formid = source_form->GetFormID();
        }

        if (favorites.contains(source_formid)) {
            logger::warn("ReceiveData: Form already favorited. FormID: {}, EditorID: {}", source_formid,
                         source_editorid);
            continue;
        }
        favorites.insert(source_formid);

        if (IsHotkeyValid(rhs)) hotkey_map[source_formid] = rhs;

        n_instances++;
    }

    lock.unlock();
    SyncHotkeys();

    logger::info("Data received. Number of instances: {}", n_instances);
};