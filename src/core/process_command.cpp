#include "../../include/core/process_command.hpp"

// Source: https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
static std::vector<std::string> split(std::string s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);

    return tokens;
}

bool processCommandGetMembers(uint32_t chatId, std::vector<std::string> command) {
    if(command.size() != 1)
        return false;

    return true;
}
bool processCommandAddMember(uint32_t chatId, std::vector<std::string> command) {
    if(command.size() < 3)
        return false;
    ChatRole role;

    if(command[2] == "owner")
        role = ChatRole::OWNER;
    else if(command[2] == "admin")
        role = ChatRole::ADMIN;
    else if(command[2] == "member")
        role = ChatRole::MEMBER;
    else
        return false;
    sendNetMsg(clientConfig.connection, [chatId, username=command[1], role]{ return buildNetMsgRequestChatMemberAdd(chatId, username, role); });

    return true;
}
bool processCommandDelMember(uint32_t chatId, std::vector<std::string> command) {
    if(command.size() < 2)
        return false;
    sendNetMsg(clientConfig.connection, [chatId, username=command[1]]{ return buildNetMsgRequestChatMemberDel(chatId, username); });

    return true;
}
bool processCommandSetRole(uint32_t chatId, std::vector<std::string> command) {
    if(command.size() < 3)
        return false;
    ChatRole role;

    if(command[2] == "admin")
        role = ChatRole::ADMIN;
    else if(command[2] == "member")
        role = ChatRole::MEMBER;
    else
        return false;
    sendNetMsg(clientConfig.connection, [chatId, username=command[1], role]{ return buildNetMsgRequestChatMemberSet(chatId, username, role); });

    return true;
}
bool processCommandSetColor(uint32_t chatId, std::vector<std::string> command) {
    if(command.size() < 5)
        return false;
    ChatColorType colorType;
    uint32_t color;
    uint8_t chanel;

    color = 0;
    if(command[1] == "msgBg")
        colorType = ChatColorType::MESSAGE_BG;
    else if(command[1] == "msgFg")
        colorType = ChatColorType::MESSAGE_FG;
    else
        return false;

    try {
        chanel = std::stoi(command[2]);
        std::printf("Channel(%u,", chanel);
        if(chanel < 0 || chanel > 255)
            return false;
        color |= ((uint32_t)chanel) << 24;

        chanel = std::stoi(command[3]);
        std::printf("%u,", chanel);
        if(chanel < 0 || chanel > 255)
            return false;
        color |= ((uint32_t)chanel) << 16;

        chanel = std::stoi(command[4]);
        std::printf("%u)\n", chanel);
        if(chanel < 0 || chanel > 255)
            return false;
        color |= ((uint32_t)chanel) << 8;

        if(command.size() >= 6) {
            chanel = std::stoi(command[5]);
            if(chanel < 0 || chanel > 255)
                return false;
            color |= (uint32_t)chanel;
        }
        else
            color |= (uint32_t)0xFF;
    } catch(std::invalid_argument& err) {
        return false;
    }

    sendNetMsg(clientConfig.connection, [chatId, colorType, color]{ return buildNetMsgRequestChatColorSet(chatId, colorType, color); });
    return true;
}
bool processCommandSetTheme(uint32_t chatId, std::vector<std::string> command) {
    if(command.size() < 2)
        return false;
    ChatTheme chatTheme;

    if(command[1] == "DEFAULT")
        chatTheme = ChatTheme::DEFAULT;
    else if(command[1] == "GRUVBOX")
        chatTheme = ChatTheme::GRUVBOX;
    else if(command[1] == "GOL")
        chatTheme = ChatTheme::GOL;
    else if(command[1] == "MATRIX")
        chatTheme = ChatTheme::MATRIX;
    else if(command[1] == "HELLO_KITTY")
        chatTheme = ChatTheme::HELLO_KITTY;
    else
        return false;

    sendNetMsg(clientConfig.connection, [chatId, theme=static_cast<uint8_t>(chatTheme)]{ return buildNetMsgRequestChatThemeSet(chatId, theme); });
    return true;
}
bool processCommand(uint32_t chatId, std::string commandStr) {
    if(commandStr.empty() || commandStr[0] != '/')
        return false;
    std::vector<std::string> command;
    bool isCommandSent;

    isCommandSent = false;
    command = split(commandStr, " ");
    for(size_t i = 0; i < command.size(); i++) {
        std::printf("%s\n", command[i].c_str());
    }
    if(command[0] == "/addMember")
        isCommandSent = processCommandAddMember(chatId, command);
    else if(command[0] == "/delMember")
        isCommandSent = processCommandDelMember(chatId, command);
    else if(command[0] == "/setRole")
        isCommandSent = processCommandSetRole( chatId, command);
    else if(command[0] == "/setTheme")
        isCommandSent = processCommandSetTheme(chatId, command);
    else
        return false;
    if(isCommandSent)
        clientCoreState = ClientCoreState::WAIT_COMMAND_PROCESS;
    return true;
}
