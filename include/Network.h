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
    // Constructor
    Network();

    // Room management methods
    void startListening(int port);
    int stopListening();
    bool joinRoom(const std::string& address, int port);
    void broadcastRoomState(const std::string& message, int port);
    void initializeEndPoints();

    // Player management methods
    void addPlayer(const std::string& playerName);
    void removePlayer(const std::string& playerName);
    std::vector<std::string> getPlayerList() const;
    void broadcastPlayerList();
    void syncPlayerList(const boost::asio::ip::udp::endpoint& target);

    // Game state management methods
    void broadcastGameState(const std::string& state);
    std::vector<std::pair<std::string, std::string>> receiveGameStateUpdates();
    bool allPlayersReady() const;
    void startGameSession();

    // Callback setters
    void setNotifyGameStateCallback(const std::function<void(const std::string&, const std::string&)>& callback);
    void setOnGameStartCallback(const std::function<void()>& callback);

    // Room view management methods
    void initializeRoomView(SDL_Renderer* renderer, bool isHost, const std::string& playerName);
    void handleReadyState(const std::string& message);

    // TCP methods
    std::string getLocalIPAddress();
    boost::asio::ip::udp::endpoint getHostEndpoint() const;

    // Callbacks
    std::function<void(const std::string&)> onRoomStateUpdate;

    // Public member variables
    RoomView* roomView;
    std::mutex roomViewMutex;

private:
    // Internal methods
    void listenForUpdates();
    void handleRoomStateUpdate(const std::string& message);
    void handleJoinRoomRequest(const boost::asio::ip::udp::endpoint& clientEndpoint, const std::string& message);
    void handlePlayerListUpdate(const std::string& message);

    // Private member variables
    bool gameSessionStarted = false;
    bool gameStarted = false;
    boost::asio::io_context ioContext;
    boost::asio::ip::udp::socket socket;
    boost::asio::ip::udp::endpoint senderEndpoint;
    boost::asio::ip::udp::endpoint hostEndpoint;
    std::vector<char> buffer = std::vector<char>(1024);
    std::vector<std::string> playerList;
    std::vector<boost::asio::ip::udp::endpoint> connectedEndpoints;

    // Private member variables for callbacks
    std::function<void()> onGameStartCallback;
    std::function<void(const std::string&, const std::string&)> notifyGameStateCallback;
};


extern std::ofstream logFile;
extern void log(const std::string& message);

#endif // NETWORK_H
