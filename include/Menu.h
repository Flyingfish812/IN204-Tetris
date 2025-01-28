#ifndef MENU_H
#define MENU_H

#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include "Button.h"

enum class MenuState {
    None,
    StartGame,
    Multiplayer,
    Exit
};

class Menu {
public:
    Menu(SDL_Renderer* renderer);
    ~Menu();
    void addButton(const std::string& text, const SDL_Rect& rect, const std::function<void()>& callback);
    void setTitle(const std::string& title);
    void show();
    MenuState getSelectedState() const;
    void setState(MenuState state);
    void resetState();
    size_t buttonsSize() const{return buttons.size();};

private:
    SDL_Renderer* renderer;
    std::vector<Button> buttons;
    size_t selectedButtonIndex;
    MenuState currentState;
    std::string title;
};

#endif // MENU_H
