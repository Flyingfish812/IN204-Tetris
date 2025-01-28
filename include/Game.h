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
    void renderText(const std::string& text, int x, int y, int w, int h, SDL_Color color);

protected:
    SDL_Renderer* renderer;
    
    bool gameStarted = false;
    bool quit;
    bool gameOver;
    
    Block* currentBlock;
    Block* nextBlock;
    Grid* grid;

    int score;
    unsigned int speed;
    int timer;
    
    void renderStatusBox(int windowWidth, int windowHeight);
    void renderBlock(Block* block, SDL_Rect displayArea);
    void renderGameOver();

private:
    bool paused;
    void showPauseMenu();
};

#endif