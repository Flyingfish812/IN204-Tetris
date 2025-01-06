#include "Game.h"
#include <SDL.h>
#include <SDL_ttf.h>

Game::Game(SDL_Renderer* renderer) : renderer(renderer), paused(false) {}

Game::~Game() {}

bool quit;

void Game::reset() {
    paused = false;
    quit = false;
}

void Game::showPauseMenu() {
    SDL_Event e;
    bool inPauseMenu = true;
    bool continueGame = false;

    // 添加按钮选中状态变量
    bool selectedContinue = true; // 初始选中状态
    bool selectedMainMenu = false;

    TTF_Init();
    TTF_Font* font = TTF_OpenFont("src/arial.ttf", 24);
    SDL_Color textColor = {0, 0, 0, 0};

    SDL_Surface* backToGameSurface = TTF_RenderText_Solid(font, "Back to Game", textColor);
    SDL_Texture* backToGameText = SDL_CreateTextureFromSurface(renderer, backToGameSurface);

    SDL_Surface* quitTextSurface = TTF_RenderText_Solid(font, "Main Menu", textColor);
    SDL_Texture* quitText = SDL_CreateTextureFromSurface(renderer, quitTextSurface);

    SDL_FreeSurface(backToGameSurface);
    SDL_FreeSurface(quitTextSurface);

    while (inPauseMenu) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw buttons
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect continueButton = {300, 200, 200, 50};
        SDL_Rect mainMenuButton = {300, 300, 200, 50};
        SDL_RenderFillRect(renderer, &continueButton);
        SDL_RenderFillRect(renderer, &mainMenuButton);

        // Draw button labels
        SDL_Rect startTextRect = {330, 215, 140, 20};
        SDL_Rect exitTextRect = {340, 315, 120, 20};
        SDL_RenderCopy(renderer, backToGameText, NULL, &startTextRect);
        SDL_RenderCopy(renderer, quitText, NULL, &exitTextRect);

        // Highlight selected button
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        if (selectedContinue) {
            SDL_Rect highlight = {295, 195, 210, 60};
            SDL_RenderDrawRect(renderer, &highlight);
        } else if (selectedMainMenu) {
            SDL_Rect highlight = {295, 295, 210, 60};
            SDL_RenderDrawRect(renderer, &highlight);
        }

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                inPauseMenu = false;
                break;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_UP || e.key.keysym.sym == SDLK_DOWN) {
                    selectedContinue = !selectedContinue;
                    selectedMainMenu = !selectedMainMenu;
                } else if (e.key.keysym.sym == SDLK_RETURN) {
                    if (selectedContinue) {
                        continueGame = true;
                    }
                    inPauseMenu = false;
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    continueGame = true;
                    inPauseMenu = false;
                }
            }
        }
    }

    if (continueGame) {
        paused = false;
    } else {
        quit = true;
    }

    SDL_DestroyTexture(backToGameText);
    SDL_DestroyTexture(quitText);
    TTF_CloseFont(font);
    TTF_Quit();
}

void Game::show() {
    SDL_Event e;
    quit = false;

    while (!quit) {
        if (!paused) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            // Draw game grid
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect grid = {200, 50, 400, 500};
            SDL_RenderDrawRect(renderer, &grid);

            SDL_RenderPresent(renderer);
        }

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                paused = true;
                showPauseMenu();
            }
        }
    }
}