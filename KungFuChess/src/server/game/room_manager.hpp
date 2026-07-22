#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <websocketpp/common/connection_hdl.hpp>

#include "room.hpp"
#include "../db/repositories/user_repository.hpp" 

class RoomManager {
private:
    std::unordered_map<std::string, std::shared_ptr<Room>> m_rooms;
    
    mutable std::mutex m_managerMutex;

    std::function<void(websocketpp::connection_hdl, const std::string&)> m_sendCallback;
    
    UserRepository& m_userRepo;
public:
    RoomManager(std::function<void(websocketpp::connection_hdl, const std::string&)> sendCb, UserRepository& repo);
    ~RoomManager() = default;

    bool createRoom(const std::string& roomName);
    std::shared_ptr<Room> getRoom(const std::string& roomName);
    bool removeRoom(const std::string& roomName);
    void checkAllRoomsForTimeouts(); 
};