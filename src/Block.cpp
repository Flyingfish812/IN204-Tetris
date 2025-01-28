#include "Block.h"
#include <cstdlib>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

extern std::ofstream logFile;
extern void log(const std::string& message);

// Form of the block shapes
const std::vector<std::vector<std::vector<int>>> SHAPES = {
    // I
    {{1, 1, 1, 1}},
    // O
    {{1, 1},
     {1, 1}},
    // T
    {{0, 1, 0},
     {1, 1, 1}},
    // L
    {{1, 0, 0},
     {1, 1, 1}},
    // J
    {{0, 0, 1},
     {1, 1, 1}},
    // Z
    {{1, 1, 0},
     {0, 1, 1}},
    // S
    {{0, 1, 1},
     {1, 1, 0}}
};

const int BASIC_COLORS[] = {
    0xFF0000, // Red
    0xFF7F00, // Orange
    0xFFFF00, // Yellow
    0x00FF00, // Green
    0x00FFFF, // Cyan
    0x0000FF, // Blue
    0x8B00FF, // Purple
    0xFF69B4  // Pink
};

const int NUM_BASIC_COLORS = sizeof(BASIC_COLORS) / sizeof(BASIC_COLORS[0]);

Block::Block() {
    type = static_cast<BlockType>(rand() % 7);            // Random block type
    shape = SHAPES[type];                                 // Initialize shape
    color = BASIC_COLORS[rand() % NUM_BASIC_COLORS];      // Random color
    x = 3;                                                // Position (center of the grid)
    y = 0;                                                // Position (top of the grid)
}

void Block::rotate() {
    int n = shape.size();
    int m = shape[0].size();
    std::vector<std::vector<int>> newShape(m, std::vector<int>(n, 0));

    // Rotate the block
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            newShape[j][n - 1 - i] = shape[i][j];
        }
    }

    shape = newShape;
}

void Block::move(int dx, int dy) {
    x += dx;
    y += dy;
}

std::vector<std::vector<int>> Block::getShape() const {
    return shape;
}

BlockType Block::getType() const { return type; }

int Block::getColor() const { return color; }

int Block::getX() const { return x; }
int Block::getY() const { return y; }

// Serialize the block data for online play
std::string Block::serialize() const {
    std::ostringstream oss;
    oss << static_cast<int>(type) << ";" << color << ";" << x << ";" << y << ";";
    for (const auto& row : shape) {
        for (const auto& cell : row) {
            oss << cell << ",";
        }
        oss << ";";
    }
    return oss.str();
}

void Block::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string value;
    
    std::getline(iss, value, ';');
    type = static_cast<BlockType>(std::stoi(value));
    std::getline(iss, value, ';');
    color = std::stoi(value);

    std::getline(iss, value, ';');
    x = std::stoi(value);
    std::getline(iss, value, ';');
    y = std::stoi(value);

    shape.clear();
    while (std::getline(iss, value, ';')) {
        std::vector<int> row;
        std::istringstream rowStream(value);
        std::string cell;
        while (std::getline(rowStream, cell, ',')) {
            row.push_back(std::stoi(cell));
        }
        shape.push_back(row);
    }
}
