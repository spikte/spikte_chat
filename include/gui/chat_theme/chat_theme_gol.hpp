#ifndef GUI_CHAT_THEME_GOL_HPP
#define GUI_CHAT_THEME_GOL_HPP

#include "chat_theme.hpp"
#include "../../utils/grid.hpp"
#include <random>
#include "../raylib_utils.hpp"

struct GuiChatThemeGOL: GuiChatThemeData {
    Grid grid;
    Vector2 cellSize;
    std::vector<int> nextCellStates;
    float timer;
    
    void init(Rectangle initPanel);
    void update(float fElapsedTime);
    void updatePanel(Rectangle newPanel);
    void draw();

    int getNeighbors(int x, int y);
    void initGrid(int chance);
    void updateGrid();
    void drawSpaceShip(Vector2 pos);
};

#endif
