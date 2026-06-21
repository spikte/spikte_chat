#include "../../include/core/db.hpp"

// Statics
static void printSqliteError(sqlite3 *db, std::string funcName, int rc) {
    std::fprintf(stderr, "(SQLite[%s]: %d): %s\n", funcName.c_str(), rc, sqlite3_errmsg(db));
}
static int sqlite3Exec(sqlite3 *db, const std::string &query) {
    sqlite3_stmt *stmt;
    int rc;

    // Here i could use `sqlite3_exec` but from what i understand from the sqlite3 doc is that
    // sqlite3_exec return SQLITE_OK on success while sqlite3_step return SQLITE_DONE
    // And i prefer to have one code that means success
    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE)
        std::fprintf(stderr, "(SQLite[sqlite3Exec]: %d) Can't execute <%s>: %s\n", rc, query.c_str(), sqlite3_errmsg(db));
    sqlite3_finalize(stmt);

    return rc;
}

// Create tables
int createTableUsers(sqlite3 *db) {
    std::string query = "CREATE TABLE IF NOT EXISTS users ("
                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "name TEXT NOT NULL UNIQUE,"
                        "password_hash BLOB NOT NULL,"
                        "password_salt BLOB NOT NULL);";

    return sqlite3Exec(db, query);
}
int createTableChats(sqlite3 *db) {
    std::string query = "CREATE TABLE IF NOT EXISTS chats ("
                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "name TEXT NOT NULL,"
                        "theme INT NOT NULL);";
    return sqlite3Exec(db, query);
}
int createTableChatRoles(sqlite3 *db) {
    std::string query = "CREATE TABLE IF NOT EXISTS chatRoles ("
                        "id INTEGER PRIMARY KEY,"
                        "name TEXT NOT NULL);";
    return sqlite3Exec(db, query);
}
int createTableChatMembers(sqlite3 *db) {
    std::string query = "CREATE TABLE IF NOT EXISTS chatMembers ("
                        "chatId INTEGER NOT NULL,"
                        "userId INTEGER NOT NULL,"
                        "roleId INTEGER NOT NULL,"
                        "FOREIGN KEY(chatId) REFERENCES chats(id) ON DELETE CASCADE,"
                        "FOREIGN KEY(userId) REFERENCES users(id) ON DELETE CASCADE,"
                        "FOREIGN KEY(roleId) REFERENCES chatRoles(id),"
                        "PRIMARY KEY (chatId, userId));";
    return sqlite3Exec(db, query);
}
int createTableChatMessages(sqlite3 *db) {
    std::string query = "CREATE TABLE IF NOT EXISTS messages ("
                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "chatId INTEGER NOT NULL,"
                        "senderId INTEGER NOT NULL,"
                        "content TEXT NOT NULL,"
                        "timestamp INTEGER NOT NULL,"
                        "FOREIGN KEY(chatId) REFERENCES chats(id) ON DELETE CASCADE,"
                        "FOREIGN KEY(senderId) REFERENCES users(id));";
    return sqlite3Exec(db, query);
}
void createTables(sqlite3 *db) {
    createTableUsers(db);
    createTableChats(db);
    createTableChatRoles(db);
    createTableChatMembers(db);
    createTableChatMessages(db);
}
// Query
bool checkUser(sqlite3 *db, const std::string &name, const std::string &password, uint32_t* userId) {
    bool error;
    bool isUserValid;
    uint32_t tempId;
    std::string query;
    sqlite3_stmt *stmt;
    int rc;
    unsigned char passwordHashUser[PASSWORD_HASH_LEN];
    int passwordSaltDBLen;
    int passwordHashDBLen;
    const void* passwordSalt;
    const void* passwordHashDB;

    isUserValid = false;
    error = false;
    isUserValid = false;
    query = "SELECT id, password_salt, password_hash FROM users WHERE name = ?";
    stmt = nullptr;

    if(name == "SERVER")
        goto error;

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_ROW))) goto error;

    // Retrieve db hash and salt
    tempId = sqlite3_column_int(stmt, 0);
    passwordSaltDBLen = sqlite3_column_bytes(stmt, 1);
    passwordSalt = sqlite3_column_blob(stmt, 1);
    passwordHashDBLen = sqlite3_column_bytes(stmt, 2);
    passwordHashDB = sqlite3_column_blob(stmt, 2);
    if(!passwordSalt || !passwordHashDB ||
       passwordSaltDBLen != PASSWORD_SALT_LEN ||
       passwordHashDBLen != PASSWORD_HASH_LEN)  {
        error = true;
        goto error;
    }
    // Compute the user hash
    if(hashArgon2d((char*)password.c_str(), password.size(), (unsigned char*)passwordSalt, PASSWORD_SALT_LEN, passwordHashUser, PASSWORD_HASH_LEN) == EXIT_FAILURE)
        goto error;
    // Compare the two hash
    if(std::memcmp(passwordHashDB, passwordHashUser, PASSWORD_HASH_LEN) != 0)
        goto error;

    *userId = tempId;
    isUserValid = true;

