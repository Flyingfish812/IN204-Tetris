#include "Application.h"
#include <SDL.h>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

std::ofstream logFile;

// Initialize the log file
void initLog() {
    // Get current time
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    // Format time as YYYY-MM-DD_HH-MM-SS
    std::ostringstream oss;
    oss << "logs/" << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S") << "_log.txt";
    std::string logFilePath = oss.str();

    logFile.open(logFilePath, std::ios::out | std::ios::trunc);
    if (!logFile) {
        std::cerr << "Failed to open log file: " << logFilePath << std::endl;
        exit(1);
    }
    logFile << "Log initialized at: " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << std::endl;
}

// Write a message to the log file
extern void log(const std::string& message) {
    if (logFile.is_open()) {
        logFile << message << std::endl;
    }
}

// Close the log file
void closeLog() {
    if (logFile.is_open()) {
        logFile << "Log closed." << std::endl;
        logFile.close();
    }
}

Application::Application() {
    initLog();
    log("Application starting...");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        logFile << "SDL Initialization failed: " << SDL_GetError() << std::endl;
        exit(1);
    }
    TTF_Init();
    window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Initialize the main menu
    menu = new Menu(renderer);
    menu->setTitle("Tetris");
    log("Creating buttons...");
    menu->addButton("Start", SDL_Rect{300, 200, 200, 50}, [this]() {
        log("Start the Game");
        menu->setState(MenuState::StartGame);
    });
    menu->addButton("Multi player", SDL_Rect{300, 300, 200, 50}, [this]() {
        log("Start the Multiplayer Game");
        menu->setState(MenuState::Multiplayer);
    });
    menu->addButton("Exit Game", SDL_Rect{300, 400, 200, 50}, [this]() {
        log("Exit the Game");
        menu->setState(MenuState::Exit);
    });

    // Initialize the multiplayer menu
    multiplayerMenu = new Menu(renderer);
    multiplayerMenu->setTitle("Multiplayer Choices");
    multiplayerMenu->addButton("Create Room", SDL_Rect{300, 200, 200, 50}, [this]() {
        log("Create Room selected!");
        createRoom();
        menu->setState(MenuState::Multiplayer);
    });
    multiplayerMenu->addButton("Join Room", SDL_Rect{300, 300, 200, 50}, [this]() {
        log("Join Room selected!");
        joinRoom();
        menu->setState(MenuState::Multiplayer);
    });
    multiplayerMenu->addButton("Back", SDL_Rect{300, 400, 200, 50}, [this]() {
        log("Returning to main menu.");
        menu->setState(MenuState::None);
    });

    // Initialize the game
    game = new Game(renderer);
    onlineGame = new OnlineGame(renderer, &network);

    network.setOnGameStartCallback([this]() {
        log("Game started. Transitioning to game screen...");
        startTogether = true;
    });
}

Application::~Application() {
    log("Application closing...");
    delete menu;
    delete game;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    log("Closed Successfully.");
}

void Application::run() {
    bool running = true;
    while (running) {
        menu->show();
        MenuState state = menu->getSelectedState();

        switch (state) {
            case MenuState::StartGame:
                game->show();
                break;
            case MenuState::Multiplayer:
                multiplayerMenu->show();
                break;
            case MenuState::Exit:
                running = false;
                break;
            default:
                break;
        }
        menu->resetState();
    }
}

void Application::createRoom() {
    // Room setup
    network.initializeRoomView(renderer, true, "Host");
    int roomPort = 12345;
    int destPort = 54321;
    network.initializeEndPoints();
    network.startListening(roomPort);
    log("Room created and listening started on port " + std::to_string(roomPort));

    // Broadcast room info
    std::string localIP = network.getLocalIPAddress();
    std::string roomInfo = "Room hosted by " + localIP;
    network.broadcastRoomState(roomInfo, destPort);
    log("Broadcasting room info: " + roomInfo);

    // Add host to the room
    network.addPlayer("Host (Ready)");
    log("Host added to the room.");

    network.roomView->addPlayer("Host (Ready)");
    log("Switched to RoomView rendering after creating the room.");
    network.roomView->render();

    // Start the game if all players are ready
    if (startTogether) {
        onlineGame->show();
        startTogether = false;
    }

    network.stopListening();

    delete network.roomView;
    network.roomView = nullptr;
}

void Application::joinRoom() {
    network.initializeRoomView(renderer, false, "Guest Player");
    int listenPort = 54321;
    log("Listening for room broadcasts on port " + std::to_string(listenPort));

    // Create a RoomList instance
    RoomList roomList(renderer);
    roomList.setTitle("Available Rooms");

    // Set callback for room selection
    roomList.setRoomSelectedCallback([this](const std::string& roomInfo) {
        log("Room selected: " + roomInfo);
        this->handleRoomSelection(roomInfo);
    });

    // Set callback for return action
    roomList.setReturnCallback([this]() {
        log("Return to main menu");
    });

    // Set callback for refreshing room list
    roomList.setRefreshCallback([this, &roomList]() {
        log("Refreshing room list");
        // Update the room list based on current network state
        std::vector<std::string> updatedRooms = network.getPlayerList(); // Use existing Network method
        roomList.refreshRoomList(updatedRooms);
    });

    // Update room list when receiving broadcasts
    network.onRoomStateUpdate = [&roomList](const std::string& roomInfo) {
        log("Received room broadcast: " + roomInfo);
        roomList.addRoom(roomInfo);
    };

    // Start listening for room broadcasts
    network.startListening(listenPort);
    log("Showing available rooms");
    roomList.show();

    // Stop listening after exiting the room list interface
    network.stopListening();
}

void Application::handleRoomSelection(const std::string& roomInfo) {
    log("Joining room: " + roomInfo);

    // Get the address and the port
    std::string hostAddress;
    const std::string prefix = "Room hosted by ";
    size_t startPos = roomInfo.find(prefix);
    if (startPos != std::string::npos) {
        startPos += prefix.length();
        hostAddress = roomInfo.substr(startPos);
    } else {
        log("Invalid roomInfo format: " + roomInfo + ", use default address");
        hostAddress = "127.0.0.1";
    }
    
    int hostPort = 12345;
    network.joinRoom(hostAddress, hostPort);

    // Renew the player list
    for (const auto& player : network.getPlayerList()) {
        network.roomView->addPlayer(player);
    }
    std::string playerName = "Guest Player";
    network.addPlayer(playerName);
    network.roomView->addPlayer(playerName);
    log("Switched to RoomView rendering with updated player list.");
    network.roomView->render();

    // Game start
    if(startTogether) {
        onlineGame->show();
        startTogether = false;
    }

    delete network.roomView;
    network.roomView = nullptr;
}