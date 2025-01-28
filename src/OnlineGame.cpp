#include "OnlineGame.h"
#include <sstream>

OnlineGame::OnlineGame(SDL_Renderer* renderer, Network* network)
    : Game(renderer), network(network) {
    network->setNotifyGameStateCallback([this](const std::string& playerId, const std::string& state) {
        handleRemoteState(playerId, state); // 处理远程玩家的状态更新
    });
    log("OnlineGame initialized.");
}

OnlineGame::~OnlineGame() {}

void OnlineGame::handleInput() {
    Game::handleInput(); 
    syncState();
}

void OnlineGame::update(Uint32 deltaTime) {
    Game::update(deltaTime);
}

void OnlineGame::render() {
    Game::render();
    renderOtherPlayers(400, 50, 150, 400); // 右侧状态栏位置
}

void OnlineGame::syncState() {
    std::ostringstream oss;
    oss << grid->serialize() << "|" 
        << currentBlock->serialize() << "|" 
        << score;
    network->broadcastGameState(oss.str());
}

void OnlineGame::handleRemoteState(const std::string& playerId, const std::string& state) {
    std::istringstream iss(state);
    std::string gridState, blockState, scoreState;

    std::getline(iss, gridState, '|');
    std::getline(iss, blockState, '|');
    std::getline(iss, scoreState, '|');

    Grid tempGrid(10, 20);
    tempGrid.deserialize(gridState);

    Block tempBlock;
    tempBlock.deserialize(blockState);
    
    int playerScore = std::stoi(scoreState);
    
    // 将状态存储
    std::ostringstream renderedState;
    
    renderedState << tempGrid.serialize() 
                  << "|" << tempBlock.serialize() 
                  << "|" << playerScore;
    playerStates[playerId] = renderedState.str();
}

void OnlineGame::renderOtherPlayers(int x, int y, int width, int height) {
    int playerCount = static_cast<int>(playerStates.size());
    if (playerCount == 0) return;
    log("Rendering other players...");

    int gridHeight = height / playerCount; // 每个玩家的网格占用高度
    int gridWidth = width;                // 网格宽度固定为传入的宽度
    int cellSize = std::min(gridWidth / 10, gridHeight / 20); // 缩小版网格每个单元格大小
    int gridYOffset = y;

    // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    for (const auto& [playerId, state] : playerStates) {
        // 反序列化玩家的网格状态
        Grid tempGrid(10, 20); // 假设网格是固定大小
        std::istringstream stateStream(state);
        std::string gridData, blockData, scoreData;
        std::getline(stateStream, gridData, '|');
        tempGrid.deserialize(gridData);

        // 绘制背景
        SDL_Rect gridBackground = {x, gridYOffset, gridWidth, gridHeight};
        SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
        SDL_RenderFillRect(renderer, &gridBackground);

        // 绘制网格中的固定方块
        const auto& gridMatrix = tempGrid.getGrid();
        const auto& gridColors = tempGrid.getGridColors();
        for (int i = 0; i < tempGrid.getHeight(); ++i) {
            for (int j = 0; j < tempGrid.getWidth(); ++j) {
                if (gridMatrix[i][j]) {
                    SDL_Rect rect = {
                        x + j * cellSize,
                        gridYOffset + i * cellSize,
                        cellSize,
                        cellSize
                    };
                    int color = gridColors[i][j];
                    SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 255);
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }

        // 绘制玩家名称或标识
        TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 16);
        if (font) {
            SDL_Color textColor = {255, 255, 255, 255};
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, playerId.c_str(), textColor);
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                SDL_Rect textRect = {x + 5, gridYOffset + 5, textSurface->w, textSurface->h};
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_FreeSurface(textSurface);
                SDL_DestroyTexture(textTexture);
            }
            TTF_CloseFont(font);
        }

        // 更新 Y 偏移以渲染下一个玩家
        gridYOffset += gridHeight + 10; // 每个玩家网格之间有10像素间隔
    }
}

void OnlineGame::show() {
    if(gameStarted){
        return;
    }
    gameStarted = true;
    reset();
    Uint32 lastTime = SDL_GetTicks();
    while (!quit) {
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
        
        SDL_Delay(16); // 控制帧率 (约60 FPS)
    }
}