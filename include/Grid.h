#ifndef GRID_H
#define GRID_H

#include "Block.h"
#include <SDL.h>
#include <vector>

class Grid {
public:
    Grid(int width, int height);
    bool canPlace(const Block& block) const;
    void placeBlock(const Block& block);
    int clearLines();
    void render(SDL_Renderer* renderer);

private:
    int width, height;
    std::vector<std::vector<int>> grid;
};

#endif
