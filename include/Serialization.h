#pragma once
#include "CLibUtilsQTR/Serialization.hpp"

using SaveDataLHS = std::pair<RE::FormID, std::string>;
using SaveDataRHS = int;


class SaveLoadData : public Serialization::BaseData<SaveDataLHS, SaveDataRHS> {
protected:
    ~SaveLoadData() = default;

public:
    [[nodiscard]] bool Save(SKSE::SerializationInterface* serializationInterface) override;

    [[nodiscard]] bool Save(SKSE::SerializationInterface* serializationInterface, std::uint32_t type,
                            std::uint32_t version) override;

    [[nodiscard]] bool Load(SKSE::SerializationInterface* serializationInterface, unsigned int plugin_version);
    bool Load(SKSE::SerializationInterface*, const bool) override { return false; }
    const char* GetType() override { return "SaveLoadData"; }
};

void SaveCallback(SKSE::SerializationInterface* serializationInterface);

void LoadCallback(SKSE::SerializationInterface* serializationInterface);


void InitializeSerialization();