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
    void showPauseMenu();
    Block* currentBlock;
    Grid* grid;
    unsigned int speed;           // 当前下落速度
    int timer;
};

#endif