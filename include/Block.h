#ifndef BLOCK_H
#define BLOCK_H

#include <vector>
#include <string>

enum BlockType { I, O, T, L, J, Z, S };

class Block {
public:
    Block();
    void rotate();
    void move(int dx, int dy);
    std::vector<std::vector<int>> getShape() const;
    BlockType getType() const;
    int getColor() const;
    int getX() const;
    int getY() const;
    
    std::string serialize() const; // 序列化方块状态
    void deserialize(const std::string& data); // 反序列化方块状态

private:
    BlockType type;
    std::vector<std::vector<int>> shape; // 形状的二维矩阵
    int x, y; // 方块位置
    int color; // 方块颜色
};

#endif
