#include "room_manager.hpp"
#include "room_manager.hpp"
#include <iostream>

RoomManager::RoomManager(std::function<void(websocketpp::connection_hdl, const std::string&)> sendCb, UserRepository& repo)
    : m_sendCallback(sendCb), m_userRepo(repo), m_stopPool(false) 
{
    unsigned int numCores = std::thread::hardware_concurrency();
    if (numCores == 0) numCores = 4; 
    
    std::cout << "[RoomManager] Initializing Thread Pool with " << numCores << " workers." << std::endl;

    for (unsigned int i = 0; i < numCores; ++i) {
        m_workers.emplace_back(&RoomManager::workerLoop, this);
    }

    m_tickTimerThread = std::thread(&RoomManager::tickTimerLoop, this);
}

RoomManager::~RoomManager() {
    m_stopPool = true;
    m_condition.notify_all(); 

    for (std::thread& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    if (m_tickTimerThread.joinable()) {
        m_tickTimerThread.join();
    }
}

void RoomManager::workerLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_condition.wait(lock, [this] { return m_stopPool || !m_tasks.empty(); });

            if (m_stopPool && m_tasks.empty()) {
                return;
            }

            task = std::move(m_tasks.front());
            m_tasks.pop();
        }
        
        task();
    }
}

void RoomManager::tickTimerLoop() {
    using clock = std::chrono::high_resolution_clock;
    auto startTime = clock::now();
    constexpr long TICK_RATE_MS = 1000 / 30; 

    while (!m_stopPool) {
        auto now = clock::now();
        long absoluteTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

        std::vector<std::shared_ptr<Room>> currentRooms;
        {
            std::lock_guard<std::mutex> lock(m_managerMutex);
            for (auto& pair : m_rooms) {
                currentRooms.push_back(pair.second);
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            for (auto& room : currentRooms) {
                m_tasks.push([room, absoluteTime]() {
                    room->update(absoluteTime);
                });
            }
        }
        m_condition.notify_all();
        auto loopEndTime = clock::now();
        long loopDuration = std::chrono::duration_cast<std::chrono::milliseconds>(loopEndTime - now).count();
        long sleepTime = TICK_RATE_MS - loopDuration;

        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        } else {
            std::this_thread::yield();
        }
    }
}

bool RoomManager::createRoom(const std::string& roomName) {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    
    if (m_rooms.find(roomName) != m_rooms.end()) {
        return false; 
    }
    
    m_rooms[roomName] = std::make_shared<Room>(roomName, m_sendCallback, m_userRepo);
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