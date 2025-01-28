#include "RoomList.h"
#include "Button.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <algorithm>
#include <SDL2/SDL_ttf.h>

extern std::ofstream logFile;
extern void log(const std::string& message);

RoomList::RoomList(SDL_Renderer* renderer) 
    : renderer(renderer), selectedButtonIndex(0), title("Available Rooms") {
    buttons.reserve(3);
    refreshInterval = 2000; // Default refresh interval in milliseconds
    lastRefreshTime = SDL_GetTicks();
}

RoomList::~RoomList() {
    TTF_Quit();
}

void RoomList::addRoom(const std::string& roomInfo) {
    SDL_Rect rect = {100, static_cast<int>(buttons.size() * 60 + 150), 600, 50};
    buttons.emplace_back(renderer, roomInfo, rect, [roomInfo, this]() {
        // log("Room selected: " + roomInfo);
        if (onRoomSelected) {
            onRoomSelected(roomInfo);
        }
    });
}

void RoomList::setReturnCallback(const std::function<void()>& callback) {
    returnCallback = callback;
}

void RoomList::setRoomSelectedCallback(const std::function<void(const std::string&)>& callback) {
    onRoomSelected = callback;
}

void RoomList::refreshRoomList(const std::vector<std::string>& newRoomList) {
    // 转换新房间列表为集合
    std::unordered_set<std::string> newRooms(newRoomList.begin(), newRoomList.end());

    // 查找需要添加的房间
    for (const auto& room : newRoomList) {
        if (currentRooms.find(room) == currentRooms.end()) {
            // 如果是新房间，添加按钮
            addRoom(room);
        }
    }

    // 查找需要移除的房间
    for (auto it = currentRooms.begin(); it != currentRooms.end();) {
        if (newRooms.find(*it) == newRooms.end()) {
            // 如果房间已不存在，移除按钮
            auto removeIt = std::find_if(buttons.begin(), buttons.end(),
                                         [&it](const Button& button) {
                                             return button.getText() == *it;
                                         });
            if (removeIt != buttons.end()) {
                buttons.erase(removeIt); // 从按钮列表中移除
            }
            it = currentRooms.erase(it); // 从当前房间集合中移除
        } else {
            ++it;
        }
    }

    // 更新当前房间集合
    currentRooms = std::move(newRooms);
}

void RoomList::show() {
    SDL_Event e;
    bool quit = false;

    while (!quit) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render title
        if (!title.empty()) {
            TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 24);

            if (font) {
                SDL_Color textColor = {255, 255, 255, 255};
                SDL_Surface* holdSurface = TTF_RenderText_Solid(font, title.c_str(), textColor);
                SDL_Texture* holdTexture = SDL_CreateTextureFromSurface(renderer, holdSurface);

                SDL_Rect holdTextRect = {400 - holdSurface->w / 2, 50, holdSurface->w, holdSurface->h};
                SDL_RenderCopy(renderer, holdTexture, NULL, &holdTextRect);

                SDL_FreeSurface(holdSurface);
                SDL_DestroyTexture(holdTexture);
            }
        }

        // Render buttons
        for (size_t i = 0; i < buttons.size(); ++i) {
            buttons[i].render(i == selectedButtonIndex);
        }

        // Render return button
        SDL_Rect returnRect = {300, 500, 200, 50};
        Button returnButton(renderer, "Return", returnRect, [this]() {
            log("Return button clicked");
            if (returnCallback) {
                returnCallback();
            }
        });
        returnButton.render(false);

        SDL_RenderPresent(renderer);

        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_UP) {
                    selectedButtonIndex = (selectedButtonIndex - 1 + buttons.size()) % buttons.size();
                } else if (e.key.keysym.sym == SDLK_DOWN) {
                    selectedButtonIndex = (selectedButtonIndex + 1) % buttons.size();
                } else if (e.key.keysym.sym == SDLK_RETURN) {
                    if (selectedButtonIndex < buttons.size()) {
                        buttons[selectedButtonIndex].handleClick();
                    } else {
                        returnCallback();
                    }
                    quit = true;
                }
            }
        }

        // Refresh room list periodically
        if (SDL_GetTicks() - lastRefreshTime > refreshInterval) {
            lastRefreshTime = SDL_GetTicks();
            if (onRefreshRooms) {
                log("Refreshing room list");
                onRefreshRooms();
            }
        }
    }
}

void RoomList::setRefreshCallback(const std::function<void()>& callback) {
    onRefreshRooms = callback;
}
