#ifndef GUI_CHAT_THEME_HPP
#define GUI_CHAT_THEME_HPP

#include "../../core/chat.hpp"
#include <raylib.h>
#include <memory>

struct GuiChatThemeData {
    ChatTheme theme;
    Rectangle panel;
    Color msgBgColor;
    Color msgFgColor;
    Color authorColor;
    Color msgBgColorServer;
    Color msgFgColorServer;
    Color msgBgColorError;
    Color msgFgColorError;
    Color bgColor;
    bool hasDynamicBackground;

    virtual void init(Rectangle initPanel);
    virtual void update(float fElapsedTime) = 0;
    virtual void draw();
    virtual void updatePanel(Rectangle newPanel);
    virtual ~GuiChatThemeData() = default;
};


#endif
