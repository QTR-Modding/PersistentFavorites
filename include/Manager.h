
#pragma once
#include <shared_mutex>
#include <unordered_set>
#include "Serialization.h"

class Manager final : public SaveLoadData, public RE::Actor::ForEachSpellVisitor {

    std::shared_mutex mutex_;

    std::unordered_set<FormID> favorites;
    std::unordered_map<FormID, int> hotkey_map;
    std::unordered_set<FormID> temp_all_spells;

    const std::set<int> allowed_hotkeys = {0,1,2,3,4,5,6,7};

    bool RemoveFavorite(FormID formid);

    int GetHotkey(const RE::InventoryEntryData* a_entry) const;

    [[nodiscard]] bool IsHotkeyValid(int hotkey) const;

    void UpdateHotkeyMap(FormID item_formid, const RE::InventoryEntryData* a_entry);
    void UpdateHotkeyMap(FormID spell_formid, int a_hotkey);

    [[nodiscard]] std::map<int, FormID> GetInventoryHotkeys() const;
    [[nodiscard]] std::map<FormID, int> GetMagicHotkeys() const;
    [[nodiscard]] std::map<int, FormID> GetHotkeysInUse() const;

    [[nodiscard]] FormID HotkeyIsInUse(FormID, int a_hotkey) const;

    void ApplyHotkey(FormID formid, const RE::TESObjectREFR::InventoryItemMap& inv);

    void SyncHotkeys_Item();

    void SyncHotkeys_Spell();

    void SyncHotkeys();

    RE::BSContainer::ForEachResult Visit(RE::SpellItem* a_spell) override;

    void CollectPlayerSpells();

    bool AddFavorites_Item();
    bool AddFavorites_Spell();
    void SyncFavorites_Item();
    void SyncFavorites_Spell();
    void FavoriteCheck_Spell(FormID formid);
public:
    static Manager* GetSingleton() {
        static Manager singleton;
        return &singleton;
    }

    bool AddFavorites();
    void SyncFavorites();
    void FavoriteCheck_Item(FormID formid);
    void FavoriteCheck_Spell();

    void Reset();
    void SendData();
    void ReceiveData();

};