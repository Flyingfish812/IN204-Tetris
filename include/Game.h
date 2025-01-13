#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include "Block.h"
#include "Grid.h"

class Game {
public:
    Game(SDL_Renderer* renderer);
    ~Game();

    void reset();
    void show();
    void handleInput();
    void update(Uint32 deltaTime);
    void render();

private:
    SDL_Renderer* renderer;
    bool paused;
    bool quit;
    
    Block* currentBlock;
    Block* nextBlock;
    Grid* grid;

    int score;
    unsigned int speed;           // 当前下落速度
    int timer;

    void showPauseMenu();
    void renderStatusBox(int windowWidth, int windowHeight);
    void renderBlock(Block* block, SDL_Rect displayArea);
};

#endif