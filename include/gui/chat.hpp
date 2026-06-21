#ifndef GUI_CHAT_HPP
#define GUI_CHAT_HPP

#include "../core/api.hpp"
#include "../core/chat.hpp"
#include "../core/client.hpp"
#include "../core/network_utils.hpp"
#include "../core/process_command.hpp"
#include "../core/state.hpp"
#include "../utils/time.hpp"
#include "create_theme.hpp"
#include "raylayout.hpp"
#include "raylib_utils.hpp"
#include <chrono>
#include <cstring>
#include <map>

/* Message */
enum class GuiMessageType {
    ERROR = 0,
    SERVER = 1,
    LOCAL_USER,
    OTHER_USER
};
GuiMessageType idToGuiMessageType(int id);
struct GuiMessageData {
    // Metadata
    uint32_t id;
    GuiMessageType msgType;
    std::string content;
    std::string author;

    // Gui data
    // // Message
    Rectangle rectBg;
    Rectangle rectFg;
    // // Author
    Rectangle rectAuthor;
    bool displayAuthor;
    // // Debug
    Vector2 boundingBox;
};
void initGuiMessageData(GuiMessageData& guiMessageData, const Message& message);
void updateGuiMessageDataPos(GuiMessageData& guiMessageData, Rectangle panel, float offsetW);
void GuiMessage(GuiMessageData guiMessageData, const GuiChatThemeData& theme, Vector2 panelScroll);

/* Chat */
struct GuiChatData {
    // Metadata
    uint32_t id;
    std::string name;

    // Gui data
    // // Ref
    Rectangle rectViewScrollBar;
    Rectangle rectViewNoScrollBar;
    // // View
    // // Messages
    std::vector<GuiMessageData> messages;
    // // Message input
    Rectangle rectTextBox;
    float textBoxBoundingY;
    char chatMessage[MAX_SIZE_CHAT_MESSAGE];
    bool chatMessageEdit;
    // // Theme
    Rectangle rectTheme;
    std::unique_ptr<GuiChatThemeData> theme;
    // // Scroll panel
    Rectangle panelContent;
    Vector2 panelScroll;
    Rectangle panelView; // Trash not used, maybe there's something i don't get about PanelScroll
    int minHeight;
    int totalHeight;
};
void initGuiChatData(GuiChatData &guiChatData);
void updateGuiChatDataChat(GuiChatData &guiChatData, uint32_t chatId);
void updateGuiChatDataMessages(GuiChatData &guiChatData, uint32_t chatId);
void GuiChat(GuiChatData &data);

/* Chat list */
struct GuiChatListData {
    // Gui data
    std::list<GuiChatData> chatList;
    std::unordered_map<uint32_t, std::list<GuiChatData>::iterator> idToGuiChatData;
    uint32_t currentChatId;
};
extern GuiChatListData guiChatListData;
void initGuiChatList();
void GuiChatList();

/* Chat create */
struct GuiChatCreateData {
    // Gui data
    // // Chat name input
    char name[MAX_SIZE_CHAT_NAME];
    bool nameEdit;
    // // Chat color
    Color msgBgColor;
    Color msgFgColor;
    // // Error
    bool error;
    std::string errorMsg;
};
extern GuiChatCreateData guiChatCreateData;
void initGuiChatCreateData();
void GuiChatCreatePanel();
void GuiChatCreate();
void GuiWaitChatCreate();

/* Chat deletion */
struct GuiChatDeleteData {
    uint32_t id;
    bool isLocalUserOwner;
};
extern GuiChatDeleteData guiChatDeleteData;
void initGuiChatDeleteData();
void GuiChatDeleteMember();
void GuiChatDeleteOwner();
void GuiChatDelete();

#endif

