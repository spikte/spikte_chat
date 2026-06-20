#include "../../include/gui/settings.hpp"

GuiSettings guiSettings;

void initSettingsDefaults() {
    guiSettings.margin = {20, 20};
    guiSettings.windowGap = 10;
    guiSettings.authorGap = 5;
    guiSettings.chatListItemHeight = -1;
    guiSettings.fontSize = 20;
    guiSettings.msgGap = 8;
    guiSettings.msgInterline = 4;
    guiSettings.spacing = 0;
    guiSettings.msgMargin = {10, 5, 10, 5};
    guiSettings.msgMaxSize = {-1};
    guiSettings.msgMaxSizeServer = {-1};
    guiSettings.msgBorderRadius = 20;
    guiSettings.msgSegments = 100;
}
// Based on: https://www.raylib.com/examples/text/loader.html?name=text_font_sdf
void initDefaultFont() {
    guiSettings.defaultFont = LoadFontEx("static/Jersey10-Regular.ttf", guiSettings.fontSize, 0, 400);
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
    guiSettings.chatListRect.x = guiSettings.margin.x;
    guiSettings.chatListRect.y = guiSettings.margin.y;
    guiSettings.chatListRect.width = (screenWidth - 2 * guiSettings.margin.x) / 4.f - guiSettings.windowGap / 2.f;
    guiSettings.chatListRect.height = screenHeight - 2 * guiSettings.margin.y;
    guiSettings.chatListItemHeight = std::max(guiSettings.fontSize, 50);
}
void initSettingsChat(int screenWidth, int screenHeight) {
    guiSettings.chatRect.x = guiSettings.chatListRect.x + guiSettings.chatListRect.width + guiSettings.windowGap / 2.f;
    guiSettings.chatRect.y = guiSettings.margin.y;
    guiSettings.chatRect.width = 3 * (screenWidth - 2 * guiSettings.margin.x) / 4.f - guiSettings.windowGap;
    guiSettings.chatRect.height = screenHeight - 2 * guiSettings.margin.y;
}
void initSettingsChatCreation(int screenWidth, int screenHeight) {
    initRectCentered(guiSettings.chatCreationRect, screenWidth, screenHeight, 250, 100);
}
void initSettingsChatMember(int screenWidth, int screenHeight) {
    initRectCentered(guiSettings.chatMemberRect, screenWidth, screenHeight, 300, 400);
}
void initSettingsMsg() {
    guiSettings.spacing = guiSettings.fontSize / 30;
}
void initSettings(int screenWidth, int screenHeight) {
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);
    GuiSetStyle(LISTVIEW, SCROLLBAR_WIDTH, 5);

    initSettingsDefaults();
    initDefaultFont();
    initSettingsServerConnection(screenWidth, screenHeight);
    initSettingsAuth(screenWidth, screenHeight);
    initSettingsChatList(screenWidth, screenHeight);
    initSettingsChat(screenWidth, screenHeight);
    initSettingsChatCreation(screenWidth, screenHeight);
    initSettingsMsg();

    if(guiSettings.chatRect.width < 100) {
        guiSettings.msgMaxSize = {guiSettings.chatRect.width * 0.3f, 0};
        guiSettings.msgMaxSizeServer = {guiSettings.chatRect.width * 0.8f, 0};
    }
    else {
        guiSettings.msgMaxSize = {guiSettings.chatRect.width, 0};
        guiSettings.msgMaxSizeServer = {guiSettings.chatRect.width, 0};
    }
}
