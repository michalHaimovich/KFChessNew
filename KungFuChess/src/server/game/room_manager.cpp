#include "room_manager.hpp"

RoomManager::RoomManager(std::function<void(websocketpp::connection_hdl, const std::string&)> sendCb)
    : m_sendCallback(sendCb) {}

bool RoomManager::createRoom(const std::string& roomName) {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    
    if (m_rooms.find(roomName) != m_rooms.end()) {
        return false; 
    }
    
    m_rooms[roomName] = std::make_shared<Room>(roomName, m_sendCallback);
    return true;
}

std::shared_ptr<Room> RoomManager::getRoom(const std::string& roomName) {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    
    auto it = m_rooms.find(roomName);
    if (it != m_rooms.end()) {
        return it->second;
    }
    return nullptr; 
}

bool RoomManager::removeRoom(const std::string& roomName) {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    return m_rooms.erase(roomName) > 0;
}

void RoomManager::checkAllRoomsForTimeouts() {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    
    for (auto& pair : m_rooms) {
        if (pair.second->checkAutoResign()) {
            pair.second->handleAutoResign();
        }
    }
}