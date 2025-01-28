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

    void syncState();
    void handleRemoteState(const std::string& playerId, const std::string& state);

private:
    Network* network;
    std::map<std::string, std::string> playerStates;
    void renderOtherPlayers(int x, int y, int width, int height);
};

#endif // ONLINE_GAME_H
