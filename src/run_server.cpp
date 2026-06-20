#include "../include/run_server.hpp"

void DEBUG_createUsers() {
    insertUser(serverConfig.db, "test", "test");
    insertUser(serverConfig.db, "test1", "test");
    insertUser(serverConfig.db, "test2", "test");
    insertUser(serverConfig.db, "test3", "test");
    insertUser(serverConfig.db, "test4", "test");
    insertUser(serverConfig.db, "test5", "test");
}

void writeMessageAdmin(uint32_t chatId, std::string content) {
    std::string name;
    int rc;
    User user;
    Message message;

    // Setup data
    user = User{1, "SERVER"};
    message = Message{0, getTimestampNow(), user, content};
    // Insert in db
    rc = insertChatMessage(serverConfig.db, chatId, 1, message.content, message.timestamp);
    if(rc != SQLITE_DONE)
        return;
    // Broadcast message
    NetworkMessage netMsg = buildNetMsgSyncMessageAdd(chatId, message);
    netMsgWriteSize(netMsg);
    broadcastNetMsg(chatId, netMsg);
}

/* Handle client message */
// Auth request
void handleClientMessageSignIn(ClientSession &session, NetworkMessage &netMsg) {
    std::string name;
    std::string password;

    uint8_t *payload;
    uint16_t offset;
    int rc;

    uint32_t userId;

    payload = netMsg.payload;
    offset = 0;
    // Parse data
    name = packetReadString(payload, &offset, true);
    password = packetReadString(payload, &offset);
    // Insert in db
    rc = insertUser(serverConfig.db, name, password);
    userId = sqlite3_last_insert_rowid(serverConfig.db);
    // Answer the user
    if(rc == SQLITE_DONE) {
        sendNetMsg(session, [userId]() { return buildNetMsgAnswerSignInSuccessful(userId); });
        session.username = name;
        session.userId = userId;
        idToSession[userId] = fdToSession[session.connection.fd];
    } else if(rc == SQLITE_CONSTRAINT_UNIQUE)
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::SIGNIN, NMStatus::USER_EXIST); });
    else
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::SIGNIN, NMStatus::UNEXPECTED); });
}
void handleClientMessageLogin(ClientSession &session, NetworkMessage &netMsg) {
    bool isUserValid;
    std::string name;
    std::string password;

    uint8_t *payload;
    uint16_t offset;
    int rc;

    uint32_t userId;
    std::vector<uint32_t> chatIds;

    payload = netMsg.payload;
    offset = 0;
    // Parse data
    name = packetReadString(payload, &offset, true);
    password = packetReadString(payload, &offset);
    // Query the db
    isUserValid = checkUser(serverConfig.db, name, password, &userId);
    // Answer the user
    if(!isUserValid) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::LOGIN, NMStatus::USER_NOT_IN_DB); });
        return;
    }
    // Check if user not already connected
    if(idToSession.find(userId) != idToSession.end()) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::LOGIN, NMStatus::USER_EXIST); });
        return;
    }
    session.userId = userId;
    session.username = name;
    idToSession[userId] = fdToSession[session.connection.fd];
    sendNetMsg(session, [userId]() { return buildNetMsgAnswerLoginSuccessful(userId); });

    rc = getAllChatIdByUserId(serverConfig.db, userId, chatIds);
    for(uint32_t id : chatIds)
        sendChatSync(session, id);
}
// Chat request
// // Create/delete/modify a chat
void handleClientMessageChatAdd(ClientSession &session, NetworkMessage &netMsg) {
    uint8_t theme;
    std::string chatName;

    uint8_t *payload;
    uint16_t offset;
    int rc;

    uint32_t chatId;

    payload = netMsg.payload;
    offset = 0;
    // Parse data
    theme = packetReadU8(payload, &offset);
    chatName = packetReadString(payload, &offset, true);
    // Insert in db
    // // Insert chat
    rc = insertChat(serverConfig.db, chatName, theme);
    if(rc != SQLITE_DONE) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT, NMStatus::UNEXPECTED); });
        return;
    }
    chatId = sqlite3_last_insert_rowid(serverConfig.db);
    // // Set chat creator as owner
    rc = insertChatMember(serverConfig.db, chatId, session.userId, static_cast<uint8_t>(ChatRole::OWNER));
    // Answer the client
    if(rc != SQLITE_DONE) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT, NMStatus::UNEXPECTED); });
        return;
    }
    sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT, NMStatus::SUCCESS); });
    // Send client synch of the chat
    sendChatSync(session, chatId);
}
void handleClientMessageChatDel(ClientSession &session, NetworkMessage &netMsg) {
    uint32_t chatId;
    uint8_t senderRoleU8;
    ChatRole senderRole;

    uint8_t *payload;
    uint16_t offset;
    int rc;

    payload = netMsg.payload;
    offset = 0;
    // Parse data
    chatId = packetReadU32(payload, &offset);
    // Get user role
    rc = getUserRole(serverConfig.db, chatId, session.userId, &senderRoleU8);
    if(rc != SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT, NMStatus::UNEXPECTED); });
        return;
    }
    senderRole = static_cast<ChatRole>(senderRoleU8);
    // Check if user is allowed
    if(!hasRightToChatDel(senderRole)) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT, NMStatus::FORBIDDEN); });
        return;
    }
    // Delete in db
    rc = deleteChat(serverConfig.db, chatId);
    if(rc != SQLITE_DONE) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT, NMStatus::UNEXPECTED); });
        return;
    }
    sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT, NMStatus::SUCCESS); });
    // Send client synch of the chat
    NetworkMessage broadcastNM = buildNetMsgSyncChatDel(chatId);
    netMsgWriteSize(broadcastNM);
    broadcastNetMsg(chatId, broadcastNM);
}
void handleClientMessageChat(ClientSession &session, NetworkMessage &netMsg) {
    if(netMsg.action == NetworkMessageAction::ADD)
        handleClientMessageChatAdd(session, netMsg);
    else if(netMsg.action == NetworkMessageAction::DEL)
        handleClientMessageChatDel(session, netMsg);
    else
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MESSAGE, NMStatus::INVALID_ACTION); });
}
// // Add/delete/modify a chat member
void handleClientMessageChatMemberAdd(ClientSession &session, NetworkMessage &netMsg) {
    uint32_t chatId;
    std::string newMemberName;
    uint8_t newMemberRoleU8;
    ChatRole newMemberRole;
    uint8_t newMemberCurrentRole;

    uint8_t *payload;
    uint16_t offset;
    int rc;

    uint32_t newMemberUserId;
    uint8_t senderRoleU8;
    ChatRole senderRole;

    payload = netMsg.payload;
    offset = 0;
    // Parse data
    chatId = packetReadU32(payload, &offset);
    newMemberName = packetReadString(payload, &offset, true);
    newMemberRoleU8 = packetReadU8(payload, &offset);
    newMemberRole = static_cast<ChatRole>(newMemberRoleU8);

    // Check sending user has the right to do it
    rc = getUserRole(serverConfig.db, chatId, session.userId, &senderRoleU8);
    if(rc != SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::UNEXPECTED); });
        return;
    }
    senderRole = static_cast<ChatRole>(senderRoleU8);
    if(!hasRightToManageMember(senderRole, newMemberRole)) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::FORBIDDEN); });
        return;
    }
    // Check user exists
    rc = getUserId(serverConfig.db, newMemberName, &newMemberUserId);
    if(rc != SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::USER_NOT_IN_DB); });
        return;
    }
    // Check user not already in the chat
    rc = getUserRole(serverConfig.db, chatId, newMemberUserId, &newMemberCurrentRole);
    if(rc == SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::USER_EXIST); });
        return;
    }
    // If all check passed add user
    rc = getUserId(serverConfig.db, newMemberName, &newMemberUserId);
    if(rc != SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::UNEXPECTED); });
        return;
    }
    rc = insertChatMember(serverConfig.db, chatId, newMemberUserId, newMemberRoleU8);
    if(rc != SQLITE_DONE) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::UNEXPECTED); });
        return;
    }
    // Notify connected chat members
    NetworkMessage broadcastNM = buildNetMsgSyncMemberAdd(chatId, User(newMemberUserId, newMemberName), newMemberRole);
    netMsgWriteSize(broadcastNM);
    broadcastNetMsg(chatId, broadcastNM);
    // If connected send new member chat sync
    auto it = idToSession.find(newMemberUserId);
    if(it != idToSession.end())
        sendChatSync(*it->second, chatId);
    // Send admin notif
    writeMessageAdmin(chatId, "Added user: " + newMemberName + " as " + chatRoleToStr(newMemberRole));
    // Notify sender of success
    sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::SUCCESS); });
}
void handleClientMessageChatMemberSet(ClientSession &session, NetworkMessage &netMsg) {
    uint32_t chatId;
    std::string userName;
    uint8_t userRoleU8;
    ChatRole userRole;

    uint8_t *payload;
    uint16_t offset;
    int rc;

    uint32_t userId;
    uint8_t userOldRoleU8;
    uint8_t senderRoleU8;
    ChatRole userOldRole;
    ChatRole senderRole;

    payload = netMsg.payload;
    offset = 0;
    // Parse data
    chatId = packetReadU32(payload, &offset);
    userName = packetReadString(payload, &offset, true);
    userRoleU8 = packetReadU8(payload, &offset);
    userRole = static_cast<ChatRole>(userRoleU8);

    // Check sending user has the right to do it
    rc = getUserRole(serverConfig.db, chatId, session.userId, &senderRoleU8);
    if(rc != SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::UNEXPECTED); });
        return;
    }
    senderRole = static_cast<ChatRole>(senderRoleU8);
    if(!hasRightToManageMember(senderRole, userRole)) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::FORBIDDEN); });
        return;
    }
    // Check user exists
    rc = getUserId(serverConfig.db, userName, &userId);
    if(rc != SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::USER_NOT_IN_DB); });
        return;
    }
    // Check user in int chat
    rc = getUserRole(serverConfig.db, chatId, userId, &userOldRoleU8);
    userOldRole = static_cast<ChatRole>(userOldRoleU8);
    if(rc != SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::USER_NOT_IN_CHAT); });
        return;
    }
    if(!hasRightToManageMember(senderRole, userOldRole)) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::FORBIDDEN); });
        return;
    }
    if(userOldRoleU8 == userRoleU8) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::NOTHING_TO_DO); });
        return;
    }
    // If all check passed add user
    rc = updateUserRole(serverConfig.db, chatId, userId, userRoleU8);
    if(rc != SQLITE_DONE) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::UNEXPECTED); });
        return;
    }
    // Notify connected user
    NetworkMessage broadcastNM = buildNetMsgSyncMemberSet(chatId, User(userId, userName), userRole);
    netMsgWriteSize(broadcastNM);
    broadcastNetMsg(chatId, broadcastNM);
    // Send admin notif
    writeMessageAdmin(chatId, userName + " is now " + chatRoleToStr(userRole));

    sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::SUCCESS); });
}
void handleClientMessageChatMemberDel(ClientSession &session, NetworkMessage &netMsg) {
    uint32_t chatId;
    std::string userName;

    uint8_t *payload;
    uint16_t offset;
    int rc;

    uint32_t userId;
    uint8_t userOldRoleU8;
    ChatRole userOldRole;
    uint8_t senderRoleU8;
    ChatRole senderRole;
    std::vector<uint32_t> userIds;
    std::string senderName;

    payload = netMsg.payload;
    offset = 0;
    // Parse data
    chatId = packetReadU32(payload, &offset);
    userName = packetReadString(payload, &offset, true);

    // Check sending user has the right to do it
    rc = getUserRole(serverConfig.db, chatId, session.userId, &senderRoleU8);
    if(rc != SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::UNEXPECTED); });
        return;
    }
    senderRole = static_cast<ChatRole>(senderRoleU8);
    if(senderRoleU8 > static_cast<uint8_t>(ChatRole::ADMIN)) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::FORBIDDEN); });
        return;
    }
    // Check user exists
    rc = getUserId(serverConfig.db, userName, &userId);
    if(rc != SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::USER_NOT_IN_DB); });
        return;
    }
    // Check user in int chat
    rc = getUserRole(serverConfig.db, chatId, userId, &userOldRoleU8);
    if(rc != SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::USER_NOT_IN_CHAT); });
        return;
    }
    userOldRole = static_cast<ChatRole>(userOldRoleU8);
    if(!hasRightToManageMember(senderRole, userOldRole)) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::FORBIDDEN); });
        return;
    }
    // If all check passed add user
    rc = deleteChatMember(serverConfig.db, chatId, userId);
    if(rc != SQLITE_DONE) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::UNEXPECTED); });
        return;
    }
    // Notify connected user
    NetworkMessage broadcastNM = buildNetMsgSyncMemberDel(chatId, userId);
    netMsgWriteSize(broadcastNM);
    broadcastNetMsg(chatId, broadcastNM);
    // If deleted user is connected notify him
    sendNetMsgIfConnected(userId, [chatId]() { return buildNetMsgSyncChatDel(chatId); });
    // Send admin notif
    rc = getUserName(serverConfig.db, session.userId, &senderName);
    writeMessageAdmin(chatId, senderName + " removed " + userName);
    // Send success to sender
    sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MEMBER, NMStatus::SUCCESS); });
}
void handleClientMessageChatMember(ClientSession &session, NetworkMessage &netMsg) {
    if(netMsg.action == NetworkMessageAction::ADD)
        handleClientMessageChatMemberAdd(session, netMsg);
    else if(netMsg.action == NetworkMessageAction::SET)
        handleClientMessageChatMemberSet(session, netMsg);
    else if(netMsg.action == NetworkMessageAction::DEL)
        handleClientMessageChatMemberDel(session, netMsg);
    else
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MESSAGE, NMStatus::INVALID_ACTION); });
}
// // Add/delete/modigy a chat message
void handleClientMessageChatMessageAdd(ClientSession &session, NetworkMessage &netMsg) {
    uint32_t chatId;
    std::string name;
    uint32_t timestamp;
    std::string content;

    uint16_t contentSize;
    uint8_t *payload;
    uint16_t offset;
    int rc;

    Message message;
    uint8_t senderRoleU8;

    contentSize = netMsg.size - NM_HEADER_SIZE - 8;
    payload = netMsg.payload;
    offset = 0;
    // Parse data
    chatId = packetReadU32(payload, &offset);
    timestamp = packetReadU32(payload, &offset);
    content = packetReadStringN(payload, &offset, contentSize);
    // Check user is in chat
    rc = getUserRole(serverConfig.db, chatId, session.userId, &senderRoleU8);
    if(rc != SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MESSAGE, NMStatus::USER_NOT_IN_CHAT); });
        return;
    }
    // Build message
    message.author = User(session.userId, session.username);
    message.content = content;
    message.timestamp = timestamp;
    // Insert message if message successfuly built
    rc = insertChatMessage(serverConfig.db, chatId, session.userId, content, timestamp);
    if(rc != SQLITE_DONE) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MESSAGE, NMStatus::UNEXPECTED); });
        return;
    }
    message.id = sqlite3_last_insert_rowid(serverConfig.db);
    // Broadcast message
    NetworkMessage broadcastNM = buildNetMsgSyncMessageAdd(chatId, message);
    netMsgWriteSize(broadcastNM);
    broadcastNetMsg(chatId, broadcastNM);
    // Notify sender
    sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MESSAGE, NMStatus::SUCCESS); });
}
void handleClientMessageChatMessageSet(ClientSession &session, NetworkMessage &netMsg) { /* TODO */ }
void handleClientMessageChatMessageDel(ClientSession &session, NetworkMessage &netMsg) { /* TODO */ }
void handleClientMessageChatMessage(ClientSession &session, NetworkMessage &netMsg) {
    switch(netMsg.action) {
    case NMAction::ADD:
        handleClientMessageChatMessageAdd(session, netMsg);
        break;
    case NMAction::SET:
        handleClientMessageChatMessageSet(session, netMsg);
        break;
    case NMAction::DEL:
        handleClientMessageChatMessageDel(session, netMsg);
        break;
    default:
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_MESSAGE, NMStatus::INVALID_ACTION); });
    }
}
// // Change theme
void handleClientMessageChatThemeSet(ClientSession &session, NetworkMessage &netMsg) {
    uint32_t chatId;
    uint8_t theme;

    uint8_t *payload;
    uint16_t offset;
    int rc;

    uint8_t senderRoleU8;
    ChatRole senderRole;

    payload = netMsg.payload;
    offset = 0;
    // Parse data
    chatId = packetReadU32(payload, &offset);
    theme = packetReadU8(payload, &offset);
    // Process data
    if(theme > chatThemeLast) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_THEME, NMStatus::INVALID_ARGUMENT); });
        return;
    }
    rc = getUserRole(serverConfig.db, chatId, session.userId, &senderRoleU8);
    if(rc != SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_THEME, NMStatus::USER_NOT_IN_CHAT); });
        return;
    }
    senderRole = static_cast<ChatRole>(senderRoleU8);
    if(!hasRightToManage(senderRole)) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_THEME, NMStatus::FORBIDDEN); });
        return;
    }
    std::printf("Update theme: %u\n", theme);
    rc = updateChatTheme(serverConfig.db, chatId, theme);
    if(rc != SQLITE_DONE) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_THEME, NMStatus::UNEXPECTED); });
        return;
    }
    Chat chat;
    rc = getChatBasicData(serverConfig.db, chatId, chat);
    if(rc != SQLITE_ROW) {
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_THEME, NMStatus::UNEXPECTED); });
        return;
    }
    NetworkMessage broadcastNM = buildNetMsgSyncChatSet(chat);
    netMsgWriteSize(broadcastNM);
    broadcastNetMsg(chatId, broadcastNM);
    // Notify sender
    sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_THEME, NMStatus::SUCCESS); });
}
void handleClientMessageChatTheme(ClientSession &session, NetworkMessage &netMsg) {
    switch(netMsg.action) {
    case NMAction::SET:
        handleClientMessageChatThemeSet(session, netMsg);
        break;
    default:
        sendNetMsg(session, []() { return buildNetMsgAnswer(NMType::CHAT_THEME, NMStatus::INVALID_ACTION); });
        break;
    }
}
// // Quit
void handleClientMessageChatQuit(ClientSession &session, NetworkMessage &netMsg) { /* TODO */ }

