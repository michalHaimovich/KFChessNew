#include "game_session.hpp"
#include <iostream>

constexpr long TARGET_FPS = 60;
constexpr long BROADCAST_INTERVAL_MS = 1000 / TARGET_FPS;


void GameSession::gameLoop()
{
    using clock = std::chrono::high_resolution_clock;
    auto startTime = clock::now(); 
    
    long lastEngineUpdateTime = 0;
    long lastBroadcastTime = 0;

    std::cout << "[GameSession] Time thread started." << std::endl;

    while (isRunning)
    {
        auto now = clock::now();
        long absoluteTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        
        long engineDt = absoluteTime - lastEngineUpdateTime;
        if (engineDt > 0)
        {
            engine.wait(engineDt);
            lastEngineUpdateTime = absoluteTime;
        }

        long timeSinceLastBroadcast = absoluteTime - lastBroadcastTime;
        if (timeSinceLastBroadcast >= BROADCAST_INTERVAL_MS)
        {
            GameSnapshot snap = engine.getSnapshot();
            std::string stateStr = serializeSnapshot(snap);
            {
                std::lock_guard<std::mutex> lock(playersMutex);
                for (const auto &pair : players)
                {
                    if (sendCallback)
                    {
                        sendCallback(pair.first, stateStr); 
                    }
                }
            }
            lastBroadcastTime = absoluteTime; 
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1)); 
    }

    std::cout << "[GameSession] Time thread stopped." << std::endl;
}

// Initialize session and store the send callback
GameSession::GameSession(std::function<void(websocketpp::connection_hdl, const std::string &)> sendCb)
    : engine(8, 8), whiteTaken(false), blackTaken(false), isRunning(true), sendCallback(sendCb)
{

    engine.setupStandardBoard();
    timeThread = std::thread(&GameSession::gameLoop, this);
}

GameSession::~GameSession()
{
    isRunning = false;
    if (timeThread.joinable())
    {
        timeThread.join();
    }
}

PlayerRole GameSession::addClient(websocketpp::connection_hdl hdl)
{
    std::lock_guard<std::mutex> lock(playersMutex);

    PlayerRole assignedRole;

    if (!whiteTaken)
    {
        assignedRole = PlayerRole::White;
        whiteTaken = true;
        std::cout << "[GameSession] Client joined - Assigned as White Player." << std::endl;
    }
    else if (!blackTaken)
    {
        assignedRole = PlayerRole::Black;
        blackTaken = true;
        std::cout << "[GameSession] Client joined - Assigned as Black Player." << std::endl;
    }
    else
    {
        assignedRole = PlayerRole::Spectator;
        std::cout << "[GameSession] Client joined - Assigned as Spectator." << std::endl;
    }

    players[hdl] = assignedRole;
    return assignedRole;
}

void GameSession::removeClient(websocketpp::connection_hdl hdl)
{
    std::lock_guard<std::mutex> lock(playersMutex);

    auto it = players.find(hdl);
    if (it != players.end())
    {
        if (it->second == PlayerRole::White)
        {
            whiteTaken = false;
            std::cout << "[GameSession] White Player disconnected. Slot is open." << std::endl;
        }
        else if (it->second == PlayerRole::Black)
        {
            blackTaken = false;
            std::cout << "[GameSession] Black Player disconnected. Slot is open." << std::endl;
        }
        else
        {
            std::cout << "[GameSession] Spectator disconnected." << std::endl;
        }
        players.erase(it);
    }
}

PlayerRole GameSession::getRole(websocketpp::connection_hdl hdl) const
{
    std::lock_guard<std::mutex> lock(playersMutex);

    auto it = players.find(hdl);
    if (it != players.end())
    {
        return it->second;
    }
    return PlayerRole::Spectator;
}

GameEngine &GameSession::getEngine()
{
    return engine;
}

std::string GameSession::serializeSnapshot(const GameSnapshot &snap)
{
    json j;
    j["serverTime"] = snap.serverTime;
    j["boardWidth"] = snap.boardWidth;
    j["boardHeight"] = snap.boardHeight;
    j["isGameOver"] = snap.isGameOver;

    if (snap.isGameOver && snap.winner.has_value())
    {
        j["winner"] = (snap.winner.value() == PieceColor::White) ? "White" : "Black";
    }

    j["stationaryPieces"] = json::array();
    for (const auto &piece : snap.stationaryPieces)
    {
        json pJson;
        pJson["id"] = piece.id;
        pJson["color"] = (piece.color == PieceColor::White) ? "White" : "Black";
        pJson["kind"] = static_cast<int>(piece.kind);
        pJson["row"] = piece.cell.row;
        pJson["col"] = piece.cell.col;
        pJson["state"] = static_cast<int>(piece.state);
        pJson["readyTime"] = piece.readyTime;

        j["stationaryPieces"].push_back(pJson);
    }

    j["activeMotions"] = json::array();
    for (const auto &motion : snap.activeMotions)
    {
        json mJson;
        mJson["pieceId"] = motion.piece.id;
        mJson["pieceColor"] = (motion.piece.color == PieceColor::White) ? "White" : "Black";
        mJson["pieceKind"] = static_cast<int>(motion.piece.kind);
        mJson["startTime"] = motion.startTime;
        mJson["arrivalTime"] = motion.arrivalTime;
        mJson["type"] = static_cast<int>(motion.type);
        mJson["sourceRow"] = motion.source.row;
        mJson["sourceCol"] = motion.source.col;
        mJson["destRow"] = motion.destination.row;
        mJson["destCol"] = motion.destination.col;

        j["activeMotions"].push_back(mJson);
    }
    return j.dump();
}