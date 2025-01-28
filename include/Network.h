#ifndef NETWORK_H
#define NETWORK_H

#include <boost/asio.hpp>
#include <functional>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include "RoomView.h"

class Network {
public:
    Network();

    // Start listening for room updates on a specified port
    void startListening(int port);

    // Join a room at a specific address and port
    bool joinRoom(const std::string& address, int port);

    // Broadcast the current room state to the network
    void initializeEndPoints();
    void broadcastRoomState(const std::string& message, int port);
    int stopListening();
    // Set a callback function to handle room state updates
    std::function<void(const std::string&)> onRoomStateUpdate;
    std::string getLocalIPAddress();
    void broadcastPlayerList(); // Broadcast current player list
    void syncPlayerList(const boost::asio::ip::udp::endpoint& target); // Sync player list to a specific player
    void addPlayer(const std::string& playerName); // Add player to the list
    void removePlayer(const std::string& playerName); // Remove player from the list
    std::vector<std::string> getPlayerList() const;
    boost::asio::ip::udp::endpoint getHostEndpoint() const;
    RoomView* roomView;
    std::mutex roomViewMutex;

    void initializeRoomView(SDL_Renderer* renderer, bool isHost, const std::string& playerName);
    void handleReadyState(const std::string& message);
    void startGameSession();
    bool allPlayersReady() const;

    void broadcastGameState(const std::string& state); // 广播当前状态
    std::vector<std::pair<std::string, std::string>> receiveGameStateUpdates(); // 接收状态更新
    void setNotifyGameStateCallback(const std::function<void(const std::string&, const std::string&)>& callback);
    void listenForGameUpdates();
    void setOnGameStartCallback(const std::function<void()>& callback);

private:
    void listenForUpdates();
    void handleRoomStateUpdate(const std::string& message);
    void handleJoinRoomRequest(const boost::asio::ip::udp::endpoint& clientEndpoint, const std::string& message);
    void handlePlayerListUpdate(const std::string& message);
    bool gameSessionStarted = false;

    boost::asio::io_context ioContext;
    boost::asio::ip::udp::socket socket;
    boost::asio::ip::udp::endpoint senderEndpoint;
    std::vector<char> buffer = std::vector<char>(1024);
    std::vector<std::string> playerList;
    std::vector<boost::asio::ip::udp::endpoint> connectedEndpoints;
    boost::asio::ip::udp::endpoint hostEndpoint;

    std::function<void()> onGameStartCallback; // 游戏开始的回调
    std::function<void(const std::string&, const std::string&)> notifyGameStateCallback; // 状态更新的回调
    bool gameStarted = false;
};

extern std::ofstream logFile;
extern void log(const std::string& message);

#endif // NETWORK_H
