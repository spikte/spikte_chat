#ifndef GUI_RAYLIB_UTILS_HPP
#define GUI_RAYLIB_UTILS_HPP

#include "settings.hpp"
#include <raylib.h>
#include <string>

bool checkVec2IsInRect(const Vector2 &point, const Rectangle &rect);
int GuiLabeledTextBox(const Rectangle &hints, float margin, char *text, int textSize, bool editMode, const char *label, Color labelColor);
uint32_t colorToU32(Color color);
Color u32ToColor(uint32_t colorU32);
void DrawRectangleRoundedRadius(Rectangle rec, float radius, int segments, Color color);

void DrawTextRect(Rectangle bounds, const char *text, Color color);
void DrawTextRectEx(Rectangle bounds, Font font, const char *text, float fontSize, float spacing, Color color);

#endif
