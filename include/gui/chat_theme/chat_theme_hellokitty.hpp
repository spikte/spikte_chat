#ifndef GUI_CHAT_THEME_HELLO_KITTY_HPP
#define GUI_CHAT_THEME_HELLO_KITTY_HPP

#include "chat_theme.hpp"
#include "../raylayout.hpp"

struct GuiChatThemeHelloKitty: GuiChatThemeData {
    Rectangle rectTexture;
    Image helloKittyImage;
    Texture2D helloKittyTexture;

    void init(Rectangle intPanel);
    void update(float fElapsedTime);
    void draw();
};


#endif

