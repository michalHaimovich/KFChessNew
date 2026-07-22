#pragma once
#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <websocketpp/common/connection_hdl.hpp>
#include <nlohmann/json.hpp>
#include <mutex>
#include <shared_mutex>

#include "model/event_bus.hpp"
#include "engine/game_engine.hpp"
#include "../db/repositories/user_repository.hpp"

using json = nlohmann::json;

enum class PlayerRole
{
    White,
    Black,
    Spectator
};

class GameSession
{
private:
    GameEngine engine;

    std::map<websocketpp::connection_hdl, PlayerRole, std::owner_less<websocketpp::connection_hdl>> players;

    bool whiteTaken;
    bool blackTaken;

    std::string m_whiteName = "Waiting...";
    std::string m_blackName = "Waiting...";

    std::thread timeThread;
    std::atomic<bool> isRunning;

    std::function<void(websocketpp::connection_hdl, const std::string &)> sendCallback;

    mutable std::mutex playersMutex;

    EventBus serverBus;
    std::vector<json> pendingEvents;
    mutable std::mutex eventsMutex;

    void gameLoop();
    
    void processGameOver(const GameSnapshot& snap);

    UserRepository& userRepo; 
    bool eloUpdated = false;

    // Helper function to serialize the snapshot into JSON string
    std::string serializeSnapshot(const GameSnapshot &snap);

public:
    GameSession(std::function<void(websocketpp::connection_hdl, const std::string &)> sendCb, UserRepository &repo);
    ~GameSession();

    PlayerRole addClient(websocketpp::connection_hdl hdl, const std::string &username);
    void removeClient(websocketpp::connection_hdl hdl);
    PlayerRole getRole(websocketpp::connection_hdl hdl) const;

    void broadcastMessage(const std::string &msg);

    GameEngine &getEngine();
};