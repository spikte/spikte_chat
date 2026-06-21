#include "../../include/gui/raylib_utils.hpp"

// Copied from raygui button, just changed the display
// However I removed the guiControlExclusiveMode check, not the best
// but not the end of the world
int GuiLink(const Rectangle& pos, const char* text, Color released, Color focused, Color pressed) {
    int result = 0;
    GuiState state = (GuiState)GuiGetState();

    // Update control
    //--------------------------------------------------------------------
    if ((state != STATE_DISABLED) && !GuiIsLocked()) {
        Vector2 mousePoint = GUI_POINTER_POSITION;

        // Check button state
        if (CheckCollisionPointRec(mousePoint, pos)) {
            if (GUI_BUTTON_DOWN) state = STATE_PRESSED;
            else state = STATE_FOCUSED;

            if (GUI_BUTTON_RELEASED) result = 1;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    Color linkColor = released;
    if(state == STATE_PRESSED)
        linkColor = pressed;
    else if(state == STATE_FOCUSED)
        linkColor = focused;
    DrawTextRect(pos, text, linkColor);
    //--------------------------------------------------------------------

    return result;      // Button pressed: result = 1
}
void DrawRectangleRoundedRadius(Rectangle bounds, float radius, int segments, Color color) {
    float minSide;
    float roundness;

    minSide = bounds.width < bounds.height ? bounds.width : bounds.height;
    roundness = 2 * radius / minSide;
    DrawRectangleRounded(bounds, roundness, segments, color);
}
void DrawTextRect(Rectangle bounds, const char* text, Color color) {
    DrawTextEx(GuiGetFont(), text, {bounds.x, bounds.y}, GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), color);
}
void DrawTextRectEx(Rectangle bounds, Font font, const char* text, float fontSize, float spacing, Color color) {
    DrawTextEx(font, text, {bounds.x, bounds.y}, fontSize, spacing, color);
}
