#include "../../../include/gui/chat_theme/chat_theme_hellokitty.hpp"

void GuiChatThemeHelloKitty::init(Rectangle initPanel) {
    panel = initPanel;
    helloKittyImage = LoadImage("./static/hello_kitty.png");
    helloKittyTexture = LoadTextureFromImage(helloKittyImage);
}

void GuiChatThemeHelloKitty::update(float fElapsedTime) {
    rectTexture = pos(panel, RAYLYT_CENTER_X | RAYLYT_CENTER_Y, {0}, {(float)helloKittyImage.width, (float)helloKittyImage.height});
}

void GuiChatThemeHelloKitty::draw() {
    DrawRectangleRec(panel, bgColor);
    DrawTexture(helloKittyTexture, rectTexture.x, rectTexture.y, WHITE);
}
