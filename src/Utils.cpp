#include "Utils.h"

#undef GetObject

namespace Utils {
    std::string DecodeTypeCode(const std::uint32_t typeCode) {
        char buf[4];
        buf[3] = static_cast<char>(typeCode);
        buf[2] = static_cast<char>(typeCode >> 8);
        buf[1] = static_cast<char>(typeCode >> 16);
        buf[0] = static_cast<char>(typeCode >> 24);
        return std::string(buf, buf + 4);
    }

    namespace MsgBoxesNotifs {
        namespace Windows {
            int Po3ErrMsg() {
                MessageBoxA(nullptr, po3_err_msgbox.c_str(), "Error", MB_OK | MB_ICONERROR);
                return 1;
            }
        };
    };

    bool FavoriteItem(RE::InventoryChanges* a_owner, const FormID a_itemID) {
        
        const auto& entries = a_owner->entryList;
        if (!entries) {
            logger::error("Owner's entry list is null");
            return false;
        }
        for (auto it = entries->begin(); it != entries->end(); ++it) {
            const auto inv_entry = *it;
            if (!inv_entry) {
                logger::error("Item entry is null");
                continue;
            }
            if (const auto object = inv_entry->GetObject(); object && object->GetFormID() == a_itemID) {
                a_owner->SetFavorite(inv_entry, nullptr);
                return true;
            }
        }
        return false;
    }
};