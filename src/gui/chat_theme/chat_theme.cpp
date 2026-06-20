#include "../../../include/gui/chat_theme/chat_theme.hpp"

void GuiChatThemeData::init(Rectangle initPanel) {
    this->panel = initPanel;
}
void GuiChatThemeData::draw() {
    DrawRectangleRec(panel, bgColor);
}
void GuiChatThemeData::updatePanel(Rectangle newPanel) {
    this->panel = newPanel;
}
