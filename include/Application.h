#ifndef APPLICATION_H
#define APPLICATION_H

#include "Menu.h"
#include "Game.h"
#include <fstream>

extern std::ofstream logFile;

class Application {
public:
    Application();
    ~Application();
    void run();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    Menu* menu;
    Game* game;
};

#endif