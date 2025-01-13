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
    void render(SDL_Renderer* renderer, int windowWidth, int windowHeight);

    int getGridPixelWidth() const { return gridPixelWidth; }
    int getGridPixelHeight() const { return gridPixelHeight; }
    int getCellSize() const { return cellSize; }
    int getGridXOffset() const { return gridXOffset; }
    int getGridYOffset() const { return gridYOffset; }

private:
    int width, height;
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<int>> gridColors; // 颜色存储

    int gridPixelWidth;
    int gridPixelHeight;
    int cellSize;
    int gridXOffset;
    int gridYOffset;
};

#endif
