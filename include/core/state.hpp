#ifndef CORE_STATE_HPP
#define CORE_STATE_HPP

enum class ClientCoreState {
    SERVER_CONNECTION,
    WAIT_SERVER_CONNECTION,
    SIGNIN,
    WAIT_SIGNIN,
    LOGIN,
    WAIT_LOGIN,
    CHAT,
    CHAT_CREATION,
    WAIT_CHAT_CREATION,
    WAIT_CHAT_MESSAGE,
    WAIT_COMMAND_PROCESS
};
extern ClientCoreState clientCoreState;

#endif
