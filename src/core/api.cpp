#include "../../include/core/api.hpp"


void packetWriteU8(uint8_t *packet, uint16_t *offset, uint8_t data) {
    packet[*offset] = data;
    (*offset)++;
}
void packetWriteU16(uint8_t *packet, uint16_t *offset, uint16_t data) {
    for(size_t i = 0; i < 2; i++)
        packet[(*offset) + i] = (data >> ((1 - i) * 8)) & 0xFF;
    (*offset) += 2;
}
void packetWriteU32(uint8_t *packet, uint16_t *offset, uint32_t data) {
    for(size_t i = 0; i < 4; i++)
        packet[(*offset) + i] = (data >> ((3 - i) * 8)) & 0xFF;
    (*offset) += 4;
}
void packetWriteU64(uint8_t *packet, uint16_t *offset, uint64_t data) {
    for(size_t i = 0; i < 8; i++)
        packet[(*offset) + i] = (data >> ((7 - i) * 8)) & 0xFF;
    (*offset) += 8;
}
void packetWriteString(uint8_t *packet, uint16_t *offset, std::string data) {
    std::memcpy(packet + *offset, data.c_str(), data.size());
    (*offset) += data.size();
}
void packetFill(uint8_t *packet, uint16_t *offset, char c, uint8_t n) {
    std::memset(packet + *offset, c, n);
    (*offset) += n;
}

uint8_t packetReadU8(uint8_t *packet, uint16_t *offset) {
    uint8_t data;

    data = packet[*offset];
    (*offset)++;

    return data;
}
uint16_t packetReadU16(uint8_t *packet, uint16_t *offset) {
    uint16_t data;

    data = bytesToU16(packet + *offset);
    (*offset) += 2;

    return data;
}
uint32_t packetReadU32(uint8_t *packet, uint16_t *offset) {
    uint32_t data;

    data = bytesToU32(packet + *offset);
    (*offset) += 4;

    return data;
}
uint64_t packetReadU64(uint8_t *packet, uint16_t *offset) {
    uint64_t data;

    data = bytesToU64(packet + *offset);
    (*offset) += 8;

    return data;
}
// A bit unsafe but skipPadding is only to be used when data is present after
std::string packetReadString(uint8_t *packet, uint16_t *offset, bool skipPadding, int maxSize) {
    uint16_t start;
    std::string data;

    start  = *offset;
    data = std::string((char*)packet + *offset);
    (*offset) += data.size();
    if(skipPadding) {
        while(packet[*offset] == '\0') {
            (*offset)++;
            if(maxSize >= 0 && *offset - start >= maxSize)
                break;
        }
    }

    return data;
}
std::string packetReadStringN(uint8_t *packet, uint16_t *offset, size_t n) {
    std::string data;

    data = std::string((char*)packet + *offset, n);
    (*offset) += n;

    return data;
}


