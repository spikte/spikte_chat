#ifndef RUN_SERVER_HPP
#define RUN_SERVER_HPP

#include "../lib/socket_utils.h"
#include "core/api.hpp"
#include "core/db.hpp"
#include "core/packet_parsing.hpp"
#include "core/server.hpp"
#include "core/tls.hpp"
#include "core/user.hpp"
#include "utils/convert.hpp"
#include "utils/time.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <string>
#include <sys/epoll.h>

void writeMessageAdmin(uint32_t chatId, std::string content);

/* Handle client message */
// Auth request
void handleClientMessageSignIn(int fd, NetworkMessage &netMsg);
void handleClientMessageLogin(int fd, NetworkMessage &netMsg);
// Chat request
// // Create/delete/modify a chat
void handleClientMessageChatAdd(ClientSession &session, NetworkMessage &netMsg);
void handleClientMessageChatDel(ClientSession &session, NetworkMessage &netMsg);
void handleClientMessageChat(ClientSession &session, NetworkMessage &netMsg);
// // Add/delete/modify a chat member
void handleClientMessageChatMemberAdd(ClientSession &session, NetworkMessage &netMsg);
void handleClientMessageChatMemberSet(ClientSession &session, NetworkMessage &netMsg);
void handleClientMessageChatMemberDel(ClientSession &session, NetworkMessage &netMsg);
void handleClientMessageChatMember(ClientSession &session, NetworkMessage &netMsg);
// // Add/delete/modigy a chat message
void handleClientMessageChatMessageAdd(ClientSession &session, NetworkMessage &netMsg);
void handleClientMessageChatMessageSet(ClientSession &session, NetworkMessage &netMsg);
void handleClientMessageChatMessageDel(ClientSession &session, NetworkMessage &netMsg);
void handleClientMessageChatMessage(ClientSession &session, NetworkMessage &netMsg);
// // Change chat color settings
void handleClientMessageChatColorSet(ClientSession &session, NetworkMessage &netMsg);
void handleClientMessageChatColor(ClientSession &session, NetworkMessage &netMsg);
// // Quit
void handleClientMessageChatQuit(ClientSession &session, NetworkMessage &netMsg);

void handleClientMessage(ClientSession &session);

void disconnectClient(int fd);

/* Init epoll */
int initEpoll();
/* Process client connection */
void processClientConnection(int fd);
void processClientConnectionTLS(int fd);
void processClientConnectionTLSHandshake(ClientSession &session);
/* Process client communication */
void processClientCommunication(ClientSession &session);

int runServer(int argc, char *argv[]);

#endif
