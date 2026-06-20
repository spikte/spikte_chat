#ifndef CORE_DB_HPP
#define CORE_DB_HPP

#include "chat.hpp"
#include "password_hash.hpp"
#include <cstdint>
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <cstring>
#include <openssl/crypto.h>

constexpr int BATCH_SIZE_MESSAGE = 100; // TODO: bad name fo this variable, rename it
constexpr int PASSWORD_SALT_LEN = 32;
constexpr int PASSWORD_HASH_LEN = 128;

// Create tables
int createTableUsers(sqlite3 *db);
int createTableChats(sqlite3 *db);
int createTableChatRoles(sqlite3 *db);
int createTableChatMembers(sqlite3 *db);
int createTableChatMessages(sqlite3 *db);
void createTables(sqlite3 *db);
// Query
bool checkUser(sqlite3 *db, const std::string &name, const std::string &password, uint32_t *userId);
int getUserId(sqlite3 *db, const std::string &name, uint32_t *userId);
int getUserName(sqlite3 *db, uint32_t userId, std::string* name);
int getUserRole(sqlite3 *db, uint32_t chatId, uint32_t userId, uint8_t *userRole);
int getAllUserIdByChatId(sqlite3 *db, uint32_t chatId, std::vector<uint32_t>& userIds);
int getAllChatIdByUserId(sqlite3 *db, uint32_t userId, std::vector<uint32_t>& chatIds);
int getChatBasicData(sqlite3 *db, uint32_t chatId, Chat &chat);
int getChatMembers(sqlite3 *db, uint32_t chatId, Chat &chat);
int getChatMessages(sqlite3 *db, uint32_t chatId, uint32_t lastMessageId, std::vector<Message>& messages);
// Inserts
int insertUser(sqlite3 *db, const std::string &name, const std::string &password);
int insertUserServer(sqlite3 *db);
int insertChat(sqlite3 *db, const std::string &name, uint8_t theme);
int insertChatRole(sqlite3 *db, uint8_t roleId, const std::string &roleName);
int insertChatMember(sqlite3 *db, uint32_t chatId, uint32_t userId, uint8_t role);
int insertChatMessage(sqlite3 *db, uint32_t chatId, uint32_t userId, const std::string &content, uint32_t timestamp);
// Update
int updateUserName(sqlite3 *db, uint32_t userId,const std::string &name);
int updateChatName(sqlite3 *db, uint32_t chatId,const std::string &name);
int updateUserRole(sqlite3 *db, uint32_t chatId, uint32_t userId, uint8_t role);
int updateChatMessage(sqlite3* db, uint32_t messageId, const std::string &content);
int updateChatTheme(sqlite3* db, uint32_t chatId, uint8_t theme);
// Delete
int deleteUser(sqlite3 *db, uint32_t userId);
int deleteChat(sqlite3 *db, uint32_t chatId);
int deleteChatMember(sqlite3 *db, uint32_t chatId, uint32_t userId);
int deleteChatMessage(sqlite3 *db, uint32_t messageId);
// Utils
sqlite3 *getDb(bool init = false);

#endif