error:
    if(stmt)
        sqlite3_finalize(stmt);
    if(error) {
        printSqliteError(db, "checkUser", rc);
        return false;
    }
    return isUserValid;
}
int getUserId(sqlite3 *db, const std::string &name, uint32_t *userId) {
    bool error;
    sqlite3_stmt *stmt;
    int rc;

    error = false;
    stmt = nullptr;

    rc = sqlite3_prepare_v2(db, "SELECT id FROM users WHERE name = ?;", -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_ROW))) goto error;

    *userId = sqlite3_column_int(stmt, 0);
error:
    if(error)
        printSqliteError(db, "getUserId", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int getUserName(sqlite3 *db, uint32_t userId, std::string* name) {
    bool error;
    sqlite3_stmt *stmt;
    int rc;
    const unsigned char* nameStr;

    error = false;
    stmt = nullptr;

    rc = sqlite3_prepare_v2(db, "SELECT name FROM users WHERE id = ?;", -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, userId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_ROW))) goto error;

    nameStr = sqlite3_column_text(stmt, 0);
    if((error = (nameStr == nullptr))) goto error;

    *name = reinterpret_cast<const char*>(nameStr);
error:
    if(error)
        printSqliteError(db, "getUserName", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int getUserRole(sqlite3 *db, uint32_t chatId, uint32_t userId, uint8_t *userRole) {
    bool error;
    sqlite3_stmt *stmt;
    int rc;

    error = false;
    stmt = nullptr;

    rc = sqlite3_prepare_v2(db, "SELECT roleId FROM chatMembers WHERE userId = ? AND chatId = ?;", -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, userId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 2, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_ROW))) goto error;

    *userRole = sqlite3_column_int(stmt, 0);

error:
    if(error)
        printSqliteError(db, "getUserRole", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int getAllUserIdByChatId(sqlite3 *db, uint32_t chatId, std::vector<uint32_t>& userIds) {
    bool error;
    sqlite3_stmt *stmt;
    int rc;

    error = false;
    stmt = nullptr;

    rc = sqlite3_prepare_v2(db, "SELECT id FROM users WHERE id IN (SELECT userId FROM chatMembers WHERE chatId = ?)", -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        userIds.push_back(id);
    }
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "getAllUserIdByChatId", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int getAllChatIdByUserId(sqlite3 *db, uint32_t userId, std::vector<uint32_t>& chatIds) {
    bool error;
    sqlite3_stmt *stmt;
    int rc;

    error = false;
    stmt = nullptr;

    rc = sqlite3_prepare_v2(db, "SELECT chatId FROM chatMembers WHERE userId = ?;", -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, userId);
    if((error = (rc != SQLITE_OK))) goto error;

    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        uint32_t id = sqlite3_column_int(stmt, 0);
        chatIds.push_back(id);
    }
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "getAllChatIdByUserId", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int getChatBasicData(sqlite3* db, uint32_t chatId, Chat& chat) {
    bool error;
    sqlite3_stmt *stmt;
    int rc;

    error = false;
    stmt = nullptr;

    rc = sqlite3_prepare_v2(db, "SELECT name, theme FROM chats WHERE id = ?;", -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_ROW))) goto error;

    chat.id = chatId;
    chat.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    chat.theme = static_cast<ChatTheme>(sqlite3_column_int(stmt, 1));
error:
    if(error)
        printSqliteError(db, "getChatBasicData", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
// Here the container is directly a Chat ref because i fill `chat.members` AND `chat.roles`
// But maybe I'll need only the user in the future without the Chat create so I'll have to
// implement another one that fill a std::vector<User>
int getChatMembers(sqlite3* db, uint32_t chatId, Chat& chat) {
    bool error;
    std::string query;
    sqlite3_stmt *stmt;
    int rc;

    error = false;
    query = "SELECT u.id, u.name, m.roleId FROM users u JOIN chatMembers m ON u.id = m.userId WHERE m.chatId = ?";
    stmt = nullptr;

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        User newUser;
        newUser.id = sqlite3_column_int(stmt, 0);
        newUser.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        ChatRole role = static_cast<ChatRole>(sqlite3_column_int(stmt, 2));
        chat.members.push_back(newUser);
        chat.roles[newUser.id] = role;
    }
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "getChatMembers", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int getChatNMembers(sqlite3 *db, uint32_t chatId, int* nMembers) {
    bool error;
    std::string query;
    sqlite3_stmt *stmt;
    int rc;

    error = false;
    query = "SELECT COUNT(u.id) FROM users u JOIN chatMembers m ON u.id = m.userId WHERE m.chatId = ?";
    stmt = nullptr;

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_ROW))) goto error;

    *nMembers = sqlite3_column_int(stmt, 0);

