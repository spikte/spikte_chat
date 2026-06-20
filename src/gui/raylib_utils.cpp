#include "../../include/gui/raylib_utils.hpp"

bool checkVec2IsInRect(const Vector2 &point, const Rectangle &rect) {
    return point.x >= rect.x && point.x <= rect.x + rect.width &&
           point.y >= rect.y && point.y <= rect.y + rect.height;
}
int GuiLabeledTextBox(const Rectangle &bounds, float margin, char *text, int textSize, bool editMode, const char *label, Color labelColor) {
    int textWidth;

    textWidth = MeasureText(label, guiSettings.fontSize);
    int y = bounds.y + bounds.height / 2 - (float)guiSettings.fontSize / 2;
    DrawText(label, bounds.x + margin, y, guiSettings.fontSize, labelColor);
    Rectangle textRect = bounds;
    textRect.x += margin + textWidth;
    textRect.width -= 2 * margin + textWidth;
    return GuiTextBox(textRect, text, textSize, editMode);
}
uint32_t colorToU32(Color color) {
    return ((uint32_t)color.r << 24) |
           ((uint32_t)color.g << 16) |
           ((uint32_t)color.b << 8 ) |
           ((uint32_t)color.a);
}
Color u32ToColor(uint32_t colorU32) {
    Color color;

    color.r = colorU32 >> 24 & 0xFF;
    color.g = colorU32 >> 16 & 0xFF;
    color.b = colorU32 >> 8 & 0xFF;
    color.a = colorU32 & 0xFF;

    return color;
}
void DrawRectangleRoundedRadius(Rectangle rec, float radius, int segments, Color color) {
    float minSide;
    float roundness;

    minSide = rec.width < rec.height ? rec.width : rec.height;
    roundness = 2 * radius / minSide;
    DrawRectangleRounded(rec, roundness, segments, color);
}
void DrawTextRect(Rectangle bounds, const char* text, Color color) {
    DrawTextEx(guiSettings.defaultFont, text, {bounds.x, bounds.y}, guiSettings.fontSize, guiSettings.spacing, color);
}
void DrawTextRectEx(Rectangle bounds, Font font, const char* text, float fontSize, float spacing, Color color) {
    DrawTextEx(font, text, {bounds.x, bounds.y}, fontSize, spacing, color);
}
