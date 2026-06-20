#ifndef CORE_API_HPP
#define CORE_API_HPP

#include "../../lib/socket_utils.h"
#include "chat.hpp"
#include "../utils/convert.hpp"
#include <arpa/inet.h>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

void packetWriteU8(uint8_t *packet, uint16_t *offset, uint8_t data);
void packetWriteU16(uint8_t *packet, uint16_t *offset, uint16_t data);
void packetWriteU32(uint8_t *packet, uint16_t *offset, uint32_t data);
void packetWriteU64(uint8_t *packet, uint16_t *offset, uint64_t data);
void packetWriteString(uint8_t *packet, uint16_t *offset, std::string data);
void packetFill(uint8_t *packet, uint16_t *offset, char c, uint8_t n);

uint8_t packetReadU8(uint8_t *packet, uint16_t *offset);
uint16_t packetReadU16(uint8_t *packet, uint16_t *offset);
uint32_t packetReadU32(uint8_t *packet, uint16_t *offset);
uint64_t packetReadU64(uint8_t *packet, uint16_t *offset);
std::string packetReadString(uint8_t *packet, uint16_t *offset, bool skipPadding = false, int maxSize = -1);
std::string packetReadStringN(uint8_t *packet, uint16_t *offset, size_t n);

// Abstract the connection, handle both TCP with and without TLS
struct Connection {
    int fd;
    SSL *ssl;
    bool tls;
};

// Bidirectional message, i think for a small chat app it's enough
enum class NetworkMessageType : uint8_t {
    QUIT = 0,
    SIGNIN = 1,
    LOGIN,
    CHAT,
    CHAT_MEMBER,
    CHAT_MESSAGE,
    CHAT_COLOR,
    CHAT_THEME,
    CHAT_SYNC_CHAT,
    CHAT_SYNC_MEMBER,
    CHAT_SYNC_MESSAGE
};
enum class NetworkMessageAction : uint8_t {
    NONE = 0,
    ADD = 1,
    DEL,
    SET,
    GET
};
enum class NetworkMessageStatus : uint8_t {
    NONE = 0,
    SUCCESS = 1,
    UNEXPECTED,
    USER_EXIST,
    USER_NOT_IN_DB,
    USER_NOT_IN_CHAT,
    FORBIDDEN,
    NOTHING_TO_DO,
    INVALID_ARGUMENT,
    INVALID_ACTION,
    INTERNAL_FAILURE
};

constexpr uint8_t MAGIC_BYTE = 0xAA;
using NetworkMessageSize = uint16_t;

constexpr int NM_HEADER_SIZE = sizeof(NetworkMessageType) + sizeof(NetworkMessageAction) + sizeof(NetworkMessageStatus) + sizeof(NetworkMessageSize);
constexpr int NM_MAX_SIZE = NM_HEADER_SIZE + std::numeric_limits<NetworkMessageSize>::max();
constexpr int MAX_SIZE_CHAT_MESSAGE = NM_MAX_SIZE - NM_HEADER_SIZE;

struct NetworkMessage {
    NetworkMessageType type;
    NetworkMessageAction action;
    NetworkMessageStatus status;
    uint16_t size;
    uint8_t payload[NM_MAX_SIZE - NM_HEADER_SIZE];
    uint16_t offset;
};
using NMType = NetworkMessageType;
using NMAction = NetworkMessageAction;
using NMStatus = NetworkMessageStatus;

NetworkMessage packetToNetMsg(uint8_t buffer[NM_MAX_SIZE]);
constexpr int MAX_SIZE_USERNAME = 64;
constexpr int MAX_SIZE_CHAT_NAME = 64;

