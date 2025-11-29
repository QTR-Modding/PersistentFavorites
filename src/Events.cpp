#include "Events.h"
#include "Hooks.h"
#include "Manager.h"
#include "Utils.h"
#include "CLibUtilsQTR/StringHelpers.hpp"

namespace {
    bool IsHotkeyEvent(const RE::BSFixedString& event_name) {
        return StringHelpers::includesString(event_name.data(), {"Hotkey"});
    }
}


RE::BSEventNotifyControl EventSink::ProcessEvent(RE::InputEvent* const* evns, RE::BSTEventSource<RE::InputEvent*>*) {
    if (!Hooks::IsAnyMenuOpen()) return RE::BSEventNotifyControl::kContinue;
    if (!*evns) return RE::BSEventNotifyControl::kContinue;
    for (RE::InputEvent* e = *evns; e; e = e->next) {
        if (const RE::ButtonEvent* a_event = e->AsButtonEvent()) {
            if (!a_event->IsUp()) continue;
            const RE::IDEvent* id_event = e->AsIDEvent();
            const auto& user_event = id_event->userEvent;
            const auto user_events = RE::UserEvents::GetSingleton();
            if (IsHotkeyEvent(user_event) && Hooks::IsMenuOpen(2)) {
                // favorites menu
                if (const auto selectedItem = Utils::GetSelectedEntryInMenu()) {
                    Manager::GetSingleton()->UpdateFavorite(selectedItem);
                }
                return RE::BSEventNotifyControl::kContinue;
            }
            if (user_event == user_events->toggleFavorite || user_event == user_events->yButton) {
                if (Hooks::IsMenuOpen(3)) {
                    // magic menu
                    Manager::GetSingleton()->SyncFavorites(true);
                } 
                else if (REL::Module::IsVR()) {
                    Manager::GetSingleton()->SyncFavorites(false);
                } 
                else if (const auto selectedItem = Utils::GetSelectedEntryInMenu()) {
                    Manager::GetSingleton()->UpdateFavorite(selectedItem);
                } else {
                    logger::warn("No selected item in menu for favorite toggle event: {}", user_event.c_str());
                }
                return RE::BSEventNotifyControl::kContinue;
            }
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl EventSink::ProcessEvent(const RE::TESContainerChangedEvent* a_event,
                                                 RE::BSTEventSource<RE::TESContainerChangedEvent>*) {
    if (!a_event) return RE::BSEventNotifyControl::kContinue;
    if (a_event->newContainer != player_refid) return RE::BSEventNotifyControl::kContinue;

    Manager::GetSingleton()->FavoriteCheck_Item(a_event->baseObj);

    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl EventSink::ProcessEvent(const RE::SpellsLearned::Event* a_event,
                                                 RE::BSTEventSource<RE::SpellsLearned::Event>*) {
    if (!a_event) return RE::BSEventNotifyControl::kContinue;
    Manager::GetSingleton()->FavoriteCheck_Spell();
    return RE::BSEventNotifyControl::kContinue;
}