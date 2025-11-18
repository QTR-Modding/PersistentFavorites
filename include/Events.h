
#pragma once
#include "Manager.h"
#include "ClibUtil/singleton.hpp"

class myEventSink : public clib_util::singleton::ISingleton<myEventSink>,
                    public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
                    public RE::BSTEventSink<RE::TESContainerChangedEvent>, 
                    public RE::BSTEventSink<RE::InputEvent*>,
                    public RE::BSTEventSink<RE::SpellsLearned::Event> {

    Manager* M = Manager::GetSingleton();

    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* evns, RE::BSTEventSource<RE::InputEvent*>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* event,
                                          RE::BSTEventSource<RE::TESContainerChangedEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(const RE::SpellsLearned::Event* a_event,
                                          RE::BSTEventSource<RE::SpellsLearned::Event>*) override;

    static inline bool IsHotkeyEvent(const RE::BSFixedString& event_name);

public:

    void SaveCallback(SKSE::SerializationInterface* serializationInterface) const;

    void LoadCallback(SKSE::SerializationInterface* serializationInterface) const;

};