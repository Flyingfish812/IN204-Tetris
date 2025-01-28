#ifndef ROOM_VIEW_H
#define ROOM_VIEW_H

#include <vector>
#include <string>
#include <functional>
#include "Button.h"
#include <SDL.h>

class RoomView {
public:
    RoomView(SDL_Renderer* renderer, bool isHost);

    void addPlayer(const std::string& playerName);
    void removePlayer(const std::string& playerName);

    void render();
    bool host(){return isHost;}
    void updatePlayers(const std::vector<std::string>& updatedPlayers);
    // void updateReadyState(const std::string& playerName, bool isReady);

    void setLeaveRoomCallback(std::function<void()> callback);
    void setReadyCallback(std::function<void(bool)> callback);
    void setStartGameCallback(std::function<void()> callback);

    void quitRendering();
    
private:
    bool quit;
    SDL_Renderer* renderer;
    bool isHost;
    bool isReady = false;
    size_t selectedButtonIndex;
    std::vector<std::string> players;
    std::vector<Button> buttons;
    bool isRendered = false;
    void renderPlayers();

    std::function<void()> leaveRoomCallback;
    std::function<void(bool)> readyCallback;
    std::function<void()> startGameCallback;
};

#endif // ROOM_VIEW_H
