#include "Game.h"
#include "Button.h"
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>
#include <fstream>

extern std::ofstream logFile;
extern void log(const std::string& message);

Game::Game(SDL_Renderer* renderer) : renderer(renderer), paused(false), quit(false), gameOver(false) {
    srand(static_cast<unsigned>(time(nullptr)));
    grid = new Grid(10, 20);
    currentBlock = new Block();
    nextBlock = new Block();
    score = 0;
    speed = 1000;
    timer = 0;
}

Game::~Game() {
    delete currentBlock;
    delete nextBlock;
    delete grid;
}

void Game::reset() {
    log("Game: reset");
    paused = false;
    quit = false;
    delete grid;
    grid = new Grid(10, 20);
    delete currentBlock;
    currentBlock = new Block();
    speed = 1000;
    timer = 0;
    log("Reset complete");
}

void Game::handleInput() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_LEFT:
                    currentBlock->move(-1, 0);
                    if (!grid->canPlace(*currentBlock)) {
                        currentBlock->move(1, 0); // Revert
                    }
                    break;
                case SDLK_RIGHT:
                    currentBlock->move(1, 0);
                    if (!grid->canPlace(*currentBlock)) {
                        currentBlock->move(-1, 0); // Revert
                    }
                    break;
                case SDLK_DOWN:
                    currentBlock->move(0, 1);
                    if (!grid->canPlace(*currentBlock)) {
                        currentBlock->move(0, -1); // Revert
                    }
                    break;
                case SDLK_UP:
                    currentBlock->rotate();
                    if (!grid->canPlace(*currentBlock)) {
                        // Revert rotation
                        currentBlock->rotate();
                        currentBlock->rotate();
                        currentBlock->rotate();
                    }
                    break;
                case SDLK_ESCAPE:
                    paused = true;
                    showPauseMenu();
                    break;
            }
        }
    }
}

void Game::renderText(const std::string& text, int x, int y, int w, int h, SDL_Color color) {
    TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 24);
    if (!font) {
        log("Failed to load font: " + std::string(TTF_GetError()));
        return;
    }
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!textSurface) {
        log("Failed to create text surface: " + std::string(TTF_GetError()));
        TTF_CloseFont(font);
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        log("Failed to create text texture: " + std::string(SDL_GetError()));
        SDL_FreeSurface(textSurface);
        TTF_CloseFont(font);
        return;
    }

    SDL_Rect textRect = {x, y, w, h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
    TTF_CloseFont(font);
}

void Game::showPauseMenu() {
    SDL_Event e;
    bool inPauseMenu = true;
    bool continueGame = false;

    // Button selection state
    bool selectedContinue = true;
    bool selectedMainMenu = false;

    SDL_Color textColor = {0, 0, 0, 0};

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
        renderText("Back to Game", 330, 215, 140, 20, textColor);
        renderText("Main Menu", 340, 315, 120, 20, textColor);

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
}

void Game::update(Uint32 deltaTime) {
    static Uint32 accumulatedTime = 0;
    accumulatedTime += deltaTime;

    if (accumulatedTime >= speed) {
        currentBlock->move(0, 1);
        if (!grid->canPlace(*currentBlock)) {
            currentBlock->move(0, -1);
            grid->placeBlock(*currentBlock);
            score += grid->clearLines() * 100;
            delete currentBlock;
            currentBlock = nextBlock;
            nextBlock = new Block();

            if (!grid->canPlace(*currentBlock)) {
                gameOver = true;
            }
        }
        accumulatedTime = 0;

        // Speed up logic
        if (speed > 200) {
            speed -= 10;
        }
    }
}

