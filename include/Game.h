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
    virtual void show();
    virtual void handleInput();
    virtual void update(Uint32 deltaTime);
    virtual void render();

protected:
    SDL_Renderer* renderer;
    
    bool gameStarted = false;
    bool quit;
    bool gameOver;
    
    Block* currentBlock;
    Block* nextBlock;
    Grid* grid;

    int score;
    unsigned int speed;           // 当前下落速度
    int timer;
    
    void renderStatusBox(int windowWidth, int windowHeight);
    void renderBlock(Block* block, SDL_Rect displayArea);
    void renderGameOver();
    // void waitForAnyKey();

private:
    bool paused;
    void showPauseMenu();
};

#endif