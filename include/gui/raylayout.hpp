#ifndef GUI_RAYLAYOUT_HPP
#define GUI_RAYLAYOUT_HPP

#include "settings.hpp"
#include <raylib.h>
#include <stdint.h>

enum RayLayoutFlags {
    // ALIGN X
    RAYLYT_LEFT = 1 << 0,
    RAYLYT_RIGHT = 1 << 1,
    RAYLYT_CENTER_X = 1 << 2,

    // ALIGN Y
    RAYLYT_TOP = 1 << 3,
    RAYLYT_BOTTOM = 1 << 4,
    RAYLYT_CENTER_Y = 1 << 5,

    // Space filling
    RAYLYT_FILL_X = 1 << 6,
    RAYLYT_FILL_Y = 1 << 7,
    RAYLYT_FILL = RAYLYT_FILL_X | RAYLYT_FILL_Y
};
extern Rectangle lastLayout;

Rectangle pos(Rectangle container, uint16_t flags, Vector4 margin, Vector2 dimension);
Rectangle posTextEx(Font font, Rectangle container, uint16_t flags, Vector4 margin, const char *text, float fontSize, float spacing);
Rectangle posText(Rectangle container, uint16_t flags, Vector4 margin, const char *text);

#endif
