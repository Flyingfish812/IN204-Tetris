#ifndef GAME_H
#define GAME_H

#include <SDL.h>

class Game {
public:
    Game(SDL_Renderer* renderer);
    ~Game();
    void show();
    void reset();

private:
    SDL_Renderer* renderer;
    bool paused;
    void showPauseMenu();
};

#endif