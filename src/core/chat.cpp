#include "../../include/core/chat.hpp"

uint8_t chatThemeLast = static_cast<uint8_t>(ChatTheme::GOL);

std::string chatRoleToStr(ChatRole role) {
    switch(role) {
        case ChatRole::OWNER:
            return "OWNER";
        case ChatRole::ADMIN:
            return "ADMIN";
        case ChatRole::MEMBER:
            return "MEMBER";
        default:
            return "";
    }
    return "";
}

bool hasRightToManage(ChatRole senderRole) {
    uint8_t senderRoleU8;
    uint8_t adminRoleU8;

    senderRoleU8 = static_cast<uint8_t>(senderRole);
    adminRoleU8 = static_cast<uint8_t>(ChatRole::ADMIN);
    if(senderRoleU8 > adminRoleU8)
        return false;
    return true;
}
bool hasRightToChatDel(ChatRole senderRole) {
    return senderRole == ChatRole::OWNER;
}
bool hasRightToManageMember(ChatRole senderRole, ChatRole delUserRole) {
    uint8_t senderRoleU8;
    uint8_t delUserRoleU8;

    senderRoleU8 = static_cast<uint8_t>(senderRole);
    delUserRoleU8 = static_cast<uint8_t>(delUserRole);
    if(senderRoleU8 >= delUserRoleU8)
        return false;
    return true;
}
bool hasRightToSetTheme(ChatRole senderRole) {
    return hasRightToManage(senderRole);
}
