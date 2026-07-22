#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include <queue>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <websocketpp/common/connection_hdl.hpp>

#include "room.hpp"
#include "../db/repositories/user_repository.hpp" 

class RoomManager {
private:
    std::unordered_map<std::string, std::shared_ptr<Room>> m_rooms;
    mutable std::mutex m_managerMutex;
    std::function<void(websocketpp::connection_hdl, const std::string&)> m_sendCallback;
    UserRepository& m_userRepo;

    // --- Thread Pool ---
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks; 
    std::mutex m_queueMutex;
    std::condition_variable m_condition; 
    std::atomic<bool> m_stopPool;

    std::thread m_tickTimerThread; 
    
    void workerLoop();
    void tickTimerLoop();

public:
    RoomManager(std::function<void(websocketpp::connection_hdl, const std::string&)> sendCb, UserRepository& repo);
    ~RoomManager();

    bool createRoom(const std::string& roomName);
    std::shared_ptr<Room> getRoom(const std::string& roomName);
    bool removeRoom(const std::string& roomName);
    void checkAllRoomsForTimeouts(); 
};