void Game::render() {
    // Get window size
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render grid
    grid->render(renderer, windowWidth, windowHeight);

    // Render status box
    renderStatusBox(windowWidth, windowHeight);

    // Render current block
    auto shape = currentBlock->getShape();
    int x = currentBlock->getX();
    int y = currentBlock->getY();
    int color = currentBlock->getColor();

    for (size_t i = 0; i < shape.size(); ++i) {
        for (size_t j = 0; j < shape[i].size(); ++j) {
            if (shape[i][j]) {
                SDL_Rect rect = {
                    grid->getGridXOffset() + static_cast<int>(x + j) * grid->getCellSize(),
                    grid->getGridYOffset() + static_cast<int>(y + i) * grid->getCellSize(),
                    grid->getCellSize(), grid->getCellSize()
                };
                SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    // Present rendered frame
    SDL_RenderPresent(renderer);
}

void Game::renderStatusBox(int windowWidth, int windowHeight) {
    int statusBoxWidth = windowWidth / 4;
    int statusBoxX = windowWidth - statusBoxWidth;
    int statusBoxY = 0;
    int statusBoxHeight = windowHeight;

    // Draw status box background
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect statusBox = {statusBoxX, statusBoxY, statusBoxWidth, statusBoxHeight};
    SDL_RenderFillRect(renderer, &statusBox);

    SDL_Color textColor = {255, 255, 255, 255};

    renderText("Hold:", statusBoxX + 20, 20, 100, 30, textColor);

    // Draw current block
    int blockBoxSize;
    if (currentBlock->getType() == I) {
        blockBoxSize = 120;
    } else if (currentBlock->getType() == O) {
        blockBoxSize = 60;
    } else {
        blockBoxSize = 90;
    }
    SDL_Rect currentBlockBox = {statusBoxX + 20, 50, blockBoxSize, blockBoxSize};
    renderBlock(currentBlock, currentBlockBox);

    renderText("Next:", statusBoxX + 20, 170, 100, 30, textColor);

    // Draw next block
    if (nextBlock->getType() == I) {
        blockBoxSize = 120;
    } else if (nextBlock->getType() == O) {
        blockBoxSize = 60;
    } else {
        blockBoxSize = 90;
    }
    SDL_Rect nextBlockBox = {statusBoxX + 20, 200, blockBoxSize, blockBoxSize};
    renderBlock(nextBlock, nextBlockBox);

    // Display score and speed
    renderText("Score: " + std::to_string(score), statusBoxX + 20, 350, 150, 50, textColor);
    renderText("Speed: " + std::to_string(speed), statusBoxX + 20, 420, 150, 50, textColor);
}

void Game::renderBlock(Block* block, SDL_Rect displayArea) {
    if (!block) return;

    auto shape = block->getShape();
    int color = block->getColor();

    // Calculate scaling factor
    int rows = shape.size();
    int cols = shape[0].size();
    int cellSize = std::min(displayArea.w / cols, displayArea.h / rows);

    // Starting coordinates to center the block in displayArea
    int offsetX = displayArea.x + (displayArea.w - cellSize * cols) / 2;
    int offsetY = displayArea.y + (displayArea.h - cellSize * rows) / 2;

    // Draw block
    SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 255);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (shape[i][j]) {
                SDL_Rect rect = {offsetX + j * cellSize, offsetY + i * cellSize, cellSize, cellSize};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

void Game::renderGameOver() {
    bool showText = true;
    Uint32 lastToggleTime = SDL_GetTicks();
    const Uint32 toggleInterval = 500;

    while (!quit) {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastToggleTime > toggleInterval) {
            showText = !showText;
            lastToggleTime = currentTime;
        }

        render();

        if (showText) {
            SDL_Color textColor = {255, 255, 255, 255};
            renderText("Game Over", 300, 200, 200, 50, textColor);
        }

        SDL_RenderPresent(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                quit = true;
                return;
            }
        }
        SDL_Delay(16);
    }
}

void Game::show() {
    if(gameStarted){
        return;
    }
    gameStarted = true;
    reset();
    Uint32 lastTime = SDL_GetTicks();
    while (!quit) {
        if (!paused) {
            Uint32 currentTime = SDL_GetTicks();
            Uint32 deltaTime = currentTime - lastTime;
            lastTime = currentTime;
            handleInput();
            if (gameOver) {
                renderGameOver();
                gameStarted = false;
                return;
            }
            update(deltaTime);
            render();
        } else {
            showPauseMenu();
        }
        SDL_Delay(16);
    }
}