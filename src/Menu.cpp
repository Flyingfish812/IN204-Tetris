#include "Menu.h"
#include "Button.h"
#include <iostream>
#include <fstream>

extern std::ofstream logFile;
extern void log(const std::string& message);

Menu::Menu(SDL_Renderer* renderer) 
    : renderer(renderer), selectedButtonIndex(0), currentState(MenuState::None), title("Title") {
    buttons.reserve(3);
}

Menu::~Menu() {
    TTF_Quit();
}

void Menu::addButton(const std::string& text, const SDL_Rect& rect, const std::function<void()>& callback) {
    try {
        buttons.emplace_back(renderer, text, rect, callback);
        logFile << "Button added: " << text << std::endl;
    } catch (const std::exception& e) {
        logFile << "Error creating button: " << e.what() << std::endl;
    } catch (...) {
        logFile << "Unknown error creating button" << std::endl;
    }
}

void Menu::setState(MenuState state) {
    currentState = state;    
}

void Menu::setTitle(const std::string& titleName) {
    title = titleName;
}

void Menu::show() {
    SDL_Event e;
    bool quit = false;

    while (!quit) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 渲染标题
        if (!title.empty()) {
            TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 24);

            if (font) {
                SDL_Color textColor = {255, 255, 255, 255};
                SDL_Surface* holdSurface = TTF_RenderText_Solid(font, title.c_str(), textColor);
                SDL_Texture* holdTexture = SDL_CreateTextureFromSurface(renderer, holdSurface);

                SDL_Rect holdTextRect = {400 - holdSurface->w / 2, 100, holdSurface->w, holdSurface->h};
                SDL_RenderCopy(renderer, holdTexture, NULL, &holdTextRect);

                SDL_FreeSurface(holdSurface);
                SDL_DestroyTexture(holdTexture);
            }
        }

        // 渲染所有按钮
        for (size_t i = 0; i < buttons.size(); ++i) {
            buttons[i].render(i == selectedButtonIndex);
        }

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                // currentState = MenuState::Exit;
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_UP) {
                    selectedButtonIndex = (selectedButtonIndex - 1 + buttons.size()) % buttons.size();
                } else if (e.key.keysym.sym == SDLK_DOWN) {
                    selectedButtonIndex = (selectedButtonIndex + 1) % buttons.size();
                } else if (e.key.keysym.sym == SDLK_RETURN) {
                    buttons[selectedButtonIndex].handleClick();
                    quit = true; // 退出菜单循环
                }
            }
        }
    }
}

MenuState Menu::getSelectedState() const {
    return currentState;
}

void Menu::resetState() {
    currentState = MenuState::None;
}