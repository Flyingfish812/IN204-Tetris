#include "Network.h"
#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>

extern std::ofstream logFile;
extern void log(const std::string& message);

Network::Network() : ioContext(), socket(ioContext) {
    // ioContext.run();
    playerList.reserve(4);
    connectedEndpoints.reserve(10);
    log("Network initialized");
}

void Network::startListening(int port) {
    try {
        boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::udp::v4(), port);
        socket.open(endpoint.protocol());
        socket.set_option(boost::asio::socket_base::reuse_address(true));
        socket.bind(endpoint);
        hostEndpoint = endpoint;
        // auto assignedPort = socket.local_endpoint().port();
        log("Listening started on port " + socket.local_endpoint().address().to_string() + ":" + std::to_string(port));
        if (!gameStarted) {
            listenForUpdates();
        } else {
            listenForGameUpdates();
        }
        std::thread([this]() {
            try {
                ioContext.run();
            } catch (const std::exception& e) {
                log("ioContext.run() error: " + std::string(e.what()));
            }
        }).detach();
    } catch (const std::exception& e) {
        std::cerr << "Error starting listening: " << e.what() << std::endl;
        log("Error starting listening: " + std::string(e.what()));
    }
}

void Network::listenForUpdates() {
    socket.async_receive_from(
        boost::asio::buffer(buffer), senderEndpoint,
        [this](const boost::system::error_code& error, std::size_t bytesTransferred) {
            if (!error) {
                std::string message(buffer.data(), bytesTransferred);
                log("Message received: " + message);

                if (message.rfind("PLAYER_LIST:", 0) == 0) {
                    handlePlayerListUpdate(message);
                } else if (message == "JOIN_ROOM") {
                    handleJoinRoomRequest(senderEndpoint, message);
                } else if (message.rfind("NEW_CLIENT:", 0) == 0) {
                    // 处理新客户端广播
                    std::string endpointStr = message.substr(11); // 跳过 "NEW_CLIENT:"
                    auto delimiterPos = endpointStr.find(':');
                    std::string ip = endpointStr.substr(0, delimiterPos);
                    int port = std::stoi(endpointStr.substr(delimiterPos + 1));
                    boost::asio::ip::udp::endpoint newClientEndpoint(boost::asio::ip::make_address(ip), port);

                    if (std::find(connectedEndpoints.begin(), connectedEndpoints.end(), newClientEndpoint) == connectedEndpoints.end()) {
                        connectedEndpoints.push_back(newClientEndpoint);
                        log("New client added to connectedEndpoints: " + newClientEndpoint.address().to_string() + ":" + std::to_string(newClientEndpoint.port()));
                    }
                } else if (gameStarted) {
                    if (notifyGameStateCallback) {
                        log("Notifying");
                        notifyGameStateCallback(senderEndpoint.address().to_string(), message);
                    }
                } else {
                    handleRoomStateUpdate(message);
                }
                log("Listening for new messages...");
                listenForUpdates();
            } else if (error == boost::asio::error::operation_aborted) {
                log("Receive operation aborted.");
            } else {
                log("Receive error: " + error.message());
                listenForUpdates();
            }
        });
}

void Network::initializeEndPoints(){
    connectedEndpoints.clear();
    std::string ip = getLocalIPAddress();
    boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::make_address(ip), 12345);
    connectedEndpoints.push_back(endpoint);
}

void Network::handleRoomStateUpdate(const std::string& message) {
    // Process the message and update room state
    if (message == "START_GAME") {
        if (gameStarted) {
            log("Game already started. Ignoring duplicate START_GAME message.");
            return;
        }
        gameStarted = true;
        log("Received START_GAME. Transitioning to game mode...");

        if (onGameStartCallback) {
            onGameStartCallback();
        }
        if (roomView) {
            roomView->quitRendering();
        }
        log("RoomView quit rendering.");

        return;
    } 
    // Notify listeners
    if (onRoomStateUpdate) {
        log("Room update received: " + message);
        std::cout << "Room update received: " + message << std::endl;
        onRoomStateUpdate(message);
    }
}

