#include "../../include/gui/chat.hpp"

static std::string splitMessage(std::string message, float maxSize) {
    Vector2 messageDims;
    std::string result;
    int start;
    Vector2 currentDims;
    std::vector<int> codepoints; // Maybe an array ? The max message size is set so

    messageDims = MeasureTextEx(guiSettings.defaultFont, message.c_str(), (float)guiSettings.fontSize, (float)guiSettings.spacing);
    if(messageDims.x < maxSize)
        return message;
    start = 0;
    for(size_t i = 0; i < message.size(); i++) {
        codepoints.push_back(message[i]);
        currentDims = MeasureTextCodepoints(guiSettings.defaultFont, codepoints.data(), codepoints.size(), (float)guiSettings.fontSize, (float)guiSettings.spacing);
        if(currentDims.x >= maxSize) {
            result += message.substr(start, i - start);
            result += '\n';
            start = i;
            codepoints.clear();
        }
    }
    if(start < message.size())
        result += message.substr(start, message.size() - start);
    if(result.back() == '\n')
        result.pop_back();

    return result;
}


/* Message */
GuiMessageType idToGuiMessageType(uint32_t id) {
    if(id == 0)
        return GuiMessageType::ERROR;
    else if(id == 1)
        return GuiMessageType::SERVER;
    else if(id == localUser.id)
        return GuiMessageType::LOCAL_USER;
    else
        return GuiMessageType::OTHER_USER;
}
void initGuiMessageData(GuiMessageData& guiMessageData, const Message& message) {
    GuiMessageData& data = guiMessageData;

    // Set message type
    guiMessageData.msgType = idToGuiMessageType(message.author.id);
    if(guiMessageData.msgType == GuiMessageType::SERVER)
        guiMessageData.content = splitMessage(message.content, (float)guiSettings.msgMaxSizeServer.x);
    else
        guiMessageData.content = splitMessage(message.content, (float)guiSettings.msgMaxSize.x);
    // Set metadata
    data.id = message.id;
    data.author = "[" + message.author.name + "]";
    // By default we don't display the author name
    if(guiMessageData.msgType == GuiMessageType::OTHER_USER)
        data.displayAuthor = true;
    else
        data.displayAuthor = false;

    data.boundingBox = MeasureTextEx(guiSettings.defaultFont, guiMessageData.content.c_str(), (float)guiSettings.fontSize, (float)guiSettings.spacing);
    // Increase the bounding box by text inside margin
    data.boundingBox.x += 2 * guiSettings.msgMargin.x;
    data.boundingBox.y += 2 * guiSettings.msgMargin.y;
    // If author needs to be displayed increase the bounding box
    if(guiMessageData.msgType == GuiMessageType::OTHER_USER)
        data.boundingBox.y += guiSettings.fontSize + guiSettings.authorGap;
}
void updateGuiMessageDataPos(GuiMessageData& guiMessageData, Rectangle panel, float offsetW) {
    GuiMessageData& data = guiMessageData;
    Vector2 vectBg;
    // Measure text
    vectBg = MeasureTextEx(guiSettings.defaultFont, guiMessageData.content.c_str(), (float)guiSettings.fontSize, (float)guiSettings.spacing);
    vectBg.x += 2 * guiSettings.msgMargin.x;
    vectBg.y += 2 * guiSettings.msgMargin.y;

    // Position the rectangles
    switch(data.msgType) {
        case GuiMessageType::ERROR:
        case GuiMessageType::SERVER:
            data.rectBg = pos(panel, RAYLYT_BOTTOM | RAYLYT_FILL_X, {0, 0, 0, offsetW}, vectBg);
            break;
        case GuiMessageType::LOCAL_USER:
            data.rectBg = pos(panel, RAYLYT_BOTTOM | RAYLYT_RIGHT, {5, 0, 5, offsetW}, vectBg);
            break;
        case GuiMessageType::OTHER_USER:
            data.rectBg = pos(panel, RAYLYT_BOTTOM | RAYLYT_LEFT, {5, 0, 5, offsetW}, vectBg);
            data.displayAuthor = true;
            data.rectAuthor = data.rectBg;
            data.rectAuthor.y -= (guiSettings.fontSize + guiSettings.authorGap);
            break;
    }
    // Compute foreground rectangle
    data.rectFg = posText(data.rectBg, RAYLYT_CENTER_X, guiSettings.msgMargin, data.content.c_str());
}
void GuiMessage(GuiMessageData guiMessageData, const GuiChatThemeData& theme, Vector2 panelScroll) {
    GuiMessageData& data = guiMessageData;
    Rectangle rectAuthor;
    Rectangle rectBg;
    Rectangle rectFg;

    // Apply scroll offset
    rectAuthor = data.rectAuthor;
    rectAuthor.x += panelScroll.x;
    rectAuthor.y += panelScroll.y;
    rectBg = data.rectBg;
    rectBg.x += panelScroll.x;
    rectBg.y += panelScroll.y;
    rectFg = data.rectFg;
    rectFg.x += panelScroll.x;
    rectFg.y += panelScroll.y;
    if(data.displayAuthor)
        DrawTextRect(rectAuthor, data.author.c_str(), theme.authorColor);
    // Draw
    switch(data.msgType) {
        case GuiMessageType::ERROR:
            DrawRectangleRec(rectBg, theme.msgBgColorError);
            DrawTextRect(rectFg, data.content.c_str(), theme.msgFgColorError);
            break;
        case GuiMessageType::SERVER:
            DrawRectangleRec(rectBg, theme.msgBgColorServer);
            DrawTextRect(rectFg, data.content.c_str(), theme.msgFgColorServer);
            break;
        case GuiMessageType::LOCAL_USER:
            DrawRectangleRoundedRadius(rectBg, guiSettings.msgBorderRadius, guiSettings.msgSegments, theme.msgBgColor);
            DrawTextRect(rectFg, data.content.c_str(), theme.msgFgColor);
            break;
        case GuiMessageType::OTHER_USER:
            DrawRectangleRoundedRadius(rectBg, guiSettings.msgBorderRadius, guiSettings.msgSegments, theme.msgBgColor);
            DrawTextRect(rectFg, data.content.c_str(), theme.msgFgColor);
            break;
    }
}

