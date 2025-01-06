#ifndef MENU_H
#define MENU_H

#include <SDL.h>

class Menu {
public:
    Menu(SDL_Renderer* renderer);
    ~Menu();
    void show();
    bool startSelected() const;
    void resetState();

private:
    SDL_Renderer* renderer;
    bool start;
    bool selectedStart;
    bool selectedExit;
};

#endif