bool Network::joinRoom(const std::string& address, int port) {
    try {
        boost::asio::ip::udp::endpoint remoteEndpoint(boost::asio::ip::make_address(address), port);
        hostEndpoint = remoteEndpoint;

        // 发送加入房间请求
        std::string joinMessage = "JOIN_ROOM";
        socket.send_to(boost::asio::buffer(joinMessage), remoteEndpoint);
        log("Joining room at " + address + ":" + std::to_string(port));

        // 接收房主发送的响应
        char buffer[1024] = {0};
        boost::asio::ip::udp::endpoint senderEndpoint;
        size_t bytesReceived = socket.receive_from(boost::asio::buffer(buffer), senderEndpoint);

        // 解析房主的响应消息
        std::string response(buffer, bytesReceived);
        if (response.rfind("PLAYER_LIST:", 0) == 0) {
            playerList.clear();
            std::istringstream ss(response.substr(12)); // 跳过 "PLAYER_LIST:"
            std::string player;
            while (std::getline(ss, player, ',')) {
                playerList.push_back(player);
            }
            log("Player list synced successfully.");
        } else if (response.rfind("ENDPOINT_LIST:", 0) == 0) {
            connectedEndpoints.clear();
            std::istringstream ss(response.substr(14)); // 跳过 "ENDPOINT_LIST:"
            std::string endpointStr;
            while (std::getline(ss, endpointStr, ',')) {
                auto delimiterPos = endpointStr.find(':');
                std::string ip = endpointStr.substr(0, delimiterPos);
                int port = std::stoi(endpointStr.substr(delimiterPos + 1));
                boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::make_address(ip), port);
                connectedEndpoints.push_back(endpoint);
            }
            log("Connected endpoints synced successfully.");
        } else {
            log("Unexpected message from host: " + response);
            return false;
        }

        // 广播更新后的玩家列表给所有客户端
        broadcastPlayerList();

        return true;
    } catch (const std::exception& e) {
        log("Error joining room: " + std::string(e.what()));
        return false;
    }
}

void Network::handleJoinRoomRequest(const boost::asio::ip::udp::endpoint& clientEndpoint, const std::string& message) {
    if (message == "JOIN_ROOM") {
        // 添加新客户端到房主的 connectedEndpoints
        if (std::find(connectedEndpoints.begin(), connectedEndpoints.end(), clientEndpoint) == connectedEndpoints.end()) {
            connectedEndpoints.push_back(clientEndpoint);
            log("Added new client to connectedEndpoints: " + clientEndpoint.address().to_string() + ":" + std::to_string(clientEndpoint.port()));
        }

        // 构造玩家列表响应并发送给新客户端
        std::string playerListMessage = "PLAYER_LIST:";
        for (const auto& player : playerList) {
            playerListMessage += player + ",";
        }
        if (!playerList.empty()) {
            playerListMessage.pop_back(); // 移除最后的逗号
        }
        socket.send_to(boost::asio::buffer(playerListMessage), clientEndpoint);
        log("Sent player list to new client: " + playerListMessage);

        // 构造终端节点列表并发送给新客户端
        std::string endpointListMessage = "ENDPOINT_LIST:";
        for (const auto& endpoint : connectedEndpoints) {
            endpointListMessage += endpoint.address().to_string() + ":" + std::to_string(endpoint.port()) + ",";
        }
        if (!connectedEndpoints.empty()) {
            endpointListMessage.pop_back(); // 移除最后的逗号
        }
        socket.send_to(boost::asio::buffer(endpointListMessage), clientEndpoint);
        log("Sent endpoint list to new client: " + endpointListMessage);

        // 向房间内其他客户端广播新客户端的终端节点
        std::string newClientMessage = "NEW_CLIENT:" + clientEndpoint.address().to_string() + ":" + std::to_string(clientEndpoint.port());
        for (const auto& endpoint : connectedEndpoints) {
            if (endpoint != clientEndpoint) { // 不向新客户端重复发送
                socket.send_to(boost::asio::buffer(newClientMessage), endpoint);
                log("Broadcasted new client to: " + endpoint.address().to_string() + ":" + std::to_string(endpoint.port()));
            }
        }

        // 广播更新后的玩家列表给所有客户端
        broadcastPlayerList();
    }
}

