#include "../../include/gui/raylayout.hpp"

Rectangle lastLayout;

Rectangle pos(Rectangle container, uint16_t flags, Vector4 margin, Vector2 dimension) {
    Rectangle result = {0};

    result.width = dimension.x;
    result.height = dimension.y;
    // X axis
    if(flags & RAYLYT_FILL_X) {
        result.x = container.x + margin.x;
        result.width = container.width - margin.x - margin.z;
    } else if(flags & RAYLYT_RIGHT)
        result.x = container.x + container.width - dimension.x - margin.z;
    else if(flags & RAYLYT_CENTER_X)
        result.x = container.x + container.width / 2 - dimension.x / 2;
    else
        result.x = container.x + margin.x;
    // Y axis
    if(flags & RAYLYT_FILL_Y) {
        result.y = container.y + margin.y;
        result.height = container.height - margin.y - margin.w;
    } else if(flags & RAYLYT_BOTTOM)
        result.y = container.y + container.height - dimension.y - margin.w;
    else if(flags & RAYLYT_CENTER_Y)
        result.y = container.y + container.height / 2 - dimension.y / 2;
    else
        result.y = container.y + margin.y;

    lastLayout = result;
    return result;
}
Rectangle posTextEx(Font font, Rectangle container, uint16_t flags, Vector4 margin, const char *text, float fontSize, float spacing) {
    Vector2 textDimension;

    textDimension = MeasureTextEx(font, text, fontSize, spacing);
    return pos(container, flags, margin, textDimension);
}
Rectangle posText(Rectangle container, uint16_t flags, Vector4 margin, const char *text) {
    return posTextEx(guiSettings.defaultFont, container, flags, margin, text, guiSettings.fontSize, guiSettings.spacing);
}