NetworkMessage packetToNetMsg(uint8_t buffer[NM_MAX_SIZE]) {
    NetworkMessage netMsg;
    uint16_t offset;

    offset = 0;
    netMsg.type = static_cast<NMType>(buffer[offset++]);
    netMsg.action = static_cast<NMAction>(buffer[offset++]);
    netMsg.status = static_cast<NMStatus>(buffer[offset++]);
    netMsg.size = packetReadU16(buffer, &offset);
    std::memcpy(netMsg.payload, buffer + offset, netMsg.size - NM_HEADER_SIZE);
    
    return netMsg;
}
void netMsgInit(NetworkMessage& netMsg) {
    netMsg.offset = 0;
}
void netMsgWriteU8(NetworkMessage& netMsg, uint8_t data) { packetWriteU8(netMsg.payload, &netMsg.offset, data); }
void netMsgWriteU16(NetworkMessage& netMsg, uint16_t data) { packetWriteU16(netMsg.payload, &netMsg.offset, data); }
void netMsgWriteU32(NetworkMessage& netMsg, uint32_t data) { packetWriteU32(netMsg.payload, &netMsg.offset, data); }
void netMsgWriteU64(NetworkMessage& netMsg, uint64_t data) { packetWriteU64(netMsg.payload, &netMsg.offset, data); }
void netMsgWriteString(NetworkMessage& netMsg, std::string data) { packetWriteString(netMsg.payload, &netMsg.offset, data); }
void netMsgFill(NetworkMessage& netMsg, char c, uint8_t n) { packetFill(netMsg.payload, &netMsg.offset, c, n); };
void netMsgWriteSize(NetworkMessage& netMsg) {
    netMsg.size = netMsg.offset + sizeof(NMType) + sizeof(NMAction) + sizeof(NMStatus) + sizeof(NetworkMessageSize);
}
static ssize_t sendBytes(int fd, uint8_t* bytes, size_t n, int flags) {
    ssize_t nBytes;
    ssize_t totalBytes;

    totalBytes = 0;
    while(totalBytes < n) {
        nBytes = send(fd, bytes + totalBytes, n - totalBytes, 0);
        if(nBytes == -1) {
            return -1;
        }
        totalBytes += nBytes;
    }
    return totalBytes;
}
static ssize_t sendBytesSSL(SSL *ssl, uint8_t *bytes, size_t n) {
    ssize_t nBytes;
    ssize_t totalBytes;

    totalBytes = 0;
    while(totalBytes < n) {
        nBytes = SSL_write(ssl, bytes + totalBytes, n - totalBytes);
        if(nBytes < -1) {
            switch(SSL_get_error(ssl, nBytes)) {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                    continue;
                case SSL_ERROR_SYSCALL:
                case SSL_ERROR_SSL:
                    if (SSL_get_verify_result(ssl) != X509_V_OK)
                        fprintf(stderr, "Verify error: %s\n", X509_verify_cert_error_string(SSL_get_verify_result(ssl)));
                default:
                    return -1;
            }
        }
        totalBytes += nBytes;
    }
    return totalBytes;
}
ssize_t netMsgSend(const Connection& connection, const NetworkMessage& netMsg) {
    uint8_t packet[NM_MAX_SIZE];
    ssize_t nBytes;
    uint16_t offset;

    // Write header
    offset = 0;
    packet[offset++] = MAGIC_BYTE;
    packet[offset++] = static_cast<uint8_t>(netMsg.type);
    packet[offset++] = static_cast<uint8_t>(netMsg.action);
    packet[offset++] = static_cast<uint8_t>(netMsg.status);
    packetWriteU16(packet, &offset, netMsg.size);
    std::memcpy(packet + offset, netMsg.payload, netMsg.offset);

    if(connection.tls)
        nBytes = sendBytesSSL(connection.ssl, packet, netMsg.size + 1);
    else
        nBytes = sendBytes(connection.fd, packet, netMsg.size + 1, 0);
    if(nBytes < 0) {
        std::fprintf(stderr, "Error while sending packet [%u, %u, %u, %u].\n",
                     packet[0],
                     packet[1],
                     packet[2],
                     netMsg.size);
    }
    return nBytes;
}