void Network::broadcastRoomState(const std::string& message, int port) {
    try {
        boost::asio::ip::udp::endpoint broadcastEndpoint(boost::asio::ip::address_v4::broadcast(), port);
        socket.set_option(boost::asio::socket_base::broadcast(true));

        // 定时器共享指针
        auto timer = std::make_shared<boost::asio::steady_timer>(ioContext);

        // 捕获 timerCallback 为值，避免生命周期问题
        auto timerCallback = std::make_shared<std::function<void(const boost::system::error_code&)>>();
        *timerCallback = [this, timer, message, broadcastEndpoint, timerCallback](const boost::system::error_code& error) {
            if (!error) {
                // 发送广播消息
                if(!gameStarted){
                    socket.send_to(boost::asio::buffer(message), broadcastEndpoint);
                    log("Broadcast message sent: " + message);

                    // 设置下一次定时器
                    timer->expires_after(std::chrono::seconds(2));
                    timer->async_wait(*timerCallback);
                }
            } else {
                log("Timer error: " + error.message());
            }
        };

        // 初始定时器设置和启动
        timer->expires_after(std::chrono::seconds(0)); // 立即启动
        timer->async_wait(*timerCallback);

        // 启动 ioContext 的事件循环
        std::thread([this]() {
            try {
                log("Starting ioContext.run() for broadcast.");
                ioContext.run();
            } catch (const std::exception& e) {
                log("ioContext.run() error: " + std::string(e.what()));
            }
        }).detach();

    } catch (const std::exception& e) {
        log("Error broadcasting room state: " + std::string(e.what()));
    }
}

int Network::stopListening() {
    int port = 0; // 默认返回值为 0，表示未成功获取端口号
    try {
        // 确保在关闭 socket 之前获取端口号
        if (socket.is_open()) {
            port = socket.local_endpoint().port(); // 获取监听的端口号
        }

        // 停止 io_context 和关闭 socket
        ioContext.stop();
        socket.close();
        log("Listening stopped. Port: " + std::to_string(port));
    } catch (const std::exception& e) {
        std::cerr << "Error stopping listening: " << e.what() << std::endl;
        log("Error stopping listening: " + std::string(e.what()));
    }

    return port;
}

std::string Network::getLocalIPAddress() {
    try {
        boost::asio::io_context ioContext;
        boost::asio::ip::udp::resolver resolver(ioContext);
        auto endpoints = resolver.resolve(boost::asio::ip::host_name(), "");

        for (auto& endpoint : endpoints) {
            auto address = endpoint.endpoint().address();
            if (address.is_v4() && !address.is_loopback()) {
                return address.to_string();
            }
        }
    } catch (const std::exception& e) {
        log("Error retrieving local IP address: " + std::string(e.what()));
    }
    return "127.0.0.1"; // Fallback to localhost if no suitable IP is found
}

void Network::addPlayer(const std::string& playerName) {
    playerList.push_back(playerName);
    broadcastPlayerList();
}

void Network::removePlayer(const std::string& playerName) {
    playerList.erase(std::remove(playerList.begin(), playerList.end(), playerName), playerList.end());
    broadcastPlayerList();
}

void Network::broadcastPlayerList() {
    std::string listMessage = "PLAYER_LIST:";
    for (const auto& player : playerList) {
        listMessage += player + ",";
    }
    if (!playerList.empty()) {
        listMessage.pop_back(); // 移除最后的逗号
    }
    for (const auto& endpoint : connectedEndpoints) {
        socket.send_to(boost::asio::buffer(listMessage), endpoint);
        log("Broadcasted player list to: " + endpoint.address().to_string() + ":" + std::to_string(endpoint.port()));
    }
    // log("Broadcasted player list to all clients: " + listMessage);
}

void Network::syncPlayerList(const boost::asio::ip::udp::endpoint& target) {
    std::string listMessage = "PLAYER_LIST:";
    for (const auto& player : playerList) {
        listMessage += player + ",";
    }
    listMessage.pop_back(); // Remove trailing comma
    log(listMessage);
    socket.send_to(boost::asio::buffer(listMessage), target);
}

void Network::handlePlayerListUpdate(const std::string& message) {
    if (message.rfind("PLAYER_LIST:", 0) == 0) {
        playerList.clear();
        std::istringstream ss(message.substr(12)); // 跳过 "PLAYER_LIST:"
        std::string player;
        while (std::getline(ss, player, ',')) {
            playerList.push_back(player);
        }
        log("Player list updated: " + message);
        // std::lock_guard<std::mutex> lock(roomViewMutex);
        if (roomView) {
            roomView->updatePlayers(playerList);
        } else {
            log("roomView is not initialized.");
        }
    }
    log("Done.");
}

boost::asio::ip::udp::endpoint Network::getHostEndpoint() const {
    return hostEndpoint; // Assuming this is set when the room is created or joined
}

std::vector<std::string> Network::getPlayerList() const {
    return playerList;
}


