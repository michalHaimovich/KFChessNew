#pragma once
#include <map>
#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <cmath>
#include <algorithm>

#include <websocketpp/common/connection_hdl.hpp>

struct QueuedPlayer
{
    websocketpp::connection_hdl hdl;
    std::string username;
    int rating;
    std::chrono::steady_clock::time_point entryTime;
};

class Matchmaker
{
private:
    std::map<websocketpp::connection_hdl, QueuedPlayer, std::owner_less<websocketpp::connection_hdl>> m_queue;
    int m_maxEloDifference;
    int m_timeoutSeconds;

    std::vector<websocketpp::connection_hdl> m_timeoutCache;
    std::vector<QueuedPlayer> m_waitingCache;

    // Callbacks to notify the NetworkServer
    std::function<void(websocketpp::connection_hdl, websocketpp::connection_hdl)> m_onMatchFound;
    std::function<void(websocketpp::connection_hdl)> m_onTimeout;

public:
    // Constructor requiring the callbacks
    Matchmaker(int maxEloDiff, int timeoutSec,
               std::function<void(websocketpp::connection_hdl, websocketpp::connection_hdl)> onMatchFound,
               std::function<void(websocketpp::connection_hdl)> onTimeout);

    // Adds a player to the matchmaking queue
    void addPlayer(websocketpp::connection_hdl hdl, const std::string &username, int rating);

    // Removes a player from the queue (e.g., if they disconnect while waiting)
    void removePlayer(websocketpp::connection_hdl hdl);

    // Called every second by the server timer to process the queue
    void processQueue();
};