/* Chat */
void initGuiChatData(GuiChatData& guiChatData) {
    GuiChatData& data = guiChatData;
    Rectangle& panel = guiSettings.chatRect;

    // Empty the text input
    std::memset(data.chatMessage, '\0', MAX_SIZE_CHAT_MESSAGE);
    data.chatMessageEdit = false;
    // Position the text input at the bottom of the panel
    data.rectTextBox = pos(panel, RAYLYT_BOTTOM | RAYLYT_FILL_X, guiSettings.inputTextBoxMargin, guiSettings.inputTextBoxDim);
    data.textBoxBoundingY = data.rectTextBox.height + guiSettings.inputTextBoxMargin.y + guiSettings.inputTextBoxMargin.w;
    // We remove the width of a scrollbar that the input is not behing or above the scrollbar but next to it
    //data.rectTextBox.width -= GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH);

    // Setup reference rect
    data.rectViewNoScrollBar = panel;
    data.rectViewNoScrollBar.y += PANEL_HEADER_HEIGHT;
    data.rectViewNoScrollBar.height -= PANEL_HEADER_HEIGHT;

    data.rectViewScrollBar = data.rectViewNoScrollBar;
    //data.rectViewScrollBar.width -= GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH);

    // Setup scroll panel
    // Panel content is the size of panel minus the width of a scrollbar and the height of the text input
    data.panelContent = data.rectViewNoScrollBar;
    data.panelContent.height -= data.textBoxBoundingY + 10;
    data.minHeight = data.panelContent.height;
    data.totalHeight = 0;
    data.panelScroll = {0};
    data.panelView = {0};

    data.rectTheme = data.rectViewNoScrollBar;
    data.theme = createTheme(ChatTheme::DEFAULT, data.rectTheme);
}
void updateGuiChatDataChat(GuiChatData& guiChatData, uint32_t chatId) {
    Chat& chat = *clientConfig.idToChat[chatId];

    guiChatData.id = chat.id;
    guiChatData.name = chat.name;
    if(chat.theme != guiChatData.theme->theme)
        guiChatData.theme = createTheme(chat.theme, guiChatData.rectTheme);
}
static void offsetMessageContained(GuiChatData& guiChatData, GuiMessageData& guiMessageData) {
    int offset;

    offset = guiMessageData.boundingBox.y + guiSettings.msgGap; 
    for(GuiMessageData& prev_message: guiChatData.messages) {
        prev_message.rectBg.y -= offset;
        prev_message.rectFg.y -= offset;
        prev_message.rectAuthor.y -= offset;
    }
    updateGuiMessageDataPos(guiMessageData, guiChatData.panelContent, 0);
}
static void offsetMessageTransition(GuiChatData& guiChatData, GuiMessageData& guiMessageData) {
    int offset;
    int offsetW;

    guiChatData.totalHeight += guiChatData.textBoxBoundingY + 10;
    guiChatData.panelContent.height = guiChatData.totalHeight;
    offset = guiChatData.panelContent.y + guiSettings.msgGap;
    for(GuiMessageData& message: guiChatData.messages) {
        message.rectBg.y = offset;
        message.rectFg.y = offset;
        message.rectAuthor.y = offset;
        //message.boundingBox.y = offset;
        offset += message.boundingBox.y + guiSettings.msgGap;
    }
    offsetW = guiChatData.textBoxBoundingY + 10;
    updateGuiMessageDataPos(guiMessageData, guiChatData.panelContent, offsetW);
}
static void offsetMessageNotContained(GuiChatData& guiChatData, GuiMessageData& guiMessageData) {
    int offsetW;

    offsetW = guiChatData.textBoxBoundingY + 10;
    guiChatData.panelContent.height = guiChatData.totalHeight;
    updateGuiMessageDataPos(guiMessageData, guiChatData.panelContent, offsetW);
}
void updateGuiChatDataMessages(GuiChatData& guiChatData, uint32_t chatId) {
    GuiChatData& data = guiChatData;
    Chat& chat = *clientConfig.idToChat[chatId];
    const Message& message = chat.messages.back();
    int offsetW;
    bool isStartContained;

    isStartContained = data.totalHeight <= data.minHeight;
    offsetW = 0;
    GuiMessageData guiMessageData;
    initGuiMessageData(guiMessageData, message);
    data.totalHeight += guiMessageData.boundingBox.y + guiSettings.msgGap;
    if(isStartContained && data.totalHeight < data.minHeight)
        offsetMessageContained(guiChatData, guiMessageData);
    else if(isStartContained && data.totalHeight >= data.minHeight)
        offsetMessageTransition(guiChatData, guiMessageData);
    else
        offsetMessageNotContained(guiChatData, guiMessageData);

    data.messages.push_back(guiMessageData);
    data.panelScroll = {0, guiSettings.chatRect.height - data.panelContent.height - data.rectTextBox.height - 15};
}
void GuiChat(GuiChatData &data) {
    GuiScrollPanel(guiSettings.chatRect, data.name.c_str(), data.panelContent, &data.panelScroll, &data.panelView);
    data.theme->update(GetFrameTime());
    data.theme->draw();
    if(data.chatMessageEdit && IsKeyPressed(KEY_ENTER)) {
        if(data.chatMessage[0] == '/')
            processCommand(data.id, data.chatMessage);
        else if(data.chatMessage[0] != '\0'){
            Message message{0, getTimestampNow(), localUser, data.chatMessage};
            sendNetMsg(clientConfig.connection, [chatId=data.id, message]{ return buildNetMsgRequestChatMessageAdd(chatId, message); });
        }
        std::memset(data.chatMessage, '\0', MAX_SIZE_CHAT_MESSAGE);
    }
    BeginScissorMode(data.rectTheme.x, data.rectTheme.y, data.rectTheme.width, data.rectTheme.height);
    for(const auto& guiMessageData: data.messages) {
        GuiMessage(guiMessageData, *data.theme, data.panelScroll);
    }
    EndScissorMode();
    DrawRectangleRec(data.rectTextBox, WHITE);
    if(GuiTextBox(data.rectTextBox, data.chatMessage, MAX_SIZE_CHAT_MESSAGE, data.chatMessageEdit)) {
        if(!IsKeyPressed(KEY_ENTER))
            data.chatMessageEdit = !data.chatMessageEdit;
    }
}

