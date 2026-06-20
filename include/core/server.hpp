#ifndef CORE_SERVER_HPP
#define CORE_SERVER_HPP

#include "api.hpp"
#include "db.hpp"
#include "network_utils.hpp"
#include "packet_parsing.hpp"
#include "user.hpp"
#include <list>
#include <unordered_map>

constexpr int MAX_EVENTS = 255;

struct ServerCoreConfig {
    // DB
    sqlite3 *db;
    // FD
    int fd;
    // Epoll
    struct epoll_event events[MAX_EVENTS];
    int epoll_fd;
    // TLS config
    bool tls;
    uint16_t minTlsVersion;
    SSL_CTX *ctx;
};
extern ServerCoreConfig serverConfig;

struct ClientSession {
    Connection connection;
    bool connected;
    uint32_t userId;
    std::string username;
    PacketParsingData parsingData;
    uint8_t msgBuffer[SIZE_PACKET_READ];
};
extern std::list<ClientSession> clients;
extern std::unordered_map<int, std::list<ClientSession>::iterator> fdToSession; // fd -> Client
extern std::unordered_map<uint32_t, std::list<ClientSession>::iterator> idToSession; // id -> Client

void sendChatSync(const ClientSession& session, uint32_t chatId);
void broadcastNetMsg(uint32_t chatId, NetworkMessage &netMsg);

// I honnestly don't remember what I was trying to solve with those templates
// I keep it because it should help performance that the NetworkMessage build is deffered after checking if 
// the session is connected
// But not sure it actually improve the perf in any significant matter
// TODO: Profile a template vs regular passing NetworkMessage ref version on a busy server
template <typename Builder>
void sendNetMsg(const ClientSession& session, Builder &&builder) {
    if(!session.connected)
        return;
    NetworkMessage msg = builder();
    netMsgWriteSize(msg);
    netMsgSend(session.connection, msg);
}

template <typename Builder>
void sendNetMsgIfConnected(uint32_t userId, Builder &&builder) {
    auto it = idToSession.find(userId);
    if(it != idToSession.end()) {
        ClientSession& session = *it->second;
        NetworkMessage msg = builder();
        netMsgWriteSize(msg);
        netMsgSend(session.connection, msg);
    }
}

#endif
