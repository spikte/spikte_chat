#ifndef CORE_PROCESS_COMMAND_HPP
#define CORE_PROCESS_COMMAND_HPP

#include "api.hpp"
#include "chat.hpp"
#include "network_utils.hpp"
#include "state.hpp"
#include "user.hpp"
#include "client.hpp"
#include <string>
#include <vector>

enum class ProcessCommandErrorType {
    UNKOWN_COMMAND,
    INVALID_ARGUMENT,
};

bool processCommandHelp(uint32_t chatId, std::vector<std::string> command);
bool processCommandGetMembers(uint32_t chatId, std::vector<std::string> command);
bool processCommandAddMember(uint32_t chatId, std::vector<std::string> command);
bool processCommandDelMember(uint32_t chatId, std::vector<std::string> command);
bool processCommandSetRole(uint32_t chatId, std::vector<std::string> command);
bool processCommandSetColor(uint32_t chatId, std::vector<std::string> command);
bool processCommandSetTheme(uint32_t chatId, std::vector<std::string> command);
bool processCommand(uint32_t chatId, std::string commandStr);

#endif