/* Chat list */
GuiChatListData guiChatListData;
void initGuiChatList() {
    GuiChatListData& data = guiChatListData;

    data.currentChatId = 0;
    data.chatList.clear();
}
void GuiChatList() {
    GuiChatListData &data = guiChatListData;
    Rectangle panel;
    Vector2 mousePos;

    panel = guiSettings.chatListRect;
    GuiPanel(panel, "Chat List");
    Rectangle rectNewChatBtn = pos(panel, RAYLYT_RIGHT, {0, 2, 2}, {80, 20});
    if(GuiButton(rectNewChatBtn, "New chat"))
        clientCoreState = ClientCoreState::CHAT_CREATION;
    panel.y += PANEL_HEADER_HEIGHT;
    mousePos = GetMousePosition();
    for(auto& guiChat: data.chatList) {
        Rectangle rectBg = pos(panel, RAYLYT_FILL_X, {1, 0, 1, 0}, {0, (float)guiSettings.chatListItemHeight});
        Rectangle rectChatName = posText(rectBg, RAYLYT_LEFT | RAYLYT_TOP, {5, 0}, guiChat.name.c_str());
        // Draw bg
        if(guiChat.id == data.currentChatId)
            DrawRectangleRec(rectBg, SKYBLUE);
        else if(checkVec2IsInRect(mousePos, rectBg)) {
            DrawRectangleRec(rectBg, LIGHTGRAY);
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                data.currentChatId = guiChat.id;
        }
        // Draw chat name
        DrawTextRect(rectChatName, guiChat.name.c_str(), BLACK);
        if(!guiChat.messages.empty()) {
            const GuiMessageData& lastMessage = guiChat.messages.back();
            std::string preview = lastMessage.author + ": " + lastMessage.content;
            Rectangle rectLastMessage = posText(rectBg, RAYLYT_LEFT | RAYLYT_BOTTOM, {10, 0, 0, 10}, preview.c_str());
            DrawTextRect(rectLastMessage, preview.c_str(), GRAY);
        }
        // If it is the current selected chat, draw it
        if(data.currentChatId == guiChat.id)
            GuiChat(guiChat);
        // Update panel y coord
        panel.y += guiSettings.chatListItemHeight;
    }
}

