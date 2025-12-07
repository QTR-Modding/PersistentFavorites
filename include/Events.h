#pragma once
#include "ClibUtil/singleton.hpp"

class EventSink : public clib_util::singleton::ISingleton<EventSink>,
                  public RE::BSTEventSink<RE::TESContainerChangedEvent>,
                  public RE::BSTEventSink<RE::SpellsLearned::Event> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* a_event,
                                          RE::BSTEventSource<RE::TESContainerChangedEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(const RE::SpellsLearned::Event* a_event,
                                          RE::BSTEventSource<RE::SpellsLearned::Event>*) override;
};