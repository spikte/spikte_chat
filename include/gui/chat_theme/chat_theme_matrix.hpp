#ifndef GUI_CHAT_THEME_MATRIX_HPP
#define GUI_CHAT_THEME_MATRIX_HPP

#include "chat_theme.hpp"
#include <cstdlib>
#include <time.h>
#include <cmath>
#include "../raylib_utils.hpp"

struct GuiChatThemeMatrix: GuiChatThemeData {
    Color textColor;
    int fontSize;
    float timer;
    Font bgFont;
    Vector2 singleCharDim;
    Vector2 charDim;
    std::vector<char> bgChars;
    std::vector<int> headIndex;
    std::vector<int> alphas;

    void init(Rectangle initPanel);
    void update(float fElapsedTime);
    void draw();
    void updatePanel(Rectangle newPanel);

    void updateChars();
    void updateHeadIndexes();
    void updateAlphas();
};

#endif
