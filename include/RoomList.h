#ifndef ROOM_LIST_H
#define ROOM_LIST_H

#include <SDL2/SDL.h>
#include <vector>
#include <unordered_set>
#include <string>
#include <functional>
#include "Button.h"

class RoomList {
public:
    RoomList(SDL_Renderer* renderer);
    ~RoomList();

    void setTitle(const std::string& titleName) { title = titleName; }
    void addRoom(const std::string& roomInfo);
    void refreshRoomList(const std::vector<std::string>& newRoomList);
    void setRoomSelectedCallback(const std::function<void(const std::string&)>& callback);
    void setReturnCallback(const std::function<void()>& callback);
    void setRefreshCallback(const std::function<void()>& callback);

    void show();

private:
    SDL_Renderer* renderer;
    std::vector<Button> buttons;
    size_t selectedButtonIndex;
    std::string title;
    std::function<void(const std::string&)> onRoomSelected;
    std::function<void()> returnCallback;
    std::function<void()> onRefreshRooms;
    Uint32 refreshInterval;
    Uint32 lastRefreshTime;
    std::unordered_set<std::string> currentRooms;
};

#endif // ROOM_LIST_H