/* User */
NetworkMessage buildNetMsgRequestAuth(std::string name, std::string password, NMType type) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = type;
    netMsg.action = NMAction::ADD;
    netMsg.status = NMStatus::NONE;
    netMsgWriteString(netMsg, name);
    netMsgFill(netMsg, '\0', MAX_SIZE_USERNAME - name.size());
    netMsgWriteString(netMsg, password);
    netMsgFill(netMsg, '\0', MAX_SIZE_USERNAME - password.size());
    return netMsg;
}
NetworkMessage buildNetMsgRequestSignIn(std::string name, std::string password) {
    return buildNetMsgRequestAuth(name, password, NMType::SIGNIN);
}
NetworkMessage buildNetMsgRequestLogin(std::string name, std::string password) {
    return buildNetMsgRequestAuth(name, password, NMType::LOGIN);
}
NetworkMessage buildNetMsgRequestChatAdd(const Chat& chat) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT;
    netMsg.action = NMAction::ADD;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU8(netMsg, static_cast<uint8_t>(chat.theme));
    netMsgWriteString(netMsg, chat.name);
    netMsgFill(netMsg, '\0', MAX_SIZE_CHAT_NAME - chat.name.size());
    return netMsg;
}
NetworkMessage buildNetMsgRequestChatDel(uint32_t chatId) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT;
    netMsg.action = NMAction::DEL;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    return netMsg;
}
NetworkMessage buildNetMsgRequestChatMemberAdd(uint32_t chatId, std::string name, const ChatRole& role) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_MEMBER;
    netMsg.action = NMAction::ADD;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteString(netMsg, name);
    netMsgFill(netMsg, '\0', MAX_SIZE_USERNAME - name.size());
    netMsgWriteU8(netMsg, static_cast<uint8_t>(role));
    return netMsg;
}
NetworkMessage buildNetMsgRequestChatMemberDel(uint32_t chatId, std::string name) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_MEMBER;
    netMsg.action = NMAction::DEL;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteString(netMsg, name);
    netMsgFill(netMsg, '\0', MAX_SIZE_USERNAME - name.size());
    return netMsg;
}
NetworkMessage buildNetMsgRequestChatMemberSet(uint32_t chatId, std::string name, const ChatRole& role) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_MEMBER;
    netMsg.action = NMAction::SET;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteString(netMsg, name);
    netMsgFill(netMsg, '\0', MAX_SIZE_USERNAME - name.size());
    netMsgWriteU8(netMsg, static_cast<uint8_t>(role));
    return netMsg;
}
NetworkMessage buildNetMsgRequestChatMessageAdd(uint32_t chatId, const Message& message) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);
    const std::string &author = message.author.name;

    netMsg.type = NMType::CHAT_MESSAGE;
    netMsg.action = NMAction::ADD;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteU32(netMsg, message.timestamp);
    netMsgWriteString(netMsg, message.content);
    return netMsg;
}
NetworkMessage buildNetMsgRequestChatMessageSet(uint32_t chatId, const Message& message) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);
    const std::string &author = message.author.name;

    netMsg.type = NMType::CHAT_MESSAGE;
    netMsg.action = NMAction::SET;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteU32(netMsg, message.id);
    netMsgWriteString(netMsg, author.c_str());
    netMsgFill(netMsg, '\0', MAX_SIZE_USERNAME - author.size());
    netMsgWriteU32(netMsg, message.timestamp);
    netMsgWriteString(netMsg, message.content);
    return netMsg;
}
NetworkMessage buildNetMsgRequestChatMessageDel(uint32_t chatId, uint32_t messageId) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_MESSAGE;
    netMsg.action = NMAction::DEL;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteU32(netMsg, messageId);
    return netMsg;
}
NetworkMessage buildNetMsgRequestChatColorSet(uint32_t chatId, ChatColorType colorType, uint32_t color) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_COLOR;
    netMsg.action = NMAction::SET;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteU8(netMsg, static_cast<uint8_t>(colorType));
    netMsgWriteU32(netMsg, color);
    return netMsg;
}
NetworkMessage buildNetMsgRequestChatThemeSet(uint32_t chatId, uint8_t theme) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_THEME;
    netMsg.action = NMAction::SET;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteU8(netMsg, theme);
    return netMsg;
}
NetworkMessage buildNetMsgRequestQuit(uint32_t userId) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::QUIT;
    netMsg.action = NMAction::ADD;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, userId);
    return netMsg;
}

