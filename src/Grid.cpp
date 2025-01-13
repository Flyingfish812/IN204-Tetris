#include "Grid.h"
#include <SDL.h>

Grid::Grid(int width, int height) : width(width), height(height) {
    grid.resize(height, std::vector<int>(width, 0)); // 初始化网格为 0
    gridColors.resize(height, std::vector<int>(width, 0)); // 初始化网格颜色为 0
}

bool Grid::canPlace(const Block& block) const {
    auto shape = block.getShape();
    int x = block.getX();
    int y = block.getY();

    for (size_t i = 0; i < shape.size(); ++i) {
        for (size_t j = 0; j < shape[i].size(); ++j) {
            if (shape[i][j]) { // 仅检查方块占据的格子
                int newX = x + j;
                int newY = y + i;

                // 超出网格边界
                if (newX < 0 || newX >= width || newY >= height) {
                    return false;
                }

                // 与网格已有内容冲突
                if (newY >= 0 && grid[newY][newX]) { // 忽略网格顶部
                    return false;
                }
            }
        }
    }
    return true;
}

void Grid::placeBlock(const Block& block) {
    auto shape = block.getShape();
    int x = block.getX();
    int y = block.getY();
    int color = block.getColor();

    for (size_t i = 0; i < shape.size(); ++i) {
        for (size_t j = 0; j < shape[i].size(); ++j) {
            if (shape[i][j]) {
                int newX = x + j;
                int newY = y + i;

                if (newY >= 0 && newY < height && newX >= 0 && newX < width) {
                    grid[newY][newX] = 1; // 将方块固定到网格中
                    gridColors[newY][newX] = color; // 存储颜色
                }
            }
        }
    }
}

int Grid::clearLines() {
    int clearedLines = 0;

    for (int i = 0; i < height; ++i) {
        bool isFullLine = true;
        for (int j = 0; j < width; ++j) {
            if (grid[i][j] == 0) {
                isFullLine = false;
                break;
            }
        }

        if (isFullLine) {
            for (int k = i; k > 0; --k) {
                grid[k] = grid[k - 1]; // 下移
                gridColors[k] = gridColors[k - 1]; // 下移颜色
            }
            grid[0] = std::vector<int>(width, 0); // 顶部行清空
            gridColors[0] = std::vector<int>(width, 0); // 清空顶部行颜色
            ++clearedLines;
        }
    }

    return clearedLines;
}

void Grid::render(SDL_Renderer* renderer, int windowWidth, int windowHeight) {
    // 动态计算网格大小
    gridPixelHeight = windowHeight * 0.9; // 80% 的窗口高度
    cellSize = gridPixelHeight / height; // 每个方块的大小
    gridPixelWidth = cellSize * width;  // 网格宽度由列数确定

    // 左对齐网格
    // gridXOffset = (windowWidth - gridPixelWidth) / 2;
    gridXOffset = windowHeight * 0.05;
    gridYOffset = (windowHeight - gridPixelHeight) / 2;

    // 绘制网格线
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    for (int i = 0; i <= height; ++i) {
        SDL_RenderDrawLine(renderer, gridXOffset, gridYOffset + i * cellSize,
                           gridXOffset + gridPixelWidth, gridYOffset + i * cellSize);
    }
    for (int j = 0; j <= width; ++j) {
        SDL_RenderDrawLine(renderer, gridXOffset + j * cellSize, gridYOffset,
                           gridXOffset + j * cellSize, gridYOffset + gridPixelHeight);
    }

    // 绘制网格中的固定方块
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (grid[i][j]) {
                SDL_Rect rect = {gridXOffset + j * cellSize, gridYOffset + i * cellSize, cellSize, cellSize};
                int color = gridColors[i][j];
                SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}
