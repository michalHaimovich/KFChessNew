#pragma once
#include <string>

struct LoginResult {
    std::string username;
    std::string password;
    bool success = false;
};

enum class HomeAction { NONE, PLAY_RANDOM, ENTER_ROOM };

struct HomeResult {
    HomeAction action = HomeAction::NONE;
};

struct RoomResult {
    std::string roomName;
    std::string action; // "CREATE_ROOM" or "JOIN_ROOM"
    bool success = false;
};