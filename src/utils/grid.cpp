#include "../../include/utils/grid.hpp"

// Wanted to do a simple chat app idk why I am doing this tho
static int fixOneCoord(int c1, int lim1, GridEdgeHandling how) {
    if(lim1 <= 0)
        return 0;
    int c;

    c = c1;
    switch(how) {
        case GridEdgeHandling::WRAP:
            c = c1 % lim1;
            if(c < 0)
                c += lim1;
            break;
        case GridEdgeHandling::CLAMP:
            if(c1 < 0)
                c = 0;
            else if(c1 >= lim1)
                c = lim1 - 1;
            break;
        case GridEdgeHandling::BOUNCE:
            if(c1 < 0)
                c = -c1;
            else if(c1 >= lim1)
                c = lim1 - 1 - c1 % lim1;
            break;
    }

    return c;
}

Grid::Grid(): dim({0}), edgeHandling(GridEdgeHandling::WRAP) {}
Grid::Grid(Vector2Int dim, GridEdgeHandling edgeHandling): dim(dim), edgeHandling(edgeHandling) {
    cellStates.resize(dim.x * dim.y);
}
Vector2Int Grid::fixCoord(Vector2Int coord) {
    Vector2Int newCoord;

    newCoord = coord;
    switch(edgeHandling) {
        case GridEdgeHandling::WRAP:
        case GridEdgeHandling::CLAMP:
        case GridEdgeHandling::BOUNCE:
            newCoord.x = fixOneCoord(coord.x, dim.x, edgeHandling);
            newCoord.y = fixOneCoord(coord.y, dim.y, edgeHandling);
            break;
        case GridEdgeHandling::LINEAR:
            if(coord.x < 0) {
                if(coord.y >= 1) {
                    newCoord.y--;
                    newCoord.x = dim.x - 1;
                }
                else
                    newCoord.x = 0;
            }
            if(coord.x > dim.x - 1) {
                if(coord.y < dim.x - 1) {
                    coord.y++;
                    newCoord.x = 0;
                }
                else
                    newCoord.x = dim.x - 1;
            }
            if(coord.y < 0) {
                if(coord.x >= 1) {
                    newCoord.x--;
                    newCoord.y = dim.y - 1;
                }
                else
                    newCoord.y = 0;
            }
            if(coord.y > dim.x - 1) {
                if(coord.x < dim.x - 1) {
                    coord.x++;
                    newCoord.y = 0;
                }
                else
                    newCoord.y = dim.y - 1;
            }
            break;
    }

    return newCoord;
}
int Grid::getState(Vector2Int coord) {
    Vector2Int fixedCoord;

    fixedCoord = fixCoord(coord);
    return cellStates[fixedCoord.y * dim.x + fixedCoord.x];
}
void Grid::setState(Vector2Int coord, int state) {
    Vector2Int fixedCoord;

    fixedCoord = fixCoord(coord);
    cellStates[fixedCoord.y * dim.x + fixedCoord.x] = state;
}
void Grid::clearState(int state) {
    for(int x = 0; x < dim.x; x++)
        for(int y = 0; y < dim.y; y++)
            setState({x, y}, state);
}
