#ifndef CORE_CHAT_HPP
#define CORE_CHAT_HPP

#include "user.hpp"
#include <string>
#include <unordered_map>
#include <vector>


enum class ChatRole: uint8_t {
    OWNER = 1,
    ADMIN,
    MEMBER
};
enum class ChatColorType: uint8_t {
    MESSAGE_BG = 1,
    MESSAGE_FG
};
enum class ChatTheme: uint8_t {
    DEFAULT = 0,
    GRUVBOX = 1,
    MATRIX,
    HELLO_KITTY,

    // Add new theme above
    GOL
};
extern uint8_t chatThemeLast;

std::string chatRoleToStr(ChatRole role);
struct Message {
    uint32_t id;
    uint32_t timestamp;
    User author;
    std::string content;
};
struct Chat {
    uint32_t id;
    std::string name;
    std::vector<User> members;
    std::unordered_map<uint32_t, ChatRole> roles;
    std::vector<Message> messages;
    ChatTheme theme;
};

bool hasRightToManage(ChatRole senderRole);
bool hasRightToChatDel(ChatRole senderRole);
bool hasRightToManageMember(ChatRole senderRole, ChatRole delUserRole);
bool hasRightToSetTheme(ChatRole senderRole);

#endif