/* Server */
NetworkMessage buildNetMsgAnswerSignInSuccessful(uint32_t userId) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::SIGNIN;
    netMsg.action = NMAction::NONE;
    netMsg.status = NMStatus::SUCCESS;
    netMsgWriteU32(netMsg, userId);
    return netMsg;
}
NetworkMessage buildNetMsgAnswerLoginSuccessful(uint32_t userId) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::LOGIN;
    netMsg.action = NMAction::NONE;
    netMsg.status = NMStatus::SUCCESS;
    netMsgWriteU32(netMsg, userId);
    return netMsg;
}
NetworkMessage buildNetMsgAnswer(NMType type, NMStatus status) {
    if((type == NMType::SIGNIN || type == NMType::LOGIN) && status == NMStatus::SUCCESS) {
        std::fprintf(stderr, "Error: wrong function used for SIGNIN/LOGIN SUCCESS message. Please use `buildNetMsgAnswer*Auth*Successful over `buildNetMSgAnswer``\n");
        return {.status=NMStatus::INTERNAL_FAILURE};
    }
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = type;
    netMsg.action = NMAction::NONE;
    netMsg.status = status;
    return netMsg;
}

NetworkMessage buildNetMsgSyncChatAdd(const Chat& chat) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_SYNC_CHAT;
    netMsg.action = NMAction::ADD;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chat.id);
    netMsgWriteU8(netMsg, static_cast<uint8_t>(chat.theme));
    netMsgWriteString(netMsg, chat.name);
    netMsgFill(netMsg, '\0', MAX_SIZE_CHAT_NAME - chat.name.size());
    return netMsg;
}
NetworkMessage buildNetMsgSyncChatSet(const Chat& chat) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_SYNC_CHAT;
    netMsg.action = NMAction::SET;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chat.id);
    netMsgWriteU8(netMsg, static_cast<uint8_t>(chat.theme));
    netMsgWriteString(netMsg, chat.name);
    netMsgFill(netMsg, '\0', MAX_SIZE_CHAT_NAME - chat.name.size());
    return netMsg;
}
NetworkMessage buildNetMsgSyncChatDel(uint32_t chatId) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_SYNC_CHAT;
    netMsg.action = NMAction::DEL;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    return netMsg;
}

NetworkMessage buildNetMsgSyncMemberAdd(uint32_t chatId, const User& user, ChatRole role) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_SYNC_MEMBER;
    netMsg.action = NMAction::ADD;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteU32(netMsg, user.id);
    netMsgWriteU8(netMsg, static_cast<uint8_t>(role));
    netMsgWriteString(netMsg, user.name);
    netMsgFill(netMsg, '\0', MAX_SIZE_USERNAME - user.name.size());
    return netMsg;
}
NetworkMessage buildNetMsgSyncMemberSet(uint32_t chatId, const User& user, ChatRole role) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_SYNC_MEMBER;
    netMsg.action = NMAction::SET;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteU32(netMsg, user.id);
    netMsgWriteU8(netMsg, static_cast<uint8_t>(role));
    netMsgWriteString(netMsg, user.name);
    netMsgFill(netMsg, '\0', MAX_SIZE_USERNAME - user.name.size());
    return netMsg;
}
NetworkMessage buildNetMsgSyncMemberDel(uint32_t chatId, uint32_t userId) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_SYNC_MEMBER;
    netMsg.action = NMAction::DEL;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteU32(netMsg, userId);
    return netMsg;
}

