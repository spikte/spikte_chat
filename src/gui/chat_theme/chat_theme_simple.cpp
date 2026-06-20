#include "../../../include/gui/chat_theme/chat_theme_simple.hpp"


void GuiChatThemeSimple::init(Rectangle initPanel) {
    GuiChatThemeData::init(initPanel);
}
void GuiChatThemeSimple::update(float fElapsedTime) { }
void GuiChatThemeSimple::draw() {
    DrawRectangleRec(panel, bgColor);
}
