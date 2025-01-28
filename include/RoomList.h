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

    // Set the title of the room list
    void setTitle(const std::string& titleName) { title = titleName; }

    // Add a room to the list
    void addRoom(const std::string& roomInfo);

    // Refresh the room list with a new set of rooms
    void refreshRoomList(const std::vector<std::string>& newRoomList);

    // Set callback for when a room is selected
    void setRoomSelectedCallback(const std::function<void(const std::string&)>& callback);

    // Set callback for the return button
    void setReturnCallback(const std::function<void()>& callback);

    // Set callback for refreshing rooms periodically
    void setRefreshCallback(const std::function<void()>& callback);

    // Display the room list interface
    void show();

private:
    SDL_Renderer* renderer;
    std::vector<Button> buttons;
    size_t selectedButtonIndex;
    std::string title;
    std::function<void(const std::string&)> onRoomSelected;
    std::function<void()> returnCallback;
    std::function<void()> onRefreshRooms;
    Uint32 refreshInterval; // Interval for refreshing room list (in milliseconds)
    Uint32 lastRefreshTime; // Timestamp of the last refresh
    std::unordered_set<std::string> currentRooms;
};

#endif // ROOM_LIST_H
