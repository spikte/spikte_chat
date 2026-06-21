#ifndef RUN_CLIENT_HPP
#define RUN_CLIENT_HPP

#include "core/api.hpp"
#include "core/chat.hpp"
#include "core/client.hpp"
#include "core/network_utils.hpp"
#include "core/packet_parsing.hpp"
#include "core/state.hpp"
#include "core/tls.hpp"
#include "core/user.hpp"
#include "gui/chat.hpp"
#include "gui/client_auth.hpp"
#include "utils/convert.hpp"
#include "utils/time.hpp"
#include <list>


/* STATE INIT */
void initState(ClientCoreState prevState);
void drawState();
void updateStateWaitServerConnection();
void updateState();
void updateNetwork(PacketParsingData &parsingData);

/* HANDLE SERVER MESSAGE */
// Auth
void handleServerMessageSignIn(NetworkMessage &netMsg);
void handleServerMessageLogin(NetworkMessage &netMsg);
// Chat create response
void handleServerMessageChatAdd(NetworkMessage &netMsg);
// Chat sync
// Chat metadta
void handleServerMessageChatSyncChatAdd(NetworkMessage &netMsg);
void handleServerMessageChatSyncChatSet(NetworkMessage &netMsg);
void handleServerMessageChatSyncChatDel(NetworkMessage &netMsg);
void handleServerMessageChatSyncChat(NetworkMessage &netMsg);
// Chat members
void handleServerMessageChatSyncMemberAdd(NetworkMessage &netMsg);
void handleServerMessageChatSyncMemberSet(NetworkMessage &netMsg);
void handleServerMessageChatSyncMemberDel(NetworkMessage &netMsg);
void handleServerMessageChatSyncMember(NetworkMessage &netMsg);
// Chat message
void handleServerMessageChatSyncMessageAdd(NetworkMessage &netMsg);
void handleServerMessageChatSyncMessageSet(NetworkMessage &netMsg);
void handleServerMessageChatSyncMessageDel(NetworkMessage &netMsg);
void handleServerMessageChatSyncMessage(NetworkMessage &netMsg);

void handleServerMessage(uint8_t buffer[NM_MAX_SIZE]);

/* INIT */
void initClientGui(int screenWidth, int screenHeight);
void runClient(int screenWidth, int screenHeight, char *argv[]);

#endif
