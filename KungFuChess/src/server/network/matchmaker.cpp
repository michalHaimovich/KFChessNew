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
    
    m_timeoutCache.clear();
    m_waitingCache.clear();

    for (auto it = m_queue.begin(); it != m_queue.end(); ++it) {
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.entryTime).count();
        if (duration >= m_timeoutSeconds) {
            m_timeoutCache.push_back(it->first);
        } else {
            m_waitingCache.push_back(it->second);
        }
    }

    for (auto hdl : m_timeoutCache) {
        removePlayer(hdl);
        if (m_onTimeout) m_onTimeout(hdl);
    }

    if (m_waitingCache.size() < 2) return;

    std::sort(m_waitingCache.begin(), m_waitingCache.end(), [](const QueuedPlayer& a, const QueuedPlayer& b) {
        return a.rating < b.rating;
    });

    for (size_t i = 0; i + 1 < m_waitingCache.size(); ++i) {
        
        int eloDiff = m_waitingCache[i+1].rating - m_waitingCache[i].rating;
        
        if (eloDiff <= m_maxEloDifference) {
            auto hdl1 = m_waitingCache[i].hdl;
            auto hdl2 = m_waitingCache[i+1].hdl;

            removePlayer(hdl1);
            removePlayer(hdl2);

            if (m_onMatchFound) m_onMatchFound(hdl1, hdl2);

            ++i; 
        }
    }
}