#ifndef APPLICATION_H
#define APPLICATION_H

#include "Menu.h"
#include "Game.h"
#include "RoomView.h"
#include "Network.h"
#include "RoomList.h"
#include "OnlineGame.h"
#include <fstream>
#include <boost/asio.hpp>

extern std::ofstream logFile;

class Application {
public:
    Application();
    ~Application();
    void run();
    void handleMultiplayerMode();
    void createRoom();
    void joinRoom();
    void handleRoomSelection(const std::string& roomInfo);

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    Menu* menu;
    Menu* multiplayerMenu;
    Game* game;
    OnlineGame* onlineGame;
    // boost::asio::io_context io;
    Network network;
    bool startTogether = false;
};

#endif