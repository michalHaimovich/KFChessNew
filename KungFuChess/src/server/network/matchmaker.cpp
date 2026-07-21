#include "matchmaker.hpp"

Matchmaker::Matchmaker(int maxEloDiff, int timeoutSec,
                       std::function<void(websocketpp::connection_hdl, websocketpp::connection_hdl)> onMatchFound,
                       std::function<void(websocketpp::connection_hdl)> onTimeout)
    : m_maxEloDifference(maxEloDiff), 
      m_timeoutSeconds(timeoutSec),
      m_onMatchFound(onMatchFound), 
      m_onTimeout(onTimeout) {}

void Matchmaker::addPlayer(websocketpp::connection_hdl hdl, const std::string& username, int rating) {
    QueuedPlayer p;
    p.hdl = hdl;
    p.username = username;
    p.rating = rating;
    p.entryTime = std::chrono::steady_clock::now();
    
    m_queue[hdl] = p;
}

void Matchmaker::removePlayer(websocketpp::connection_hdl hdl) {
    m_queue.erase(hdl);
}

void Matchmaker::processQueue() {
    auto now = std::chrono::steady_clock::now();
    std::vector<websocketpp::connection_hdl> matchedPlayers;
    std::vector<websocketpp::connection_hdl> timeoutPlayers;

    // 1. Check for timeouts
    for (const auto& pair : m_queue) {
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - pair.second.entryTime).count();
        if (duration >= m_timeoutSeconds) {
            timeoutPlayers.push_back(pair.first);
        }
    }

    // Process timeouts (remove from queue and notify)
    for (auto hdl : timeoutPlayers) {
        removePlayer(hdl);
        if (m_onTimeout) m_onTimeout(hdl);
    }

    // 2. Find matches
    // Create a vector of remaining players for easier iteration
    std::vector<QueuedPlayer> waitingPlayers;
    for (const auto& pair : m_queue) {
        waitingPlayers.push_back(pair.second);
    }

    // Find valid pairs
    for (size_t i = 0; i < waitingPlayers.size(); ++i) {
        // Skip if this player was already matched in this loop
        if (m_queue.find(waitingPlayers[i].hdl) == m_queue.end()) continue;

        for (size_t j = i + 1; j < waitingPlayers.size(); ++j) {
            // Skip if the second player was already matched
            if (m_queue.find(waitingPlayers[j].hdl) == m_queue.end()) continue;

            int eloDiff = std::abs(waitingPlayers[i].rating - waitingPlayers[j].rating);
            if (eloDiff <= m_maxEloDifference) {
                // Match found! 
                auto hdl1 = waitingPlayers[i].hdl;
                auto hdl2 = waitingPlayers[j].hdl;

                // Remove both from queue
                removePlayer(hdl1);
                removePlayer(hdl2);

                // Notify server to create the room
                if (m_onMatchFound) m_onMatchFound(hdl1, hdl2);
                
                break; // Player 'i' is matched, move to the next 'i'
            }
        }
    }
}