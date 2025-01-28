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
    
    std::string serialize() const;
    void deserialize(const std::string& data);

private:
    BlockType type;
    std::vector<std::vector<int>> shape;
    int x, y;
    int color;
};

#endif