void handleClientMessage(ClientSession &session) {
    NetworkMessage netMsg;

    netMsg = packetToNetMsg(session.msgBuffer);
    DEBUG_printNetworkMessage(session.msgBuffer, netMsg);
    switch(netMsg.type) {
    case NetworkMessageType::SIGNIN:
        handleClientMessageSignIn(session, netMsg);
        break;
    case NetworkMessageType::LOGIN:
        handleClientMessageLogin(session, netMsg);
        break;
    case NetworkMessageType::CHAT:
        handleClientMessageChat(session, netMsg);
        break;
    case NetworkMessageType::CHAT_MEMBER:
        handleClientMessageChatMember(session, netMsg);
        break;
    case NetworkMessageType::CHAT_MESSAGE:
        handleClientMessageChatMessage(session, netMsg);
        break;
    case NetworkMessageType::CHAT_THEME:
        handleClientMessageChatTheme(session, netMsg);
        break;
    case NetworkMessageType::QUIT:
        break;
    default:
        break;
    }
}

void disconnectClient(ClientSession &session) {
    int fd = session.connection.fd;
    uint32_t id = session.userId;

    // Remove from epoll
    epoll_ctl(serverConfig.epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    // Free ssl
    if(session.connection.ssl != NULL) {
        SSL_shutdown(session.connection.ssl);
        SSL_free(session.connection.ssl);
    }
    // Close TCP connection
    if(session.connection.fd != -1)
        close(fd);
    auto itId = idToSession.find(session.userId);
    if(itId != idToSession.end())
        idToSession.erase(itId);
    auto itFd = fdToSession.find(session.connection.fd);
    if(itFd != fdToSession.end()) {
        clients.erase(itFd->second);
        fdToSession.erase(itFd);
    }
    std::printf("Disconnected client fd=%d, id=%u\n", fd, id);

    return;
}
/* Init epoll */
int initEpoll() {
    struct epoll_event event;

    serverConfig.epoll_fd = epoll_create1(0);
    if(serverConfig.epoll_fd == -1) {
        fprintf(stderr, "Failed to create epoll file descriptor.\n");
        close(serverConfig.fd);
        return EXIT_FAILURE;
    }
    event.events = EPOLLIN;
    event.data.fd = serverConfig.fd;
    if(epoll_ctl(serverConfig.epoll_fd, EPOLL_CTL_ADD, serverConfig.fd, &event)) {
        fprintf(stderr, "Failed to add server fd to epoll.\n");
        close(serverConfig.fd);
        close(serverConfig.epoll_fd);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
/* Process client connection */
void processClientConnection(int fd) {
    socklen_t client_fd_len;
    struct sockaddr_in client_addr;
    char client_ip_str[INET_ADDRSTRLEN];
    int client_fd;
    struct epoll_event event;

    // Accept connection
    client_fd_len = sizeof(client_addr);
    client_fd = accept(serverConfig.fd, (struct sockaddr *)&client_addr, &client_fd_len);
    if(client_fd == -1) {
        perror("accept\n");
        return;
    }
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip_str, INET_ADDRSTRLEN);
    printf("Recieved connection from %s.\n", client_ip_str);
    // Add session
    clients.push_back(ClientSession());
    ClientSession &session = clients.back();
    session.connected = true;
    session.connection.ssl = nullptr;
    session.connection.tls = false;
    session.connection.fd = client_fd;
    fdToSession[client_fd] = std::prev(clients.end());
    initMessageParsing(session.parsingData);
    // Make connection none blocking
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    // Add to epoll
    event.events = EPOLLIN;
    event.data.fd = client_fd;
    epoll_ctl(serverConfig.epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
}
void processClientConnectionTLS(int fd) {
    socklen_t client_fd_len;
    struct sockaddr_in client_addr;
    char client_ip_str[INET_ADDRSTRLEN];
    int client_fd;

    // Accept connection
    client_fd_len = sizeof(client_addr);
    client_fd = accept(serverConfig.fd, (struct sockaddr *)&client_addr, &client_fd_len);
    if(client_fd == -1) {
        perror("accept\n");
        return;
    }
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip_str, INET_ADDRSTRLEN);
    printf("Recieved connection from %s.\n", client_ip_str);
    // Add a client
    clients.push_back(ClientSession());
    ClientSession &session = clients.back();
    session.connected = false;
    session.connection.ssl = nullptr;
    session.connection.tls = true;
    session.connection.fd = client_fd;
    fdToSession[client_fd] = std::prev(clients.end());
    initMessageParsing(session.parsingData);
    // Make connection none blocking
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    // Create SSL socket binding
    SSL *ssl = SSL_new(serverConfig.ctx);
    SSL_set_fd(ssl, client_fd);
    session.connection.ssl = ssl;
    // Add to epoll
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = client_fd;
    epoll_ctl(serverConfig.epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
}
void processClientConnectionTLSHandshake(ClientSession &session) {
    SSL *ssl;
    int ret;

    ssl = session.connection.ssl;
    ret = SSL_accept(ssl);
    if(ret <= 0) {
        switch(SSL_get_error(ssl, ret)) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            return;
        case SSL_ERROR_SYSCALL:
        case SSL_ERROR_SSL:
            if(SSL_get_verify_result(ssl) != X509_V_OK)
                fprintf(stderr, "Verify error: %s\n", X509_verify_cert_error_string(SSL_get_verify_result(ssl)));
        default:
            ERR_print_errors_fp(stderr);
            disconnectClient(session);
            return;
        }
    } else {
        // Set connection variables
        session.connection.ssl = ssl;
        session.connection.tls = true;
        session.connected = true;
    }
}
/* Process client communication */
void processClientCommunication(ClientSession &session) {
    NextMessageState nextMessageState;

    ssize_t nbytes = recvInBuffer(session.parsingData, session.connection);
    if(nbytes <= 0) {
        if(session.connection.tls) {
            SSL *ssl = session.connection.ssl;
            switch(SSL_get_error(ssl, nbytes)) {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                return;
            case SSL_ERROR_SYSCALL:
            case SSL_ERROR_SSL:
                if(SSL_get_verify_result(ssl) != X509_V_OK)
                    fprintf(stderr, "Verify error: %s\n", X509_verify_cert_error_string(SSL_get_verify_result(ssl)));
            default:
                ERR_print_errors_fp(stderr);
                disconnectClient(session);
                return;
            }
        } else {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            else {
                disconnectClient(session);
                return;
            }
        }
    } else {
        do {
            nextMessageState = nextMessage(session.parsingData, MAGIC_BYTE, session.msgBuffer);
            if(nextMessageState == NextMessageState::OK)
                handleClientMessage(session);
        } while(nextMessageState != NextMessageState::NO_MESSAGE);
    }
}

int runServer(int argc, char *argv[]) {
    if(argc < 3)
        serverConfig.tls = true;
    else {
        if(std::strncmp(argv[2], "TLS", 3) == 0)
            serverConfig.tls = true;
        else if(std::strncmp(argv[2], "RAW", 3) == 0)
            serverConfig.tls = false;
        else {
            std::printf("Unknown server encryption option. Please user TLS or RAW.\n");
            exit(1);
        }
    }
    /* == VARIABLE == */
    // Server
    std::string port;

    /* == INIT == */
    // DB
    serverConfig.db = getDb(true);
    sqlite3_extended_result_codes(serverConfig.db, 1);
    insertChatRole(serverConfig.db, static_cast<uint8_t>(ChatRole::OWNER), "OWNER");
    insertChatRole(serverConfig.db, static_cast<uint8_t>(ChatRole::ADMIN), "ADMIN");
    insertChatRole(serverConfig.db, static_cast<uint8_t>(ChatRole::MEMBER), "MEMBER");
    insertUserServer(serverConfig.db);
    DEBUG_createUsers();
    // Setting up server
    port = "50000";
    serverConfig.fd = socket_server(port.c_str());
    if(serverConfig.fd == -1) {
        fprintf(stderr, "Failed to create server file descriptor.\n");
        return 1;
    }
    // Epoll
    initEpoll();
    // TLS context
    if(serverConfig.tls)
        serverConfig.ctx = create_SSL_ctx_server(TLS1_VERSION, TLS1_3_VERSION);

    /* == MAIN LOOP == */
    while(1) {
        int n = epoll_wait(serverConfig.epoll_fd, serverConfig.events, MAX_EVENTS, -1);
        for(int i = 0; i < n; i++) {
            int fd = serverConfig.events[i].data.fd;
            if(fd == serverConfig.fd) {
                if(serverConfig.tls)
                    processClientConnectionTLS(fd);
                else
                    processClientConnection(fd);
            } else {
                ClientSession &session = *fdToSession[fd];
                if(session.connection.tls && !session.connected)
                    processClientConnectionTLSHandshake(session);
                else
                    processClientCommunication(session);
            }
        }
    }

    return 0;
}
