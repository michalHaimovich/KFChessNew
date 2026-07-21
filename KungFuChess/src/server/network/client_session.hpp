#pragma once
#include <string>
#include <chrono>

enum class ClientState {
    CONNECTED,   
    LOBBY,       
    IN_QUEUE,     
    IN_ROOM       
};

struct ClientSession {
    ClientState state = ClientState::CONNECTED; 
    std::string username = "";
    int rating = 1200;      
    std::string roomId = ""; 
    std::chrono::steady_clock::time_point queueStartTime;
};