error:
    if(error)
        printSqliteError(db, "getChatMembers", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int getChatNewOwner(sqlite3 *db, uint32_t chatId, uint32_t *userId) {
    bool error;
    std::string query;
    sqlite3_stmt *stmt;
    int rc;

    error = false;
    query = "SELECT u.id FROM users u JOIN chatMembers m ON u.id = m.userId WHERE m.chatId = ? AND roleId = ?";
    stmt = nullptr;

    /* Select the first ADMIN */
    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 2, static_cast<uint8_t>(ChatRole::ADMIN));
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if(rc == SQLITE_ROW) {
        *userId = sqlite3_column_int(stmt, 0);
        goto error;
    }
    else
        if((error = (rc != SQLITE_DONE))) goto error;

    /* If no admin select rando user */
    sqlite3_finalize(stmt);
    query = "SELECT u.id FROM users u JOIN chatMembers m ON u.id = m.userId WHERE m.chatId = ?";
    stmt = nullptr;

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

    *userId = sqlite3_column_int(stmt, 0);

error:
    if(error)
        printSqliteError(db, "getChatMembers", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int getChatMessages(sqlite3 *db, uint32_t chatId, uint32_t lastMessageId, std::vector<Message>& messages) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    // I assume that bigger id = bigger timestamp, from the doc it seems true but maybe i'll get some bug, we'll see
    query = "SELECT m.id, m.senderId, u.name, m.content, m.timestamp FROM users u JOIN messages m ON u.id = m.senderId "
            "WHERE m.chatId = ? "
            "AND m.id < ? "
            "ORDER BY m.id ASC "
            "LIMIT ?;";
    stmt = nullptr;

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 2, lastMessageId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 3, BATCH_SIZE_MESSAGE);
    if((error = (rc != SQLITE_OK))) goto error;

    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        uint32_t id = sqlite3_column_int(stmt, 0);
        uint32_t senderId = sqlite3_column_int(stmt, 1);
        const unsigned char* nameText = sqlite3_column_text(stmt, 2);
        const unsigned char* contentText = sqlite3_column_text(stmt, 3);
        std::string name = nameText ? reinterpret_cast<const char*>(nameText) : "";
        std::string content = contentText ? reinterpret_cast<const char*>(contentText): "";
        uint32_t msgTimestamp = sqlite3_column_int(stmt, 4);
        User user;
        user.id = senderId;
        user.name = name;
        Message message{id, msgTimestamp, user, content};
        messages.push_back(message);
    }
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "getChatMessages", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}

