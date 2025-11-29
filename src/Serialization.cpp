#include "Serialization.h"
#include "Manager.h"
#include "Settings.h"
#include "Utils.h"

[[nodiscard]] bool SaveLoadData::Save(SKSE::SerializationInterface* serializationInterface) {
    assert(serializationInterface);
    Locker locker(m_Lock);

    const auto numRecords = m_Data.size();
    if (!serializationInterface->WriteRecordData(numRecords)) {
        logger::error("Failed to save {} data records", numRecords);
        return false;
    }

    for (const auto& [lhs, rhs] : m_Data) {
        std::uint32_t formid = lhs.first;
        if (!serializationInterface->WriteRecordData(formid)) {
            logger::error("Failed to save formid {:x}", formid);
            return false;
        }

        const std::string editorid = lhs.second;
        Serialization::write_string(serializationInterface, editorid);

        if (!serializationInterface->WriteRecordData(rhs)) {
            logger::error("Failed to save hotkey {}", rhs);
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool SaveLoadData::Save(SKSE::SerializationInterface* serializationInterface, const std::uint32_t type,
                                      const std::uint32_t version) {
    if (!serializationInterface->OpenRecord(type, version)) {
        logger::error("Failed to open record for Data Serialization!");
        return false;
    }

    return Save(serializationInterface);
}

[[nodiscard]] bool SaveLoadData::Load(SKSE::SerializationInterface* serializationInterface,
                                      const unsigned int plugin_version) {
    assert(serializationInterface);

    if (plugin_version < 1) {
        logger::error("Plugin version is less than 0.1, skipping load.");
        return false;
    }

    std::size_t recordDataSize;
    serializationInterface->ReadRecordData(recordDataSize);
    logger::info("Loading data from serialization interface with size: {}", recordDataSize);

    Locker locker(m_Lock);
    m_Data.clear();

    for (size_t i = 0; i < recordDataSize; i++) {
        std::uint32_t formid = 0;

        if (!serializationInterface->ReadRecordData(formid)) {
            logger::error("Failed to read form ID");
            return false;
        }

        if (!serializationInterface->ResolveFormID(formid, formid)) {
            logger::error("Failed to resolve form ID, 0x{:X}.", formid);
            continue;
        }

        std::string editorid;
        if (!Serialization::read_string(serializationInterface, editorid)) {
            logger::error("Failed to read EditorID for FormID {:x}", formid);
            return false;
        }

        SaveDataLHS lhs({formid, editorid});

        if (plugin_version < 2) {
            m_Data[lhs] = -1;
        } else {
            SaveDataRHS rhs;
            if (!serializationInterface->ReadRecordData(rhs)) {
                logger::error("Failed to read hotkey");
                return false;
            }
            m_Data[lhs] = rhs;
        }

        logger::info("Loaded data for FormID {:x}, EditorID {}", formid, editorid);
    }

    return true;
}

void SaveCallback(SKSE::SerializationInterface* serializationInterface) {
    const auto M = Manager::GetSingleton();
    M->SendData();
    if (!M->Save(serializationInterface, Settings::kDataKey, Settings::kSerializationVersion)) {
        logger::critical("Failed to save Data");
    }
}

void LoadCallback(SKSE::SerializationInterface* serializationInterface) {
    logger::info("Loading Data from SKSE co-save.");

    const auto M = Manager::GetSingleton();
    M->Reset();

    std::uint32_t type;
    std::uint32_t version;
    std::uint32_t length;

    bool cosave_found = false;
    unsigned int received_version = 0;
    while (serializationInterface->GetNextRecordInfo(type, version, length)) {
        auto temp = Utils::DecodeTypeCode(type);
        if (!Settings::version_map.contains(version)) {
            logger::critical("Loaded data has incorrect version. Received ({}) - Expected ({}) for Data Key ({})",
                             version, Settings::kSerializationVersion, temp);
            continue;
        }

        switch (type) {
            case Settings::kDataKey: {
                received_version = Settings::version_map.at(version);
                if (!M->Load(serializationInterface, received_version))
                    logger::critical("Failed to Load Data for Manager");
                else
                    cosave_found = true;
            }
            break;
            default:
                logger::critical("Unrecognized Record Type: {}", temp);
                break;
        }
    }

    if (cosave_found) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << static_cast<float>(received_version) / 10.f;
        logger::info("Receiving Data from cosave with plugin version: {}.", oss.str());
        M->ReceiveData();
        logger::info("Data loaded from skse co-save.");
    } else
        logger::info("No cosave data found.");
}

void InitializeSerialization() {
    const auto serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID(Settings::kDataKey);
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);
}