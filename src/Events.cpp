#include "Events.h"
#include "Manager.h"
#include "CLibUtilsQTR/StringHelpers.hpp"

namespace {
    bool IsHotkeyEvent(const RE::BSFixedString& event_name) {
        return StringHelpers::includesString(event_name.data(), {"Hotkey"});
    }
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