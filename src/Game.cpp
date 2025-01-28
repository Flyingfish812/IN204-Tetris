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
    grid = new Grid(10, 20); // 初始化网格大小 (10列, 20行)
    currentBlock = new Block();
    nextBlock = new Block(); // 初始化下一个方块
    score = 0;               // 初始得分
    speed = 100;            // 初始下落速度
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
    speed = 100;
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
        currentBlock->move(0, 1);
        if (!grid->canPlace(*currentBlock)) {
            currentBlock->move(0, -1); // 恢复
            grid->placeBlock(*currentBlock); // 固定当前方块
            score += grid->clearLines() * 100;    // 更新得分
            delete currentBlock;
            currentBlock = nextBlock;       // 当前方块替换为下一个方块
            nextBlock = new Block();        // 生成新的下一个方块

            if (!grid->canPlace(*currentBlock)) {
                // quit = true;
                gameOver = true;
            }
        }
        accumulatedTime = 0;

        // 加速逻辑
        if (speed > 200) {
            speed -= 10;
        }
    }
}

void Game::render() {
    // 获取窗口大小
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // 清空屏幕
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 渲染网格
    grid->render(renderer, windowWidth, windowHeight);

    // 渲染状态框
    renderStatusBox(windowWidth, windowHeight);

    // 渲染当前方块
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

    // 显示绘制结果
    SDL_RenderPresent(renderer);
}

void Game::renderStatusBox(int windowWidth, int windowHeight) {
    int statusBoxWidth = windowWidth / 4; // 状态框占窗口宽度的1/4
    int statusBoxX = windowWidth - statusBoxWidth; // 状态框靠右对齐
    int statusBoxY = 0;
    int statusBoxHeight = windowHeight;

    // 绘制状态框背景
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect statusBox = {statusBoxX, statusBoxY, statusBoxWidth, statusBoxHeight};
    SDL_RenderFillRect(renderer, &statusBox);

    TTF_Init();
    TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 24);

    if (font) {
        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* holdSurface = TTF_RenderText_Solid(font, "Hold:", textColor);
        SDL_Texture* holdTexture = SDL_CreateTextureFromSurface(renderer, holdSurface);

        SDL_Rect holdTextRect = {statusBoxX + 20, 20, holdSurface->w, holdSurface->h};
        SDL_RenderCopy(renderer, holdTexture, NULL, &holdTextRect);

        SDL_FreeSurface(holdSurface);
        SDL_DestroyTexture(holdTexture);
    }

    // 绘制当前方块
    int blockBoxSize; // 当前方块显示区域的大小
    if (currentBlock->getType() == I) {
        blockBoxSize = 120;
    } else if (currentBlock->getType() == O) {
        blockBoxSize = 60;
    } else {
        blockBoxSize = 90;
    }
    SDL_Rect currentBlockBox = {statusBoxX + 20, 50, blockBoxSize, blockBoxSize};
    renderBlock(currentBlock, currentBlockBox);

    if (font) {
        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* nextSurface = TTF_RenderText_Solid(font, "Next:", textColor);
        SDL_Texture* nextTexture = SDL_CreateTextureFromSurface(renderer, nextSurface);

        SDL_Rect nextTextRect = {statusBoxX + 20, 170, nextSurface->w, nextSurface->h};
        SDL_RenderCopy(renderer, nextTexture, NULL, &nextTextRect);

        SDL_FreeSurface(nextSurface);
        SDL_DestroyTexture(nextTexture);
    }

    // 绘制下一个方块
    if (nextBlock->getType() == I) {
        blockBoxSize = 120;
    } else if (nextBlock->getType() == O) {
        blockBoxSize = 60;
    } else {
        blockBoxSize = 90;
    }
    SDL_Rect nextBlockBox = {statusBoxX + 20, 200, blockBoxSize, blockBoxSize};
    renderBlock(nextBlock, nextBlockBox);

    // 显示得分
    
    if (font) {
        // logFile << "Font loaded successfully" << std::endl;
        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, ("Score: " + std::to_string(score)).c_str(), textColor);
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);

        SDL_Rect scoreRect = {statusBoxX + 20, 350, 150, 50};
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);

        SDL_FreeSurface(scoreSurface);
        SDL_DestroyTexture(scoreTexture);

        SDL_Surface* speedSurface = TTF_RenderText_Solid(font, ("Speed: " + std::to_string(speed)).c_str(), textColor);
        SDL_Texture* speedTexture = SDL_CreateTextureFromSurface(renderer, speedSurface);

        SDL_Rect speedRect = {statusBoxX + 20, 420, 150, 50};
        SDL_RenderCopy(renderer, speedTexture, NULL, &speedRect);

        SDL_FreeSurface(speedSurface);
        SDL_DestroyTexture(speedTexture);
        TTF_CloseFont(font);
    }
}

void Game::renderBlock(Block* block, SDL_Rect displayArea) {
    if (!block) return;

    auto shape = block->getShape(); // 获取方块形状
    int color = block->getColor(); // 获取方块颜色

    // 计算缩放比例
    int rows = shape.size();
    int cols = shape[0].size();
    int cellSize = std::min(displayArea.w / cols, displayArea.h / rows);

    // 起始坐标，使方块居中显示在 displayArea 中
    int offsetX = displayArea.x + (displayArea.w - cellSize * cols) / 2;
    int offsetY = displayArea.y + (displayArea.h - cellSize * rows) / 2;

    // 绘制方块
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
    const Uint32 toggleInterval = 500; // 闪烁间隔（毫秒）

    // SDL_Color backgroundColor = {0, 0, 0, 255}; // 黑色背景

    while (!quit) {
        // 检测闪烁时间
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastToggleTime > toggleInterval) {
            showText = !showText;
            lastToggleTime = currentTime;
        }

        render();

        // 渲染文本
        if (showText) {
            TTF_Init();
            TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 24);
            // 假设 createTextTexture 是一个生成文本纹理的函数
            SDL_Color textColor = {255, 255, 255, 255};
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Game Over", textColor);
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

            SDL_Rect scoreRect = {300, 200, 200, 50};
            SDL_RenderCopy(renderer, textTexture, NULL, &scoreRect);
            SDL_DestroyTexture(textTexture);
        }

        SDL_RenderPresent(renderer);

        // 检查是否有按键按下
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                quit = true;
                return; // 任意按键退出
            }
        }
        SDL_Delay(16); // 控制帧率
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
            // 处理暂停菜单逻辑
            showPauseMenu();
        }
        SDL_Delay(16); // 控制帧率 (约60 FPS)
    }
}