#include "../../include/gui/settings.hpp"

GuiSettings guiSettings;

void initSettingsDefaults() {
    guiSettings.margin = {20, 20};
    guiSettings.windowGap = 10;
    guiSettings.authorGap = 5;
    guiSettings.chatListItemHeight = -1;
    guiSettings.msgGap = 8;
    guiSettings.msgMargin = {10, 5, 10, 5};
    guiSettings.msgMaxSize = {-1};
    guiSettings.msgMaxSizeServer = {-1};
    guiSettings.msgBorderRadius = 20;
    guiSettings.msgSegments = 100;
}
// Based on: https://www.raylib.com/examples/text/loader.html?name=text_font_sdf
void initDefaultFont() {
    guiSettings.defaultFont = LoadFontEx("static/Jersey10-Regular.ttf", 20, 0, 400);
}
void initRectCentered(Rectangle& rect, int screenWidth, int screenHeight, int width, int height) {
    rect.x = screenWidth < width ? 0 : screenWidth / 2 - width / 2;
    rect.y = screenHeight < height ? 0 : screenHeight / 2 - height / 2;
    rect.width = width;
    rect.height = height;
}
void initSettingsServerConnection(int screenWidth, int screenHeight) {
    initRectCentered(guiSettings.rectServerConnection, screenWidth, screenHeight, 400, 250);
}
void initSettingsAuth(int screenWidth, int screenHeight) {
    initRectCentered(guiSettings.rectAuth, screenWidth, screenHeight, 300, 300);
}
void initSettingsChatList(int screenWidth, int screenHeight) {
    guiSettings.rectChatList.x = guiSettings.margin.x;
    guiSettings.rectChatList.y = guiSettings.margin.y;
    guiSettings.rectChatList.width = (screenWidth - 2 * guiSettings.margin.x) / 4.f - guiSettings.windowGap / 2.f;
    guiSettings.rectChatList.height = screenHeight - 2 * guiSettings.margin.y;
    guiSettings.chatListItemHeight = std::max(GuiGetStyle(DEFAULT, TEXT_SIZE), 50);
    guiSettings.maxWidthMessagePreview = guiSettings.rectChatList.width * 0.6f;
}
void initSettingsChat(int screenWidth, int screenHeight) {
    guiSettings.rectChat.x = guiSettings.rectChatList.x + guiSettings.rectChatList.width + guiSettings.windowGap / 2.f;
    guiSettings.rectChat.y = guiSettings.margin.y;
    guiSettings.rectChat.width = 3 * (screenWidth - 2 * guiSettings.margin.x) / 4.f - guiSettings.windowGap;
    guiSettings.rectChat.height = screenHeight - 2 * guiSettings.margin.y;
    guiSettings.inputTextBoxDim = {0, 20};
    guiSettings.inputTextBoxMargin = {5, 0, 5, 10};
    guiSettings.rectChatMargin = guiSettings.inputTextBoxMargin;
    if(guiSettings.rectChat.width > 100) {
        guiSettings.msgMaxSize = {guiSettings.rectChat.width * 0.5f, 0};
        guiSettings.msgMaxSizeServer = {guiSettings.rectChat.width * 0.8f, 0};
    }
    else {
        guiSettings.msgMaxSize = {guiSettings.rectChat.width, 0};
        guiSettings.msgMaxSizeServer = {guiSettings.rectChat.width, 0};
    }
}
void initSettingsChatCreate(int screenWidth, int screenHeight) {
    initRectCentered(guiSettings.rectChatCreate, screenWidth, screenHeight, 250, 100);
}
void initSettingsChatDelete(int screenWidth, int screenHeight) {
    initRectCentered(guiSettings.rectChatDelete, screenWidth, screenHeight, 400, 100);
}
void initSettingsChatMember(int screenWidth, int screenHeight) {
    initRectCentered(guiSettings.rectChatMember, screenWidth, screenHeight, 300, 400);
}
void initSettingsMsg() { }
void initSettings(int screenWidth, int screenHeight) {
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);
    GuiSetStyle(LISTVIEW, SCROLLBAR_WIDTH, 5);

    initSettingsDefaults();
    initDefaultFont();
    initSettingsServerConnection(screenWidth, screenHeight);
    initSettingsAuth(screenWidth, screenHeight);
    initSettingsChatList(screenWidth, screenHeight);
    initSettingsChat(screenWidth, screenHeight);
    initSettingsChatCreate(screenWidth, screenHeight);
    initSettingsChatDelete(screenWidth, screenHeight);
    initSettingsMsg();
}
