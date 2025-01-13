#include "Game.h"
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>

Game::Game(SDL_Renderer* renderer) : renderer(renderer), paused(false), quit(false) {
    srand(static_cast<unsigned>(time(nullptr)));
    grid = new Grid(10, 20); // 初始化网格大小 (10列, 20行)
    currentBlock = new Block();
    speed = 1000; // 初始下落速度 (单位: 毫秒)
    timer = 0;
}

Game::~Game() {
    delete currentBlock;
    delete grid;
}

void Game::reset() {
    paused = false;
    quit = false;
    delete grid;
    grid = new Grid(10, 20);
    delete currentBlock;
    currentBlock = new Block();
    speed = 1000;
    timer = 0;
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
                        currentBlock->move(1, 0); // 恢复
                    }
                    break;
                case SDLK_RIGHT:
                    currentBlock->move(1, 0);
                    if (!grid->canPlace(*currentBlock)) {
                        currentBlock->move(-1, 0); // 恢复
                    }
                    break;
                case SDLK_DOWN:
                    currentBlock->move(0, 1);
                    if (!grid->canPlace(*currentBlock)) {
                        currentBlock->move(0, -1); // 恢复
                    }
                    break;
                case SDLK_UP:
                    currentBlock->rotate();
                    if (!grid->canPlace(*currentBlock)) {
                        // 若旋转后无效，则恢复
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

void Game::showPauseMenu() {
    SDL_Event e;
    bool inPauseMenu = true;
    bool continueGame = false;

    // 添加按钮选中状态变量
    bool selectedContinue = true; // 初始选中状态
    bool selectedMainMenu = false;

    TTF_Init();
    TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 24);
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

void Game::update(Uint32 deltaTime) {
    static Uint32 accumulatedTime = 0;
    accumulatedTime += deltaTime;

    if (accumulatedTime >= speed) {
        // 方块自动下落
        currentBlock->move(0, 1);
        if (!grid->canPlace(*currentBlock)) {
            currentBlock->move(0, -1); // 恢复
            grid->placeBlock(*currentBlock); // 固定方块
            grid->clearLines();             // 检查并消除行
            delete currentBlock;
            currentBlock = new Block();     // 生成新方块

            if (!grid->canPlace(*currentBlock)) {
                quit = true; // 游戏结束
            }
        }
        accumulatedTime = 0;

        // 随游戏进程加速
        if (speed > 200) {
            speed -= 10;
        }
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    grid->render(renderer);
    // 渲染当前方块
    auto shape = currentBlock->getShape();
    int x = currentBlock->getX();
    int y = currentBlock->getY();
    for (size_t i = 0; i < shape.size(); ++i) {
        for (size_t j = 0; j < shape[i].size(); ++j) {
            if (shape[i][j]) {
                SDL_Rect rect = {200 + static_cast<int>(x + j) * 20, 50 + static_cast<int>(y + i) * 20, 20, 20};
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 假设颜色为红色
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void Game::show() {
    reset();
    Uint32 lastTime = SDL_GetTicks();
    while (!quit) {
        if (!paused) {
            Uint32 currentTime = SDL_GetTicks();
            Uint32 deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            handleInput();
            update(deltaTime);
            render();
        } else {
            // 处理暂停菜单逻辑
            showPauseMenu();
        }
        SDL_Delay(16); // 控制帧率 (约60 FPS)
    }
}