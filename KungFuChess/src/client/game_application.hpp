#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>  
#include <mutex>
#include <atomic>
#include <chrono>
#include <nlohmann/json.hpp>

#include "network/network_client.hpp"
#include "model/game_snapshot.hpp"
#include "model/event_bus.hpp"
#include "view/renderer.hpp"
#include "view/score_manager.hpp"
#include "view/move_history_manager.hpp"
#include "input/controller.hpp"
#include "input/board_mapper.hpp"
#include "windows/login_dialog.hpp"
#include "windows/home_dialog.hpp"
#include "windows/room_dialog.hpp"
#include "view/theme.hpp"

class GameApplication {
private:
    NetworkClient network;
    EventBus clientBus;
    
    std::mutex snapshotMutex;
    GameSnapshot localSnapshot;
    std::chrono::high_resolution_clock::time_point startTime;
    std::atomic<long> timeOffset{0};

    std::atomic<int> loginStatus{0};
    std::string loginErrorMessage;
    std::mutex loginMutex;

    std::atomic<int> roomStatus{0};
    std::string roomErrorMessage;
    std::mutex roomMutex;

    void setupNetworkCallbacks();
    void handleServerMessage(const std::string& msg);
    void parseGameState(const nlohmann::json& j);
    
    bool performLogin(std::string& outUsername);
    bool joinRoom(const std::string& username);
    void runGameLoop(const std::string& username);

public:
    GameApplication();
    void run();
};