void Network::initializeRoomView(SDL_Renderer* renderer, bool isHost, const std::string& playerName) {
    roomView = new RoomView(renderer, isHost);

    // 设置 Leave Room 回调
    roomView->setLeaveRoomCallback([this, playerName]() {
        removePlayer(playerName);  // 使用传递的玩家名称
        broadcastPlayerList();
        log("Player left the room: " + playerName);
        roomView->quitRendering();
    });

    // 设置 Ready 回调
    roomView->setReadyCallback([this, playerName](bool isReady) {
        std::string message = isReady ? "READY:" + playerName : "CANCEL_READY:" + playerName;
        handleReadyState(message);
        broadcastPlayerList();
        if (roomView) {
            roomView->updatePlayers(playerList);  // 刷新玩家列表显示
        }
        log(playerName + (isReady ? " is ready." : " canceled ready."));
    });

    // 设置 Start Game 回调（仅限房主）
    if (isHost) {
        roomView->setStartGameCallback([this]() {
            if (allPlayersReady()) {
                startGameSession();
                if(roomView) {
                    roomView->quitRendering();
                }
            } else {
                log("Not all players are ready.");
            }
        });
    }
}

void Network::handleReadyState(const std::string& message) {
    if (message.rfind("READY:", 0) == 0) {
        std::string playerName = message.substr(6);  // 提取玩家名称
        auto it = std::find(playerList.begin(), playerList.end(), playerName);
        if (it != playerList.end() && it->find("(Ready)") == std::string::npos) {
            *it += " (Ready)";
        }
    } else if (message.rfind("CANCEL_READY:", 0) == 0) {
        std::string playerName = message.substr(13);  // 提取玩家名称
        auto it = std::find(playerList.begin(), playerList.end(), playerName);
        if (it != playerList.end()) {
            *it = playerName;  // 移除 "(Ready)" 标记
        }
    }
}

void Network::startGameSession() {
    if (!notifyGameStateCallback) {
        log("Warning: No game state callback set. State updates may be ignored.");
    }
    if (!gameSessionStarted) {
        // 仅在首次调用时广播游戏开始消息
        std::string startMessage = "START_GAME";
        for (const auto& endpoint : connectedEndpoints) {
            socket.send_to(boost::asio::buffer(startMessage), endpoint);
            log("Sent START_GAME to: " + endpoint.address().to_string() + ":" + std::to_string(endpoint.port()));
        }
        gameSessionStarted = true; // 标记游戏会话已启动
        log("Game session started.");
    }

    // listenForGameUpdates();
}

void Network::listenForGameUpdates() {
    socket.async_receive_from(
        boost::asio::buffer(buffer), senderEndpoint,
        [this](const boost::system::error_code& error, std::size_t bytesTransferred) {
            if (!error) {
                std::string state(buffer.data(), bytesTransferred);
                log("Message received: " + state);
                // 通知游戏模块处理接收到的状态
                if (notifyGameStateCallback) {
                    notifyGameStateCallback(senderEndpoint.address().to_string(), state);
                }

                // 继续监听
                listenForGameUpdates();
            } else if (error == boost::asio::error::operation_aborted) {
                log("Game session listening stopped.");
            } else {
                log("Error receiving game state: " + error.message());
                listenForGameUpdates();
            }
        });
}

void Network::setNotifyGameStateCallback(const std::function<void(const std::string&, const std::string&)>& callback) {
    notifyGameStateCallback = callback;
    log("Game state callback set.");
}

bool Network::allPlayersReady() const {
    for (const auto& player : playerList) {
        if (player.find("(Ready)") == std::string::npos) {
            return false;
        }
    }
    return true;
}

void Network::broadcastGameState(const std::string& state) {
    for (const auto& endpoint : connectedEndpoints) {
        socket.send_to(boost::asio::buffer(state), endpoint);
    }
    log("Broadcasted game state: " + state);
}

std::vector<std::pair<std::string, std::string>> Network::receiveGameStateUpdates() {
    std::vector<std::pair<std::string, std::string>> updates;
    while (true) {
        char buffer[1024] = {0};
        boost::asio::ip::udp::endpoint senderEndpoint;
        size_t bytesReceived = socket.receive_from(boost::asio::buffer(buffer), senderEndpoint);
        if (bytesReceived > 0) {
            std::string state(buffer, bytesReceived);
            updates.emplace_back(senderEndpoint.address().to_string(), state);
        }
    }
    return updates;
}

void Network::setOnGameStartCallback(const std::function<void()>& callback) {
    onGameStartCallback = callback;
}