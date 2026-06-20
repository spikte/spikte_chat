#ifndef GUI_CHAT_THEME_SIMPLE_HPP
#define GUI_CHAT_THEME_SIMPLE_HPP

#include "chat_theme.hpp"

struct GuiChatThemeSimple: GuiChatThemeData {
    void init(Rectangle initPanel);
    void update(float fElapsedTime);
    void draw();
};

#endif
