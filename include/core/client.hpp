#ifndef CORE_CLIENT_HPP
#define CORE_CLIENT_HPP

#include "chat.hpp"
#include "api.hpp"
#include <vector>
#include <list>
#include <unordered_map>
#include <openssl/ssl.h>
#include <fcntl.h>


struct UserCoreConfig {
    std::string serverAddr;
    std::string serverPort;
    Connection connection;
    SSL_CTX *ctx;
    uint16_t minTlsVersion;
    uint32_t serverId;
    std::list<Chat> loadedChat;
    std::unordered_map<uint32_t, std::list<Chat>::iterator> idToChat;
};
extern UserCoreConfig clientConfig;

int connectToServer(const char* addr, const char* port, Connection& connection, int min_tls_version);

#endif
