#include "Menu.h"
#include <SDL.h>
#include <SDL_ttf.h>

Menu::Menu(SDL_Renderer* renderer) : renderer(renderer), start(false), selectedStart(true), selectedExit(false) {}

Menu::~Menu() {}

void Menu::show() {
    SDL_Event e;
    bool quit = false;

    TTF_Init();
    TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 24);
    SDL_Color textColor = {0, 0, 0, 0};

    SDL_Surface* startTextSurface = TTF_RenderText_Solid(font, "Start Game", textColor);
    SDL_Texture* startText = SDL_CreateTextureFromSurface(renderer, startTextSurface);

    SDL_Surface* exitTextSurface = TTF_RenderText_Solid(font, "Exit Game", textColor);
    SDL_Texture* exitText = SDL_CreateTextureFromSurface(renderer, exitTextSurface);

    SDL_FreeSurface(startTextSurface);
    SDL_FreeSurface(exitTextSurface);

    while (!quit) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw buttons
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect startButton = {300, 200, 200, 50};
        SDL_Rect exitButton = {300, 300, 200, 50};
        SDL_RenderFillRect(renderer, &startButton);
        SDL_RenderFillRect(renderer, &exitButton);

        // Render button labels
        SDL_Rect startTextRect = {330, 215, 140, 20};
        SDL_Rect exitTextRect = {340, 315, 120, 20};
        SDL_RenderCopy(renderer, startText, NULL, &startTextRect);
        SDL_RenderCopy(renderer, exitText, NULL, &exitTextRect);

        // Highlight selected button
        if (selectedStart) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect highlight = {295, 195, 210, 60};
            SDL_RenderDrawRect(renderer, &highlight);
        } else if (selectedExit) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect highlight = {295, 295, 210, 60};
            SDL_RenderDrawRect(renderer, &highlight);
        }

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x = e.button.x;
                int y = e.button.y;

                if (x > 300 && x < 500) {
                    if (y > 200 && y < 250) {
                        start = true;
                        quit = true;
                    } else if (y > 300 && y < 350) {
                        quit = true;
                    }
                }
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_UP || e.key.keysym.sym == SDLK_DOWN) {
                    selectedStart = !selectedStart;
                    selectedExit = !selectedExit;
                } else if (e.key.keysym.sym == SDLK_RETURN) {
                    if (selectedStart) {
                        start = true;
                    }
                    quit = true;
                }
            }
        }
    }

    SDL_DestroyTexture(startText);
    SDL_DestroyTexture(exitText);
    TTF_CloseFont(font);
    TTF_Quit();
}

bool Menu::startSelected() const {
    return start;
}

void Menu::resetState() {
    start = false;
    selectedStart = true;
    selectedExit = false;
}