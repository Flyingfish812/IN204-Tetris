#include "Grid.h"
#include <SDL.h>

Grid::Grid(int width, int height) : width(width), height(height) {
    grid.resize(height, std::vector<int>(width, 0)); // 初始化网格为 0
}

bool Grid::canPlace(const Block& block) const {
    auto shape = block.getShape();
    int x = block.getX();
    int y = block.getY();

    for (int i = 0; i < shape.size(); ++i) {
        for (int j = 0; j < shape[i].size(); ++j) {
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

    for (int i = 0; i < shape.size(); ++i) {
        for (int j = 0; j < shape[i].size(); ++j) {
            if (shape[i][j]) {
                int newX = x + j;
                int newY = y + i;

                if (newY >= 0 && newY < height && newX >= 0 && newX < width) {
                    grid[newY][newX] = 1; // 将方块固定到网格中
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
            // 清除当前行，并将其上方的行下移
            for (int k = i; k > 0; --k) {
                grid[k] = grid[k - 1]; // 将上一行复制到当前行
            }
            grid[0] = std::vector<int>(width, 0); // 顶部行清空
            ++clearedLines;
        }
    }

    return clearedLines;
}

void Grid::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // 网格线颜色

    // 绘制网格线
    for (int i = 0; i <= height; ++i) {
        SDL_RenderDrawLine(renderer, 200, 50 + i * 20, 400, 50 + i * 20);
    }
    for (int j = 0; j <= width; ++j) {
        SDL_RenderDrawLine(renderer, 200 + j * 20, 50, 200 + j * 20, 450);
    }

    // 绘制网格中的方块
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (grid[i][j]) {
                SDL_Rect rect = {200 + j * 20, 50 + i * 20, 20, 20};
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // 固定方块颜色
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}
