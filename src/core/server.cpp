#include "../../include/core/server.hpp"


ServerCoreConfig serverConfig;
std::list<ClientSession> clients;
std::unordered_map<int, std::list<ClientSession>::iterator> fdToSession; // fd -> Client
std::unordered_map<uint32_t, std::list<ClientSession>::iterator> idToSession; // id -> Client

void sendChatSync(const ClientSession& session, uint32_t chatId) {
    int rc;
    Chat chat;

    // Basic data
    rc = getChatBasicData(serverConfig.db, chatId, chat);
    if(rc != SQLITE_ROW) {
        return;
    }
    sendNetMsg(session, [chat](){ return buildNetMsgSyncChatAdd(chat); });
    
    // Members
    rc = getChatMembers(serverConfig.db, chatId, chat);
    if(rc != SQLITE_DONE)
        return;
    for(const User& user: chat.members) {
        ChatRole role = chat.roles[user.id];
        sendNetMsg(session, [chatId, user, role](){ return buildNetMsgSyncMemberAdd(chatId, user, role); });
    }
    // Messages
    rc = getChatMessages(serverConfig.db, chatId, INT32_MAX, chat.messages);
    if(rc != SQLITE_DONE) {
        return;
    }
    for(const Message& message: chat.messages)
        sendNetMsg(session, [chatId = chat.id, message](){ return buildNetMsgSyncMessageAdd(chatId, message); });
}
void broadcastNetMsg(uint32_t chatId, NetworkMessage& netMsg) {
    int rc;
    std::vector<uint32_t> userIds;

    rc = getAllUserIdByChatId(serverConfig.db, chatId, userIds);
    if(rc != SQLITE_DONE)
        return;
    for(const uint32_t id: userIds) {
        auto it = idToSession.find(id);
        if(it == idToSession.end())
            continue;
        netMsgSend(it->second->connection, netMsg);
    }
}
