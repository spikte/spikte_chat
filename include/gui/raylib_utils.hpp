#ifndef GUI_RAYLIB_UTILS_HPP
#define GUI_RAYLIB_UTILS_HPP

#include "settings.hpp"
#include <raylib.h>
#include <string>

int GuiLink(const Rectangle& pos, const char* text, Color released = SKYBLUE, Color focused = BLUE, Color pressed = DARKBLUE);
void DrawRectangleRoundedRadius(Rectangle bounds, float radius, int segments, Color color);

void DrawTextRect(Rectangle bounds, const char *text, Color color);
void DrawTextRectEx(Rectangle bounds, Font font, const char *text, float fontSize, float spacing, Color color);

#endif