NetworkMessage buildNetMsgSyncMessageAdd(uint32_t chatId, const Message& message) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_SYNC_MESSAGE;
    netMsg.action = NMAction::ADD;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteU32(netMsg, message.author.id);
    netMsgWriteString(netMsg, message.author.name);
    netMsgFill(netMsg, '\0', MAX_SIZE_USERNAME - message.author.name.size());
    netMsgWriteU32(netMsg, message.id);
    netMsgWriteU32(netMsg, message.timestamp);
    netMsgWriteString(netMsg, message.content);
    return netMsg;
}
NetworkMessage buildNetMsgSyncMessageSet(uint32_t chatId, const Message& message) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_SYNC_MESSAGE;
    netMsg.action = NMAction::SET;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteU32(netMsg, message.id);
    netMsgWriteU32(netMsg, message.author.id);
    netMsgWriteU32(netMsg, message.timestamp);
    netMsgWriteU32(netMsg, message.author.id);
    netMsgWriteString(netMsg, message.author.name);
    netMsgFill(netMsg, '\0', MAX_SIZE_USERNAME - message.author.name.size());
    netMsgWriteString(netMsg, message.content);
    return netMsg;
}
NetworkMessage buildNetMsgSyncMessageDel(uint32_t chatId, uint32_t messageId) {
    NetworkMessage netMsg;
    netMsgInit(netMsg);

    netMsg.type = NMType::CHAT_SYNC_MESSAGE;
    netMsg.action = NMAction::DEL;
    netMsg.status = NMStatus::NONE;
    netMsgWriteU32(netMsg, chatId);
    netMsgWriteU32(netMsg, messageId);
    return netMsg;
}


/* Debug utils (Not ISO complient, the compiler is not happy about it) */
constexpr char* DEBUG_NMTypeToStr(NMType type) {
    switch (type) {
        case NMType::QUIT:                 return "QUIT";
        case NMType::SIGNIN:               return "SIGNIN";
        case NMType::LOGIN:                return "LOGIN";
        case NMType::CHAT:                 return "CHAT";
        case NMType::CHAT_MEMBER:          return "CHAT_MEMBER";
        case NMType::CHAT_MESSAGE:         return "CHAT_MESSAGE";
        case NMType::CHAT_COLOR:           return "CHAT_COLOR";
        case NMType::CHAT_THEME:           return "CHAT_THEME";
        case NMType::CHAT_SYNC_CHAT:       return "CHAT_SYNC_CHAT";
        case NMType::CHAT_SYNC_MEMBER:     return "CHAT_SYNC_MEMBER";
        case NMType::CHAT_SYNC_MESSAGE:    return "CHAT_SYNC_MESSAGE";
        default:                           return "UNKNOWN";
    }
}

constexpr char* DEBUG_NMActionToStr(NMAction status) {
    switch (status) {
        case NMAction::NONE:    return "NONE";
        case NMAction::ADD:     return "ADD";
        case NMAction::DEL:     return "DEL";
        case NMAction::SET:     return "SET";
        case NMAction::GET:     return "GET";
        default:                return "UNKNOWN";
    }
}

constexpr char* DEBUG_NMStatusToStr(NMStatus error) {
    switch (error) {
        case NMStatus::NONE:             return "NONE";
        case NMStatus::SUCCESS:          return "SUCCESS";
        case NMStatus::UNEXPECTED:       return "UNEXPECTED";
        case NMStatus::USER_EXIST:       return "USER_EXIST";
        case NMStatus::USER_NOT_IN_DB:   return "USER_NOT_IN_DB";
        case NMStatus::USER_NOT_IN_CHAT: return "USER_NOT_IN_CHAT";
        case NMStatus::FORBIDDEN:        return "FORBIDDEN";
        default:                         return "UNKNOWN";
    }
}


void DEBUG_printNetworkMessage(uint8_t buffer[NM_MAX_SIZE], const NetworkMessage& netMsg) {
    std::printf("[%s(%u)][%s(%u)][%s(%u)][%u(%u %u)][",
                DEBUG_NMTypeToStr(netMsg.type), buffer[0],
                DEBUG_NMActionToStr(netMsg.action), buffer[1],
                DEBUG_NMStatusToStr(netMsg.status), buffer[2],
                netMsg.size, buffer[3], buffer[4]);
    for(size_t i = 0; i < netMsg.size - NM_HEADER_SIZE; i++) {
        if(i == netMsg.size - NM_HEADER_SIZE - 1)
            std::printf("%u", netMsg.payload[i]);
        else
            std::printf("%u ", netMsg.payload[i]);
    }
    std::printf("]\n");
}
