#include "room.hpp"

#include "room.hpp"


Room::Room(const std::string& name, std::function<void(websocketpp::connection_hdl, const std::string&)> sendCb, UserRepository& repo)
    : m_roomName(name), 
      m_gameSession(std::make_shared<GameSession>(sendCb, repo)), 
      m_isGracePeriodActive(false) {}

std::string Room::getName() const {
    return m_roomName;
}

std::shared_ptr<GameSession> Room::getSession() {
    return m_gameSession;
}

PlayerRole Room::addPlayer(websocketpp::connection_hdl hdl , const std::string& username) {
    return m_gameSession->addClient(hdl, username);
}

void Room::removePlayer(websocketpp::connection_hdl hdl) {
    m_gameSession->removeClient(hdl);
}

void Room::startGracePeriod(websocketpp::connection_hdl hdl, PlayerRole role) {
    m_isGracePeriodActive = true;
    m_disconnectedPlayer = hdl;
    m_disconnectedRole = role;
    m_disconnectTime = std::chrono::steady_clock::now();
}

void Room::handleAutoResign() {
    std::string winnerStr = (m_disconnectedRole == PlayerRole::White) ? "Black" : "White";
    
    nlohmann::json j;
    j["type"] = "GAME_OVER_RESIGN";
    j["winner"] = winnerStr;
    j["message"] = "Opponent disconnected for 20 seconds. " + winnerStr + " wins!";
    
    m_gameSession->broadcastMessage(j.dump());
    cancelGracePeriod(); 
}

void Room::cancelGracePeriod() {
    m_isGracePeriodActive = false;
}

bool Room::checkAutoResign() {
    if (!m_isGracePeriodActive) {
        return false;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_disconnectTime).count();
    
    return duration >= 20; 
}

void Room::handlePlayerDisconnect(websocketpp::connection_hdl hdl) {
    PlayerRole role = m_gameSession->getRole(hdl);
    m_gameSession->removeClient(hdl);
    
    if (role == PlayerRole::White || role == PlayerRole::Black) {
        startGracePeriod(hdl, role);
    }
}

void Room::update(long absoluteTime) {
    if (m_gameSession) {
        m_gameSession->update(absoluteTime);
    }
}

