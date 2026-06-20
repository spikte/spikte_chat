#ifndef CORE_NETWORK_UTILS_HPP
#define CORE_NETWORK_UTILS_HPP

#include "api.hpp"

template <typename Builder>
void sendNetMsg(const Connection &connection, Builder &&builder) {
    NetworkMessage msg = builder();
    netMsgWriteSize(msg);
    netMsgSend(connection, msg);
}


#endif
