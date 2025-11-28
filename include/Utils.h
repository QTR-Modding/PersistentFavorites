#pragma once

namespace Utils {
    const auto mod_name = static_cast<std::string>(SKSE::PluginDeclaration::GetSingleton()->GetName());
    constexpr auto po3path = "Data/SKSE/Plugins/po3_Tweaks.dll";
    inline bool IsPo3Installed() { return std::filesystem::exists(po3path); };
    const auto po3_err_msgbox = std::format(
        "{}: You must have powerofthree's Tweaks "
        "installed. See mod page for further instructions.",
        mod_name);

    std::string DecodeTypeCode(std::uint32_t typeCode);

    bool FavoriteItem(RE::InventoryChanges* a_owner, FormID a_itemID);
    RE::InventoryEntryData* GetSelectedEntryInMenu();

    namespace MsgBoxesNotifs {
        namespace Windows {
            int Po3ErrMsg();
        };
    };
};