GuiChatCreationData guiChatCreationData;
void initGuiChatCreationData() {
    GuiChatCreationData &data = guiChatCreationData;

    std::memset(data.name, '\0', MAX_SIZE_CHAT_NAME);
    data.nameEdit = false;
    data.msgBgColor = BLUE;
    data.msgFgColor = RAYWHITE;
}
void GuiChatCreationPanel() {
    GuiChatCreationData &data = guiChatCreationData;
    Rectangle panel;

    panel = guiSettings.chatCreationRect;;
    Rectangle rectPanel = panel;
    panel.y += 20;
    Rectangle rectChatName = pos(panel, RAYLYT_CENTER_X, {0, 0}, {0.8f * panel.width, guiSettings.fontSize + 20.0f});
    Rectangle rectChatNameLabel = posText(rectChatName, RAYLYT_TOP | RAYLYT_LEFT, {0}, "Chat name");
    Rectangle rectChatNameInput = pos(rectChatName, RAYLYT_BOTTOM | RAYLYT_CENTER_X, {0}, {rectChatName.width, 20});
    panel.y += 45;
    Rectangle rectCreateChatBtn = pos(panel, RAYLYT_CENTER_X, {0}, {120, 25});
    panel.y += 50;
    Rectangle rectErrorMsg = posText(panel, RAYLYT_CENTER_X, {0}, data.errorMsg.c_str());

    GuiPanel(rectPanel, "Chat creation");
    // Chat name input
    DrawTextRect(rectChatNameLabel, "Chat name", GRAY);
    if(GuiTextBox(rectChatNameInput, data.name, MAX_SIZE_CHAT_NAME, data.nameEdit))
        data.nameEdit = !data.nameEdit;
    // Create message button
    if(GuiButton(rectCreateChatBtn, "Create Chat")) {
        if(data.name[0] == '\0')
            return;
        Chat tempChat;
        tempChat.name = std::string(data.name);
        tempChat.members.clear();
        tempChat.roles.clear();
        tempChat.messages.clear();
        tempChat.theme = ChatTheme::DEFAULT;
        sendNetMsg(clientConfig.connection, [userId=localUser.id, tempChat]{ return buildNetMsgRequestChatAdd(tempChat); });
        clientCoreState = ClientCoreState::WAIT_CHAT_CREATION;
    }
    // Back button
    if(GuiButton(pos(rectPanel, RAYLYT_TOP | RAYLYT_RIGHT, {0, 2, 2, 0}, {20, 20}), "#113#"))
        clientCoreState = ClientCoreState::CHAT;
    // Error display
    if(data.error)
        DrawTextRect(rectErrorMsg, data.errorMsg.c_str(), RED);
}
void GuiChatCreation() {
    GuiDisable();
    GuiChatList();
    GuiEnable();
    GuiChatCreationPanel();
}
void GuiWaitChatCreation() {
    GuiDisable();
    GuiChatList();
    GuiChatCreationPanel();
    GuiEnable();
    DrawTextRect(posText(guiSettings.chatCreationRect, RAYLYT_CENTER_X | RAYLYT_BOTTOM, {0}, "Creating chat..."), "Creating chat..", BLACK);
}
