#include "../../../include/gui/chat_theme/chat_theme_matrix.hpp"

// Dirty code all that,
// a bit tired will come back to it later, maybe
void GuiChatThemeMatrix::init(Rectangle initPanel) {
    std::srand(time(nullptr));
    timer = 0;
    fontSize = 9;
    textColor = GetColor(0x004d11ff);
    panel = initPanel;
    bgFont = LoadFont("static/digital-7 (mono).ttf");
    // Assuming the font is mono
    singleCharDim = MeasureTextEx(bgFont, "0", fontSize, 0);
    charDim.x = (int)(panel.width / singleCharDim.x + 1);
    charDim.y = (int)(panel.height / singleCharDim.y + 1);
    bgChars.resize(charDim.x * charDim.y);
    // Init chars
    for(size_t i = 0; i < bgChars.size(); i++) {
        if(std::rand() % 5 < 2)
            bgChars[i] = '0';
        else
            bgChars[i] = '1';
    }
    // Init heads
    headIndex.resize(charDim.x);
    for(size_t i = 0; i < headIndex.size(); i++)
        headIndex[i] = std::rand() % (int)charDim.y;
    // Setup alphas
    alphas.resize(charDim.x * charDim.y);
    updateAlphas();
}
void GuiChatThemeMatrix::update(float fElapsedTime) {
    timer += fElapsedTime;
    if(timer > 0.1f) {
        timer = 0;
        updateChars();
        updateHeadIndexes();
        updateAlphas();
    }
}
void GuiChatThemeMatrix::draw() {
    Vector2 pos = {panel.x, panel.y};
    BeginScissorMode(panel.x, panel.y, panel.width, panel.height);
    DrawRectangleRec(panel, BLACK);
    char s[2] = {0};
    for(int x = 0; x < charDim.x; x++) {
        for(int y = 0; y < charDim.y; y++) {
            int index = y * charDim.x + x;
            float px = pos.x + x * singleCharDim.x;
            float py = pos.y + y * singleCharDim.y;
            textColor.a = alphas[index];
            s[0] = bgChars[index];
            DrawText(s, px, py, fontSize, textColor);
            //DrawTextCodepoint(bgFont, bgChars[index], {px, py}, fontSize, textColor);
        }
    }
    EndScissorMode();
}
void GuiChatThemeMatrix::updatePanel(Rectangle newPanel) {
    GuiChatThemeData::updatePanel(newPanel);
    Vector2 oldDim;
    size_t oldSize;

    oldDim = charDim;
    oldSize = bgChars.size();

    charDim.x = panel.width / singleCharDim.x;
    charDim.y = panel.height / singleCharDim.y;
    if(oldDim.x == (int)charDim.x && oldDim.y == (int)charDim.y)
        return;

    bgChars.resize(charDim.x * charDim.y);
    headIndex.resize(charDim.x);
    alphas.resize(charDim.x * charDim.y);

    // If we reduced the size nothing to do
    if(oldSize <= bgChars.size())
        return;
    // Else fill missing data
    for(size_t i = oldSize; i < bgChars.size(); i++) {
        if(std::rand() % 5 < 2)
            bgChars[i] = '0';
        else
            bgChars[i] = '1';
    }
    for(int x = oldDim.x; x < charDim.x; x++) {
        headIndex[x] = std::rand() % (int)charDim.y;
    }
    // Recompute the alphas
    updateAlphas();
}

void GuiChatThemeMatrix::updateChars() {
    for(size_t i = 0; i < bgChars.size(); i++) {
        // 5% chance to change
        if(std::rand() % 100 < 5) {
            if(bgChars[i] == '0')
                bgChars[i] = '1';
            else
                bgChars[i] = '0';
        }
    }
}
void GuiChatThemeMatrix::updateHeadIndexes() {
    for(size_t i = 0; i < headIndex.size(); i++) {
        // If in a none state there is a 30 percent chance it starts
        if(headIndex[i] == -1) {
            if(std::rand() % 100 < 30)
                headIndex[i] = 0;
        }
        else if(headIndex[i] >= 1.3 * charDim.y)
            headIndex[i] = -1;
        else
            headIndex[i]++;
    }
}
void GuiChatThemeMatrix::updateAlphas() {
    for(int x = 0; x < charDim.x; x++) {
        for(int y = 0; y < charDim.y; y++) {
            int index = y * charDim.x + x;
            if(y > headIndex[x] || headIndex[x] == 0)
                alphas[index] = 0;
            else {
                int alpha = 255 - 5 * (headIndex[x] - y);
                if(alpha < 0)
                    alphas[index] = 0;
                else
                    alphas[index] = alpha;
            }
        }
    }
}
