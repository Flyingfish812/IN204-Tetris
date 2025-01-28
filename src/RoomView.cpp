#include "RoomView.h"
#include <SDL_ttf.h>
#include <iostream>
#include <fstream>
#include <algorithm>

extern std::ofstream logFile;
extern void log(const std::string& message);

// Initialize, the buttons
RoomView::RoomView(SDL_Renderer* renderer, bool isHost)
    : renderer(renderer), isHost(isHost), selectedButtonIndex(0) {
    buttons.reserve(2);
    buttons.emplace_back(renderer, "Leave Room", SDL_Rect{100, 500, 200, 50}, [this]() {
        if (leaveRoomCallback) {
            leaveRoomCallback();
        }
    });

    if (isHost) {
        buttons.emplace_back(renderer, "Start Game", SDL_Rect{500, 500, 200, 50}, [this]() {
            if (startGameCallback) {
                startGameCallback();
            }
        });
    } else {
        buttons.emplace_back(renderer, "Ready", SDL_Rect{500, 500, 200, 50}, [this]() {
            isReady = !isReady;
            if (readyCallback) {
                readyCallback(isReady);
            }
        });
    }
}

void RoomView::setLeaveRoomCallback(std::function<void()> callback) {
    leaveRoomCallback = callback;
}

void RoomView::setReadyCallback(std::function<void(bool)> callback) {
    readyCallback = callback;
}

void RoomView::setStartGameCallback(std::function<void()> callback) {
    startGameCallback = callback;
}

void RoomView::addPlayer(const std::string& playerName) {
    players.push_back(playerName);
}

void RoomView::removePlayer(const std::string& playerName) {
    players.erase(std::remove(players.begin(), players.end(), playerName), players.end());
}

void RoomView::quitRendering() {
    quit = true; // 当所有玩家准备好时退出界面
}

void RoomView::render() {
    if(isRendered){
        return;
    } else {
        isRendered = true;
    }
    log("Rendering RoomView...");
    SDL_Event e;
    quit = false;

    while (!quit) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderPlayers();

        // Buttons
        for (size_t i = 0; i < buttons.size(); ++i) {
            buttons[i].render(i == selectedButtonIndex);
        }

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_LEFT) {
                    selectedButtonIndex = (selectedButtonIndex - 1 + buttons.size()) % buttons.size();
                } else if (e.key.keysym.sym == SDLK_RIGHT) {
                    selectedButtonIndex = (selectedButtonIndex + 1) % buttons.size();
                } else if (e.key.keysym.sym == SDLK_RETURN) {
                    buttons[selectedButtonIndex].handleClick();
                }
            }
        }
    }
}

void RoomView::renderPlayers() {
    SDL_Rect playerListArea = {0, 0, 800, 400};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // 使用黑色清空
    SDL_RenderFillRect(renderer, &playerListArea);
    TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 24); // 确保字体文件路径正确
    if (!font) {
        logFile << "Failed to load font: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Color color = {255, 255, 255, 255};
    int yOffset = 50;

    for (const auto& player : players) {
        SDL_Surface* surface = TTF_RenderText_Solid(font, player.c_str(), color);
        if (!surface) {
            logFile << "Failed to create text surface: " << TTF_GetError() << std::endl;
            continue;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture) {
            logFile << "Failed to create text texture: " << SDL_GetError() << std::endl;
            SDL_FreeSurface(surface);
            continue;
        }

        SDL_Rect dstRect = {50, yOffset, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &dstRect);

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        yOffset += 30;
    }

    TTF_CloseFont(font);
}

void RoomView::updatePlayers(const std::vector<std::string>& updatedPlayers) {
    players = updatedPlayers;
    log("Player list updated.");
}

