#ifndef UTILS_GRID_HPP
#define UTILS_GRID_HPP

#include <vector>
#include <cstdio>

struct Vector2Int {
    int x;
    int y;
};

enum class GridEdgeHandling {
    WRAP,
    LINEAR,
    CLAMP,
    BOUNCE
};

struct Grid {
    Vector2Int dim;
    std::vector<int> cellStates;
    GridEdgeHandling edgeHandling;

    Grid();
    Grid(Vector2Int dim, GridEdgeHandling edgeHandling);
    ~Grid() = default;
    Vector2Int fixCoord(Vector2Int p);
    int getState(Vector2Int coord);
    void setState(Vector2Int coord, int state);
    void clearState(int states);
};

#endif