// Inserts
int insertUser(sqlite3 *db, const std::string &name, const std::string &password) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;
    unsigned char passwordHash[PASSWORD_HASH_LEN];
    unsigned char passwordSalt[PASSWORD_SALT_LEN];


    RAND_bytes(passwordSalt, PASSWORD_SALT_LEN);
    if(hashArgon2d((char*)password.c_str(), password.size(), passwordSalt, PASSWORD_SALT_LEN, passwordHash, PASSWORD_HASH_LEN) == EXIT_FAILURE)
        return SQLITE_ERROR;
    stmt = nullptr;
    query = "INSERT INTO users(name, password_salt, password_hash) VALUES(? , ?, ?)";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_blob(stmt, 2, passwordSalt, PASSWORD_SALT_LEN, SQLITE_STATIC);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_blob(stmt, 3, passwordHash, PASSWORD_HASH_LEN, SQLITE_STATIC);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "insertUser", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int insertUserServer(sqlite3 *db) {
    std::string query;

    query = "INSERT INTO users(id, name, password_salt, password_hash) VALUES(1, 'SERVER' , '', '')";
    return sqlite3Exec(db, query);
}
int insertChat(sqlite3 *db, const std::string& name, uint8_t theme) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    stmt = nullptr;
    query = "INSERT INTO chats(name, theme) VALUES(?, ?)";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 2, theme);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "insertChat", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int insertChatRole(sqlite3 *db, uint8_t roleId, const std::string& roleName) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    stmt = nullptr;
    query = "INSERT INTO chatRoles(id, name) VALUES(?, ?)";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, roleId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_text(stmt, 2, roleName.c_str(), -1, SQLITE_TRANSIENT);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "insertChatRole", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int insertChatMember(sqlite3 *db, uint32_t chatId, uint32_t userId, uint8_t role) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    stmt = nullptr;
    query = "INSERT INTO chatMembers(chatId, userId, roleId) VALUES(?, ?, ?)";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 2, userId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 3, role);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "insertChatMember", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int insertChatMessage(sqlite3 *db, uint32_t chatId, uint32_t userId, const std::string &content, uint32_t timestamp) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    stmt = nullptr;
    query = "INSERT INTO messages(chatId, senderId, content, timestamp) VALUES(?, ?, ?, ?)";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 2, userId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_text(stmt, 3, content.c_str(), -1, SQLITE_TRANSIENT);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 4, timestamp);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "insertChatMessage", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
// Sets
int updateUserName(sqlite3 *db, uint32_t userId,const std::string &name) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    stmt = nullptr;
    query = "UPDATE users SET name = ? WHERE id = ?";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 2, userId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "updateUserName", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int updateChatName(sqlite3 *db, uint32_t chatId,const std::string &name) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    stmt = nullptr;
    query = "UPDATE chats SET name = ? WHERE id = ?";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 2, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "updateChatName", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int updateUserRole(sqlite3 *db, uint32_t chatId, uint32_t userId, uint8_t role) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    stmt = nullptr;
    query = "UPDATE chatMembers SET roleId = ? WHERE chatId = ? AND userId = ?";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, role);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 2, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 3, userId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "updateUserRole", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int updateChatMessage(sqlite3* db, uint32_t messageId, const std::string &content) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    stmt = nullptr;
    query = "UPDATE messages SET content = ? WHERE id = ?;";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_text(stmt, 1, content.c_str(), -1, SQLITE_TRANSIENT);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 2, messageId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "updateChatMessage", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int updateChatTheme(sqlite3* db, uint32_t chatId, uint8_t theme) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    stmt = nullptr;
    query = "UPDATE chats SET theme = ? WHERE id = ?;";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, theme);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 2, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "updateChatTheme", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
// Delete
int deleteUser(sqlite3 *db, uint32_t userId) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    stmt = nullptr;
    query = "DELETE FROM users WHERE id = ?;";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, userId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "deleteUser", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int deleteChat(sqlite3 *db, uint32_t chatId) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    stmt = nullptr;
    query = "DELETE FROM chats WHERE id = ?;";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "deleteChat", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int deleteChatMember(sqlite3 *db, uint32_t chatId, uint32_t userId) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    stmt = nullptr;
    query = "DELETE FROM chatMembers WHERE chatId = ? and userId = ?;";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, chatId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 2, userId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "deleteChatMember", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}
int deleteChatMessage(sqlite3 *db, uint32_t messageId) {
    bool error;
    sqlite3_stmt *stmt;
    std::string query;
    int rc;

    error = false;
    stmt = nullptr;
    query = "DELETE FROM messages WHERE id = ?;";

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_bind_int(stmt, 1, messageId);
    if((error = (rc != SQLITE_OK))) goto error;

    rc = sqlite3_step(stmt);
    if((error = (rc != SQLITE_DONE))) goto error;

error:
    if(error)
        printSqliteError(db, "deleteChatMessage", rc);
    if(stmt)
        sqlite3_finalize(stmt);
    return rc;
}


sqlite3 *getDb(bool init) {
    sqlite3 *db;
    int rc;

    rc = sqlite3_open("chat.db", &db);
    //sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_FKEY, 1, NULL);
    if(rc) {
        std::fprintf(stderr, "(SQLite: %d) Can't open database: %s\n", rc, sqlite3_errmsg(db));
        return nullptr;
    }
    if(init)
        createTables(db);
    return db;
}
