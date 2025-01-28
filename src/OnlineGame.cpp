#include "OnlineGame.h"
#include <sstream>

OnlineGame::OnlineGame(SDL_Renderer* renderer, Network* network)
    : Game(renderer), network(network) {
    network->setNotifyGameStateCallback([this](const std::string& playerId, const std::string& state) {
        handleRemoteState(playerId, state);
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

// Syncronize the game state with other players
void OnlineGame::syncState() {
    std::ostringstream oss;
    oss << grid->serialize() << "|" 
        << currentBlock->serialize() << "|" 
        << score;
    network->broadcastGameState(oss.str());
}

// Handle remote player state updates
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
    
    // Store the state
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

    int gridHeight = height / playerCount;
    int gridWidth = width;
    int cellSize = std::min(gridWidth / 10, gridHeight / 20);
    int gridYOffset = y;

    for (const auto& [playerId, state] : playerStates) {
        Grid tempGrid(10, 20);
        std::istringstream stateStream(state);
        std::string gridData, blockData, scoreData;
        std::getline(stateStream, gridData, '|');
        tempGrid.deserialize(gridData);

        // Background
        SDL_Rect gridBackground = {x, gridYOffset, gridWidth, gridHeight};
        SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
        SDL_RenderFillRect(renderer, &gridBackground);

        // Blocks
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

        // Player name
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

        // Offset for next player
        gridYOffset += gridHeight + 10;
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
        
        SDL_Delay(16);
    }
}