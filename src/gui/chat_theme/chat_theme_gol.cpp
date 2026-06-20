#include "../../../include/gui/chat_theme/chat_theme_gol.hpp"


void GuiChatThemeGOL::init(Rectangle initPanel) {
    GuiChatThemeData::init(initPanel);
    cellSize = {5, 5};
    grid = Grid(
            {(int)(panel.width / cellSize.x), (int)(panel.height / cellSize.y)},
            GridEdgeHandling::WRAP
    );
    panel.width = grid.dim.x * cellSize.x;
    panel.height = grid.dim.y * cellSize.y;
    nextCellStates.resize(grid.dim.x * grid.dim.y);
    initGrid(40);
    timer = 0;
}
void GuiChatThemeGOL::update(float fElapsedTime) {
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        if(mousePos.x >= panel.x && mousePos.x <= panel.x + panel.width && mousePos.y >= panel.y && mousePos.y <= panel.y + panel.height) {
            Vector2 cellPos = {(mousePos.x - panel.x) / cellSize.x, (mousePos.y - panel.y) / cellSize.y};
            drawSpaceShip(cellPos);
        }
    }
    timer += fElapsedTime;
    if(timer >= 0.1f) {
        timer = 0;
        updateGrid();
    }
}
void GuiChatThemeGOL::draw() {
    GuiChatThemeData::draw();
    for(int y = 0; y < grid.dim.y; y++) {
        for(int x = 0; x < grid.dim.x; x++) {
            if(grid.getState({x, y}))
                DrawRectangle(panel.x + x * cellSize.x, panel.y + y * cellSize.y, cellSize.x, cellSize.y, LIGHTGRAY);
            else
                DrawRectangle(panel.x + x * cellSize.x, panel.y + y * cellSize.y, cellSize.x, cellSize.y, WHITE);
        }
    }
}
// TODO: could be optimized
void GuiChatThemeGOL::updatePanel(Rectangle newPanel) {
    GuiChatThemeData::updatePanel(newPanel);
    std::vector<int> oldCellStates;
    Vector2Int prevDim;

    oldCellStates = grid.cellStates;
    prevDim = grid.dim;
    grid = Grid(
        {(int)(panel.width / cellSize.x), (int)(panel.height / cellSize.y)},
        GridEdgeHandling::WRAP
    );
    panel.width = grid.dim.x * cellSize.x;
    panel.height = grid.dim.y * cellSize.y;
    nextCellStates.resize(grid.dim.x * grid.dim.y);
    grid.clearState(0);
    for(int x = 0; x < prevDim.x; x++) {
        for(int y = 0; y < prevDim.y; y++) {
            grid.setState({x, y}, oldCellStates[y * prevDim.x + x]);
        }
    }
}

void GuiChatThemeGOL::initGrid(int chance) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, 100);

    for(int x = 0; x < grid.dim.x; x++) {
        for(int y = 0; y < grid.dim.y; y++) {
            if(dist(rng) < chance)
                grid.setState({x, y}, 1);
            else
                grid.setState({x, y}, 0);
        }
    }
}
int GuiChatThemeGOL::getNeighbors(int x, int y) {
    int nNeighbors;

    nNeighbors = 0;
    for(int xIt = x - 1; xIt < x + 2; xIt++) {
        for(int yIt = y - 1; yIt < y + 2; yIt++) {
            if(xIt == x && yIt == y)
                continue;
            nNeighbors += grid.getState({xIt, yIt});
        }
    }

    return nNeighbors;
}
void GuiChatThemeGOL::updateGrid() {
    int nNeighbors;

    for(int x = 0; x < grid.dim.x; x++) {
        for(int y = 0; y < grid.dim.y; y++) {
            nNeighbors = getNeighbors(x, y);
            if(nNeighbors == 3)
                nextCellStates[y * grid.dim.x + x] = 1;
            else if(nNeighbors == 2)
                nextCellStates[y * grid.dim.x + x] = grid.cellStates[y * grid.dim.x + x];
            else 
                nextCellStates[y * grid.dim.x + x] = 0;
        }
    }
    grid.cellStates = nextCellStates;
}
void GuiChatThemeGOL::drawSpaceShip(Vector2 pos) {
    Vector2Int posInt = {(int)pos.x, (int)pos.y};
    grid.setState(posInt, 1);
    grid.setState({posInt.x + 1, posInt.y + 1}, 1);
    grid.setState({posInt.x + 2, posInt.y - 1}, 1);
    grid.setState({posInt.x + 2, posInt.y}, 1);
    grid.setState({posInt.x + 2, posInt.y + 1}, 1);
}
