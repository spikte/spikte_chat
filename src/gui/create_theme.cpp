#include "../../include/gui/create_theme.hpp"

std::unique_ptr<GuiChatThemeData> createTheme(ChatTheme theme, Rectangle panel) {
    std::unique_ptr<GuiChatThemeData> data;

    switch(theme) {
        case ChatTheme::DEFAULT:
            data = std::make_unique<GuiChatThemeSimple>();
            data->msgBgColor = BLUE;
            data->msgFgColor = RAYWHITE;
            data->bgColor = WHITE;
            data->authorColor = GRAY;
            data->msgBgColorServer = LIGHTGRAY;
            //data->msgBgColorServer.a = 128;
            data->msgFgColorServer = DARKGRAY;
            data->msgBgColorError = BLANK;
            data->msgFgColorError = RED;
            data->hasDynamicBackground = false;
            break;
        case ChatTheme::GRUVBOX:
            data = std::make_unique<GuiChatThemeSimple>();
            data->msgBgColor = GetColor(0xd65d0eff);
            data->msgFgColor = GetColor(0xebdbb2ff);
            data->bgColor = GetColor(0x282828ff);
            data->authorColor = GetColor(0xb8bb26ff);
            data->msgBgColorServer = GetColor(0x32302fff);
            data->msgFgColorServer = GetColor(0xbdae93ff);
            data->msgBgColorError = BLANK;
            data->msgFgColorError = GetColor(0xcc241dff);
            data->hasDynamicBackground = false;
            break;
        case ChatTheme::GOL:
            data = std::make_unique<GuiChatThemeGOL>();
            data->msgBgColor = BLACK;
            data->msgFgColor = LIGHTGRAY;
            data->bgColor = WHITE;
            data->authorColor = RED;
            data->msgBgColorServer = LIGHTGRAY;
            data->msgBgColorServer.a = 128;
            data->msgFgColorServer = DARKGRAY;
            data->msgBgColorError = BLANK;
            data->msgFgColorError = RED;
            data->hasDynamicBackground = true;
            break;
        case ChatTheme::MATRIX:
            data = std::make_unique<GuiChatThemeMatrix>();
            data->msgBgColor = GetColor(0x00ff2bff);
            data->msgFgColor = BLACK;
            data->bgColor = BLACK;
            data->authorColor = GetColor(0x00ff2bff);
            data->msgBgColorServer = BLACK;
            data->msgFgColorServer = GetColor(0x00ff2bff);
            data->msgBgColorError = BLANK;
            data->msgFgColorError = RED;
            data->hasDynamicBackground = true;
            break;
        case ChatTheme::HELLO_KITTY:
            data = std::make_unique<GuiChatThemeHelloKitty>();
            data->msgBgColor = GetColor(0x095d9aff);
            data->msgFgColor = GetColor(0xfbd8b7ff);
            data->bgColor = GetColor(0xfbd8b7ff);
            data->authorColor = GetColor(0x095d9aff);
            data->msgBgColorServer = GetColor(0x5a5858ff);
            data->msgFgColorServer = GetColor(0xe16389ff);
            data->msgBgColorError = GetColor(0xe782a0ff);
            data->msgFgColorError = GetColor(0x431d29ff);
            data->hasDynamicBackground = false;
            break;
    }
    data->theme = theme;
    data->init(panel);

    return data;
}