void netMsgInit(NetworkMessage &netMsg);
void netMsgWriteU8(NetworkMessage &netMsg, uint8_t data);
void netMsgWriteU16(NetworkMessage &netMsg, uint16_t data);
void netMsgWriteU32(NetworkMessage &netMsg, uint32_t data);
void netMsgWriteU64(NetworkMessage &netMsg, uint64_t data);
void netMsgWriteString(NetworkMessage &netMsg, std::string data);
void netMsgFill(NetworkMessage &netMsg, char c, uint8_t n);
void netMsgWriteSize(NetworkMessage &netMsg);
ssize_t netMsgSend(const Connection &connection, const NetworkMessage &netMsg);

/* User */
int connectToServer(const char *addr, const char *port);
int connectToServerTLS(const char *addr, const char *port, SSL_CTX **ctx, SSL **ssl);
// Auth
NetworkMessage buildNetMsgRequestAuth(std::string name, std::string password, NetworkMessageType type);
NetworkMessage buildNetMsgRequestSignIn(std::string pseudo, std::string password);
NetworkMessage buildNetMsgRequestLogin(std::string pseudo, std::string password);
// Chat request
// // Create/delete a chat
NetworkMessage buildNetMsgRequestChatAdd(const Chat &chat);
NetworkMessage buildNetMsgRequestChatDel(const Chat &chat);
// // Add/delete/modify a chat member
NetworkMessage buildNetMsgRequestChatMemberAdd(uint32_t chatId, std::string name, const ChatRole &role);
NetworkMessage buildNetMsgRequestChatMemberDel(uint32_t chatId, std::string name);
NetworkMessage buildNetMsgRequestChatMemberSet(uint32_t chatId, std::string name, const ChatRole &role);
// // Add/delete/modigy a chat message
NetworkMessage buildNetMsgRequestChatMessageAdd(uint32_t chatId, const Message &message);
NetworkMessage buildNetMsgRequestChatMessageSet(uint32_t chatId, const Message &message);
NetworkMessage buildNetMsgRequestChatMessageDel(uint32_t chatId, const Message &message);
// // Change chat color settings
NetworkMessage buildNetMsgRequestChatColorSet(uint32_t chatId, ChatColorType colorType, uint32_t color);
NetworkMessage buildNetMsgRequestChatThemeSet(uint32_t chatId, uint8_t color);
// // Quit
NetworkMessage buildNetMsgRequestQuit(int fd);

/* Server */
// ANSWER
// // SignIn
NetworkMessage buildNetMsgAnswerSignInSuccessful(uint32_t id);
// // Login
NetworkMessage buildNetMsgAnswerLoginSuccessful(uint32_t id);
// // General
NetworkMessage buildNetMsgAnswer(NMType type, NMStatus status);

// SYNCHRONISATION
// // Create/delete/modify a chat
NetworkMessage buildNetMsgSyncChatAdd(const Chat &chat);
NetworkMessage buildNetMsgSyncChatSet(const Chat &chat);
NetworkMessage buildNetMsgSyncChatDel(uint32_t chatId);
// // Create/delete/modify a chat member
NetworkMessage buildNetMsgSyncMemberAdd(uint32_t chatId, const User &user, ChatRole role);
NetworkMessage buildNetMsgSyncMemberSet(uint32_t chatId, const User &user, ChatRole role);
NetworkMessage buildNetMsgSyncMemberDel(uint32_t chatId, uint32_t userId);
// // Create/delete/modify a chat message
NetworkMessage buildNetMsgSyncMessageAdd(uint32_t chatId, const Message &message);
NetworkMessage buildNetMsgSyncMessageSet(uint32_t chatId, const Message &message);
NetworkMessage buildNetMsgSyncMessageDel(uint32_t chatId, uint32_t messageId);

/* Debug utils */
constexpr char *DEBUG_NMTypeToStr(NMType type);
constexpr char *DEBUG_NMActionToStr(NMAction action);
constexpr char *DEBUG_NMStatusToStr(NMStatus status);
void DEBUG_printNetworkMessage(uint8_t buffer[NM_MAX_SIZE], const NetworkMessage &netMsg);

#endif
