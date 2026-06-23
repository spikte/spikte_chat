#include "../include/run_client.hpp"


static void disconnect() {
    localUser = User();
    clientConfig.loadedChat.clear();
    clientConfig.idToChat.clear();
    guiChatListData.chatList.clear();
    guiChatListData.currentChatId = 0;
    guiChatListData.idToGuiChatData.clear();
    if(clientConfig.connection.ssl) {
        SSL_shutdown(clientConfig.connection.ssl);
        SSL_free(clientConfig.connection.ssl);
    }
    if(clientConfig.ctx)
        SSL_CTX_free(clientConfig.ctx);
    if(clientConfig.connection.fd != -1)
        close(clientConfig.connection.fd);
    clientConfig.connection.ssl = nullptr;
    clientConfig.connection.tls = false;
    clientConfig.ctx = nullptr;
    clientConfig.serverId = 0;
    clientCoreState = ClientCoreState::SERVER_CONNECTION;
}
static void moveGuiChatDataToFront(uint32_t chatId) {
    std::list<GuiChatData>& chatList = guiChatListData.chatList;
    auto& idToGuiChatData = guiChatListData.idToGuiChatData;

    auto itKeyVal = idToGuiChatData.find(chatId);
    if(itKeyVal == idToGuiChatData.end())
        return;
    auto itVal = itKeyVal->second;
    chatList.splice(chatList.begin(), chatList, itVal);
}


