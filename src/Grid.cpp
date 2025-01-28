#include "Grid.h"
#include <SDL.h>
#include <sstream>

Grid::Grid(int width, int height) : width(width), height(height) {
    grid.resize(height, std::vector<int>(width, 0)); // Initialize grid by 0
    gridColors.resize(height, std::vector<int>(width, 0)); // Initial color by 0
}

bool Grid::canPlace(const Block& block) const {
    auto shape = block.getShape();
    int x = block.getX();
    int y = block.getY();

    for (size_t i = 0; i < shape.size(); ++i) {
        for (size_t j = 0; j < shape[i].size(); ++j) {
            if (shape[i][j]) { // Only check the filled cells
                int newX = x + j;
                int newY = y + i;

                // Out of bounds
                if (newX < 0 || newX >= width || newY >= height) {
                    return false;
                }

                // Conflict with existing block
                if (newY >= 0 && grid[newY][newX]) { // Neglect the top row
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
                    grid[newY][newX] = 1; // Fix the block in the grid
                    gridColors[newY][newX] = color; // Store the color
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
                grid[k] = grid[k - 1];
                gridColors[k] = gridColors[k - 1];
            }
            grid[0] = std::vector<int>(width, 0); // Clear the top line
            gridColors[0] = std::vector<int>(width, 0); // Clear the top line color
            ++clearedLines;
        }
    }

    return clearedLines;
}

void Grid::render(SDL_Renderer* renderer, int windowWidth, int windowHeight) {
    // Calculate the size of the grid
    gridPixelHeight = windowHeight * 0.9;
    cellSize = gridPixelHeight / height;
    gridPixelWidth = cellSize * width;

    gridXOffset = windowHeight * 0.05;
    gridYOffset = (windowHeight - gridPixelHeight) / 2;

    // Render the grid
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    for (int i = 0; i <= height; ++i) {
        SDL_RenderDrawLine(renderer, gridXOffset, gridYOffset + i * cellSize,
                           gridXOffset + gridPixelWidth, gridYOffset + i * cellSize);
    }
    for (int j = 0; j <= width; ++j) {
        SDL_RenderDrawLine(renderer, gridXOffset + j * cellSize, gridYOffset,
                           gridXOffset + j * cellSize, gridYOffset + gridPixelHeight);
    }

    // Render the blocks
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

// Serialize the grid state for online play
std::string Grid::serialize() const {
    std::ostringstream oss;
    for (const auto& row : grid) {
        for (const auto& cell : row) {
            oss << cell << ",";
        }
        oss << ";";
    }
    return oss.str();
}

void Grid::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string rowStr;
    int rowIdx = 0;

    while (std::getline(iss, rowStr, ';') && rowIdx < height) {
        std::istringstream rowStream(rowStr);
        std::string cellStr;
        int colIdx = 0;

        while (std::getline(rowStream, cellStr, ',') && colIdx < width) {
            grid[rowIdx][colIdx] = std::stoi(cellStr);
            ++colIdx;
        }
        ++rowIdx;
    }
}
