#include "Events.h"
#include "Hooks.h"
#include "Logger.h"
#include "Manager.h"
#include "Utils.h"

namespace {
    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    void OnMessage(SKSE::MessagingInterface::Message* message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            const auto eventSink = EventSink::GetSingleton();
            auto* eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
            eventSourceHolder->AddEventSink<RE::TESContainerChangedEvent>(eventSink);
            const auto spellsource = RE::SpellsLearned::GetEventSource();
            spellsource->AddEventSink(eventSink);
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {

    SetupLog();
    SKSE::Init(skse);
    InitializeSerialization();
    Hooks::Install();
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    if (!Utils::IsPo3Installed()) {
        logger::error("Po3 is not installed.");
        Utils::MsgBoxesNotifs::Windows::Po3ErrMsg();
        return false;
    }
    logger::info("Plugin loaded");
    return true;
}