/* STATE */
void initState(ClientCoreState prevState) {
    bool setDefaultAddr = true;
    switch(clientCoreState) {
        case ClientCoreState::SERVER_CONNECTION:
            if(prevState != ClientCoreState::SERVER_CONNECTION || prevState != ClientCoreState::WAIT_SERVER_CONNECTION) {
                setDefaultAddr = false;
                disconnect();
            }
            initGuiServerInputData(setDefaultAddr);
            break;
        case ClientCoreState::WAIT_SERVER_CONNECTION:
            clientConfig.ctx = nullptr;
            clientConfig.connection.ssl = nullptr;
            clientConfig.connection.fd = -1;
            break;
        case ClientCoreState::SIGNIN:
            if(prevState == ClientCoreState::WAIT_SIGNIN)
                initGuiSignInData(false);
            else
                initGuiSignInData(false);
            break;
        case ClientCoreState::LOGIN:
            if(prevState == ClientCoreState::WAIT_LOGIN)
                initGuiLoginData(false);
            else
                initGuiLoginData(false);
            break;
        case ClientCoreState::CHAT_CREATE:
            initGuiChatCreateData();
            break;
        case ClientCoreState::CHAT_DELETE:
            initGuiChatDeleteData();
            break;
        default:
            break;
    }
}
void drawState() {
    switch(clientCoreState) {
        case ClientCoreState::SERVER_CONNECTION:
            GuiServerInput();
            break;
        case ClientCoreState::WAIT_SERVER_CONNECTION:
            GuiWaitServerInput();
            break;
        case ClientCoreState::SIGNIN:
        case ClientCoreState::LOGIN:
            GuiAuth();
            break;
        case ClientCoreState::WAIT_SIGNIN:
        case ClientCoreState::WAIT_LOGIN:
            GuiWaitAuth();
            break;
        case ClientCoreState::CHAT_CREATE:
            GuiChatCreate();
            break;
        case ClientCoreState::CHAT_DELETE:
            GuiChatDelete();
            break;
        case ClientCoreState::WAIT_CHAT_CREATE:
            GuiWaitChatCreate();
            break;
        case ClientCoreState::WAIT_COMMAND_PROCESS:
        case ClientCoreState::CHAT:
            GuiChatList();
            break;
        default:
            DrawText("ERROR NO GUI FOR THIS STATE", 0, 0, 16, BLACK);
            break;
    }
}
void updateStateWaitServerConnection() {
    if(clientConfig.connection.tls) {
        // Setup OpenSLL context
        clientConfig.connection.fd = init_socket_TLS(clientConfig.serverAddr.c_str(),
                                                     clientConfig.serverPort.c_str(),
                                                     clientConfig.minTlsVersion,
                                                     clientConfig.minTlsVersion,
                                                     &clientConfig.ctx,
                                                     &clientConfig.connection.ssl);
        if(clientConfig.ctx == nullptr) {
            guiServerInputData.error = true;
            disconnect();
            return;
        }
        if(clientConfig.connection.ssl == nullptr) {
            guiServerInputData.error = true;
            disconnect();
            return;
        }
        if(clientConfig.connection.fd == -1) {
            guiServerInputData.error = true;
            disconnect();
            return;
        }
        if(SSL_connect(clientConfig.connection.ssl) < 1) {
            guiServerInputData.error = true;
            disconnect();
            return;
        }
    }
    else {
        // TCP connection
        clientConfig.connection.fd = socket_connect(clientConfig.serverAddr.c_str(), clientConfig.serverPort.c_str());
        if(clientConfig.connection.fd == -1) {
            guiServerInputData.error = true;
            disconnect();
            return;
        }
    }
    // Set server file descriptor as non blocking
    int flags = fcntl(clientConfig.connection.fd, F_GETFL, 0);
    if(fcntl(clientConfig.connection.fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        guiServerInputData.error = true;
        clientCoreState = ClientCoreState::SERVER_CONNECTION;
    }
    else
        clientCoreState = ClientCoreState::SIGNIN;
}
void updateState() {
    switch(clientCoreState) {
        case ClientCoreState::WAIT_SERVER_CONNECTION:
            updateStateWaitServerConnection();
            break;
        default:
            break;
    }
}
void updateNetwork(PacketParsingData& parsingData) {
    ssize_t nbytes;
    NextMessageState nextMessageState;
    uint8_t buffer[NM_MAX_SIZE];

    nbytes = recvInBuffer(parsingData, clientConfig.connection);
    if(nbytes <= 0) {
        if(clientConfig.connection.tls) {
            switch(SSL_get_error(clientConfig.connection.ssl, nbytes)) {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                    return;
                case SSL_ERROR_SYSCALL:
                case SSL_ERROR_SSL:
                    if (SSL_get_verify_result(clientConfig.connection.ssl) != X509_V_OK)
                        fprintf(stderr, "Verify error: %s\n", X509_verify_cert_error_string(SSL_get_verify_result(clientConfig.connection.ssl)));
                default:
                    disconnect();
                    return;
            }
        }
        else {
            if(nbytes == 0) {
                disconnect();
                return;
            } else if(nbytes < 0) {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                    return;
                disconnect();
                return;
            }
        }
    }
    else {
        do {
            nextMessageState = nextMessage(parsingData, MAGIC_BYTE, buffer);
            if(nextMessageState == NextMessageState::OK)
                handleServerMessage(buffer);
        } while(nextMessageState != NextMessageState::NO_MESSAGE);
    }
}

/* HANDLE SERVER MESSAGE */
static bool canHandleMessage(NetworkMessageType messageType) {
    switch(clientCoreState) {
        case ClientCoreState::SERVER_CONNECTION:
            return false;
        case ClientCoreState::WAIT_SERVER_CONNECTION:
            return false;
        case ClientCoreState::SIGNIN:
            return false;
        case ClientCoreState::WAIT_SIGNIN:
            if(messageType == NMType::SIGNIN)
                return true;
            return false;
        case ClientCoreState::LOGIN:
            return false;
        case ClientCoreState::WAIT_LOGIN:
            if(messageType == NMType::LOGIN)
                return true;
            return false;
        case ClientCoreState::CHAT:
        case ClientCoreState::CHAT_CREATE:
        case ClientCoreState::WAIT_CHAT_CREATE:
        case ClientCoreState::WAIT_CHAT_MESSAGE:
        case ClientCoreState::WAIT_COMMAND_PROCESS:
            if(messageType == NMType::CHAT ||
               messageType == NMType::CHAT_MEMBER ||
               messageType == NMType::CHAT_MESSAGE ||
               messageType == NMType::CHAT_SYNC_CHAT ||
               messageType == NMType::CHAT_SYNC_MEMBER ||
               messageType == NMType::CHAT_SYNC_MESSAGE)
                return true;
            return false;
    }
    return false;
}
// Auth
void handleServerMessageSignIn(NetworkMessage& netMsg) {
    if(netMsg.status == NMStatus::SUCCESS) {
        uint16_t offset;

        offset = 0;
        localUser.id = packetReadU32(netMsg.payload, &offset);
        clientCoreState = ClientCoreState::CHAT;
        return;
    }
    guiAuthData.error = true;
    switch(netMsg.status) {
        case NMStatus::USER_EXIST:
            guiAuthData.errorMsg = "ERROR: Username already taken.";
            break;
        default:
            guiAuthData.errorMsg = "ERROR: Unexpected error, please try again later.";
            break;
    }
    clientCoreState = ClientCoreState::SIGNIN;
}
void handleServerMessageLogin(NetworkMessage& netMsg) {
    if(netMsg.status == NMStatus::SUCCESS) {
        uint16_t offset;

        offset = 0;
        localUser.id = packetReadU32(netMsg.payload, &offset);
        clientCoreState = ClientCoreState::CHAT;
        return;
    }
    guiAuthData.error = true;
    switch(netMsg.status) {
        case NMStatus::USER_NOT_IN_DB:
            guiAuthData.errorMsg = "ERROR: Username doesn't exist.";
            break;
        default:
            guiAuthData.errorMsg = "ERROR: Unexpected error, please try again later.";
            break;
    }
    clientCoreState = ClientCoreState::LOGIN;
}
// Chat create response
void handleServerMessageChatAdd(NetworkMessage& netMsg) {
    if(netMsg.status == NMStatus::SUCCESS) {
        clientCoreState = ClientCoreState::CHAT;
        return;
    }
    clientCoreState = ClientCoreState::CHAT_CREATE;
}
// Chat sync
// Chat metadta
void handleServerMessageChatSyncChatAdd(NetworkMessage& netMsg) {
    uint32_t chatId;
    uint8_t chatTheme;
    std::string chatName;

    uint16_t offset;
    uint8_t* payload;


    payload = netMsg.payload;
    offset = 0;
    // Parse data
    chatId = packetReadU32(payload, &offset);
    chatTheme = packetReadU8(payload, &offset);
    chatName = packetReadString(payload, &offset, true, MAX_SIZE_CHAT_NAME);
    // Check if chat doesn't already exists
    if(clientConfig.idToChat.find(chatId) != clientConfig.idToChat.end())
        return;
    // Load chat
    clientConfig.loadedChat.push_front({});
    Chat& chat = clientConfig.loadedChat.front();
    // Set chat property
    chat.id = chatId;
    chat.theme = static_cast<ChatTheme>(chatTheme);
    chat.name = chatName;
    // Store ID -> Iterator in `clientConfig.loadedChat`
    clientConfig.idToChat[chat.id] = clientConfig.loadedChat.begin();
    // Create corresponding GuiChatList
    std::list<GuiChatData>& chatList = guiChatListData.chatList;
    chatList.push_front({});
    GuiChatData& chatData = chatList.front();
    chatData.id = chatId;
    guiChatListData.currentChatId = chatId;
    initGuiChatData(chatData);
    // Store ID -> index in `guiChatListData.chatList`
    guiChatListData.idToGuiChatData[chat.id] = chatList.begin();
    // Update gui
    GuiChatData& guiChatData = *guiChatListData.idToGuiChatData[chatId];
    updateGuiChatDataChat(guiChatData, chatId);
    moveGuiChatDataToFront(chatId);
}
void handleServerMessageChatSyncChatSet(NetworkMessage& netMsg) {
    uint32_t chatId;
    uint8_t chatTheme;
    std::string chatName;

    uint8_t *payload;
    uint16_t offset;


    payload = netMsg.payload;
    offset = 0;
    // Parse data
    chatId = packetReadU32(payload, &offset);
    chatTheme = packetReadU8(payload, &offset);
    chatName = packetReadString(payload, &offset);
    // Check if chat exist with this user
    auto itChat = clientConfig.idToChat.find(chatId);
    if(itChat == clientConfig.idToChat.end())
        return;
    // If so we set value for the chat
    Chat& chat = *itChat->second;
    chat.theme = static_cast<ChatTheme>(chatTheme);
    chat.name = chatName;
    // Update gui
    GuiChatData& guiChatData = *guiChatListData.idToGuiChatData[chatId];
    updateGuiChatDataChat(guiChatData, chatId);
    moveGuiChatDataToFront(chatId);
}
void handleServerMessageChatSyncChatDel(NetworkMessage& netMsg) {
    uint32_t chatId;

    uint8_t *payload;
    uint16_t offset;


    payload = netMsg.payload;
    offset = 0;
    // Parse data
    chatId = packetReadU32(payload, &offset);
    // Delete GuiChatData
    auto itGui = guiChatListData.idToGuiChatData.find(chatId);
    if(itGui != guiChatListData.idToGuiChatData.end()) {
        guiChatListData.chatList.erase(itGui->second);
        guiChatListData.idToGuiChatData.erase(itGui);
    }
    // Delete Chat
    auto itChat = clientConfig.idToChat.find(chatId);
    if(itChat != clientConfig.idToChat.end()) {
        clientConfig.loadedChat.erase(itChat->second);
        clientConfig.idToChat.erase(itChat);
    }
    if(guiChatListData.currentChatId == chatId) {
        if(!guiChatListData.chatList.empty())
            guiChatListData.currentChatId = guiChatListData.chatList.front().id;
        else
            guiChatListData.currentChatId = 0;
    }
}
void handleServerMessageChatSyncChat(NetworkMessage& netMsg) {
    switch(netMsg.action) {
        case NMAction::ADD:
            handleServerMessageChatSyncChatAdd(netMsg);
            break;
        case NMAction::SET:
            handleServerMessageChatSyncChatSet(netMsg);
            break;
        case NMAction::DEL:
            handleServerMessageChatSyncChatDel(netMsg);
            break;
        default:
            break;
    }
}
// Chat members
void handleServerMessageChatSyncMemberAdd(NetworkMessage& netMsg) {
    uint16_t offset;
    uint32_t chatId;
    User user;
    ChatRole role;
    uint8_t *buffer;


    buffer = netMsg.payload;
    offset = 0;
    chatId = packetReadU32(buffer, &offset);
    if(clientConfig.idToChat.find(chatId) == clientConfig.idToChat.end())
        return;
    Chat& chat = *clientConfig.idToChat[chatId];
    user.id = packetReadU32(buffer, &offset);
    role = static_cast<ChatRole>(packetReadU8(buffer, &offset));
    user.name = packetReadString(buffer, &offset);
    if(chat.roles.find(user.id) != chat.roles.end())
        return;
    else {
        chat.members.push_back(user);
        chat.roles[user.id] = role;
    }
}
void handleServerMessageChatSyncMemberSet(NetworkMessage& netMsg) {/*TODO (actually nothing to do for this action with the current system design)*/}
void handleServerMessageChatSyncMemberDel(NetworkMessage& netMsg) {
    uint16_t offset;
    uint32_t chatId;
    uint32_t userId;
    uint8_t *buffer;


    buffer = netMsg.payload;
    offset = 0;
    chatId = packetReadU32(buffer, &offset);
    userId = packetReadU32(buffer, &offset);

    auto itChat = clientConfig.idToChat.find(chatId);
    if(itChat == clientConfig.idToChat.end())
        return;
    Chat& chat = *itChat->second;
    auto itUser = std::find_if(chat.members.begin(),
                               chat.members.end(),
                               [userId](const User& u) { return u.id == userId; });
    if(itUser == chat.members.end())
        return;
    chat.roles.erase(userId);
    chat.members.erase(itUser);
}
void handleServerMessageChatSyncMember(NetworkMessage& netMsg) {
    switch(netMsg.action) {
        case NMAction::ADD:
            handleServerMessageChatSyncMemberAdd(netMsg);
            break;
        case NMAction::SET:
            handleServerMessageChatSyncMemberSet(netMsg);
            break;
        case NMAction::DEL:
            handleServerMessageChatSyncMemberDel(netMsg);
            break;
        default:
            break;
    }
}
// Chat message
void handleServerMessageChatSyncMessageAdd(NetworkMessage& netMsg) {
    uint32_t chatId;
    uint32_t authorId;
    std::string authorName;
    Message message;

    uint16_t contentSize;
    uint8_t* payload;
    uint16_t offset;


    contentSize = netMsg.size - NM_HEADER_SIZE - 4 * 4 - MAX_SIZE_USERNAME;
    payload = netMsg.payload;
    offset = 0;
    // Parse content
    chatId = packetReadU32(payload, &offset);
    if(clientConfig.idToChat.find(chatId) == clientConfig.idToChat.end())
        return;
    authorId = packetReadU32(payload, &offset);
    authorName = packetReadString(payload, &offset, true, MAX_SIZE_USERNAME);
    message.id = packetReadU32(payload, &offset);
    message.timestamp = packetReadU32(payload, &offset);
    message.content = packetReadStringN(payload, &offset, contentSize);
    message.author = User(authorId, authorName);
    // Add message to the chat
    std::vector<Message>& messages = (*clientConfig.idToChat[chatId]).messages;
    messages.push_back(message); // I assume that message are sent in order which should be the case within a TCP connection
    // Update gui
    GuiChatData& guiChatData = *guiChatListData.idToGuiChatData[chatId];
    updateGuiChatDataMessages(guiChatData, chatId);
    moveGuiChatDataToFront(chatId);
}
void handleServerMessageChatSyncMessageSet(NetworkMessage& netMsg) {/*TODO*/}
void handleServerMessageChatSyncMessageDel(NetworkMessage& netMsg) {/*TODO*/}
void handleServerMessageChatSyncMessage(NetworkMessage& netMsg) {
    switch(netMsg.action) {
        case NMAction::ADD:
            handleServerMessageChatSyncMessageAdd(netMsg);
            break;
        case NMAction::SET:
            handleServerMessageChatSyncMessageSet(netMsg);
            break;
        case NMAction::DEL:
            handleServerMessageChatSyncMessageDel(netMsg);
            break;
        default:
            break;
    }
}
void handleServerMessage(uint8_t buffer[NM_MAX_SIZE]) {
    NetworkMessage netMsg;

    netMsg = packetToNetMsg(buffer);
    DEBUG_printNetworkMessage(buffer, netMsg);
    if(!canHandleMessage(netMsg.type))
        return;
    switch(netMsg.type) {
        case NetworkMessageType::SIGNIN:
            handleServerMessageSignIn(netMsg);
            break;
        case NetworkMessageType::LOGIN:
            handleServerMessageLogin(netMsg);
            break;
        case NetworkMessageType::CHAT:
            handleServerMessageChatAdd(netMsg);
            break;
        case NetworkMessageType::CHAT_SYNC_CHAT:
            handleServerMessageChatSyncChat(netMsg);
            break;
        case NetworkMessageType::CHAT_SYNC_MEMBER:
            handleServerMessageChatSyncMember(netMsg);
            break;
        case NetworkMessageType::CHAT_SYNC_MESSAGE:
            handleServerMessageChatSyncMessage(netMsg);
            break;
        default:
            break;
    }
}


/* INIT */
void initClientGui(int screenWidth, int screenHeight) {
    InitWindow(screenWidth, screenHeight, "Beej's chat (client)");
    SetTargetFPS(60);
    initSettings(screenWidth, screenHeight);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 0);
}
void runClient(int screenWidth, int screenHeight, char** argv) {
    /* Variable */
    // Packet Parsing
    PacketParsingData parsingData;
    // State management
    ClientCoreState oldState;

    /* INIT */
    // GUI
    initClientGui(screenWidth, screenHeight);
    // Packet parsing
    initMessageParsing(parsingData);
    // State is a global variable
    clientCoreState = ClientCoreState::SERVER_CONNECTION;
    oldState = clientCoreState;
    initGuiServerInputData();

    /* MAIN LOOP */
    while(!WindowShouldClose()) {
        if(oldState != clientCoreState)
            initState(oldState);
        oldState = clientCoreState;
        updateState();
        updateNetwork(parsingData);
        BeginDrawing();
        ClearBackground(GRAY);
        drawState();
        EndDrawing();
    }

    CloseWindow();
}
