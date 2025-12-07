#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

namespace logger = SKSE::log;
using namespace std::literals;

using FormID = RE::FormID;
using RefID = RE::FormID;
using Count = RE::TESObjectREFR::Count;

constexpr RefID player_refid = 20;