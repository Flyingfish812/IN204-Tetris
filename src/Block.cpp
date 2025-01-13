#include "Block.h"
#include <cstdlib>
#include <vector>

// 方块的初始形状定义
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
    0xFF0000, // 红色
    0xFF7F00, // 橙色
    0xFFFF00, // 黄色
    0x00FF00, // 绿色
    0x00FFFF, // 青色
    0x0000FF, // 蓝色
    0x8B00FF, // 紫色
    0xFF69B4  // 粉色
};

const int NUM_BASIC_COLORS = sizeof(BASIC_COLORS) / sizeof(BASIC_COLORS[0]);

Block::Block() {
    type = static_cast<BlockType>(rand() % 7);            // 随机生成类型
    shape = SHAPES[type];                                 // 初始化形状
    color = BASIC_COLORS[rand() % NUM_BASIC_COLORS];      // 随机颜色
    x = 3;                                                // 起始位置（网格中间偏左）
    y = 0;                                                // 起始位置（网格顶部）
}

void Block::rotate() {
    int n = shape.size();
    int m = shape[0].size();
    std::vector<std::vector<int>> newShape(m, std::vector<int>(n, 0));

    // 逆时针旋转 90 度
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            newShape[j][n - 1 - i] = shape[i][j];
        }
    }

    shape = newShape; // 更新形状
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
