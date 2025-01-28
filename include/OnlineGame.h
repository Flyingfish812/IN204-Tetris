#ifndef ONLINE_GAME_H
#define ONLINE_GAME_H

#include "Game.h"
#include "Network.h"
#include <map>

class OnlineGame : public Game {
public:
    OnlineGame(SDL_Renderer* renderer, Network* network);
    ~OnlineGame();

    void show() override;
    void handleInput() override;
    void update(Uint32 deltaTime) override;
    void render() override;

    void syncState(); // 同步当前状态到网络
    void handleRemoteState(const std::string& playerId, const std::string& state); // 处理远程玩家状态

private:
    Network* network;
    std::map<std::string, std::string> playerStates; // 其他玩家的状态映射
    void renderOtherPlayers(int x, int y, int width, int height); // 渲染其他玩家的状态
};

#endif // ONLINE_GAME_H
