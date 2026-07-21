#include "game_session.hpp"
#include <iostream>

constexpr long TARGET_FPS = 30;
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

            snap.whitePlayerName = m_whiteName;
            snap.blackPlayerName = m_blackName;

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

        auto loopEndTime = clock::now();
        long loopDuration = std::chrono::duration_cast<std::chrono::milliseconds>(loopEndTime - now).count();
        long sleepTime = BROADCAST_INTERVAL_MS - loopDuration;

        if (sleepTime > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        }
        else
        {
            std::this_thread::yield();
        }
    }

    std::cout << "[GameSession] Time thread stopped." << std::endl;
}

GameSession::GameSession(std::function<void(websocketpp::connection_hdl, const std::string &)> sendCb)
    : engine(8, 8), whiteTaken(false), blackTaken(false), isRunning(true), sendCallback(sendCb)
{
    engine.setEventBus(&serverBus);

    serverBus.subscribe<PieceCapturedEvent>([this](const PieceCapturedEvent &e)
                                            {
        json j;
        j["type"] = "PieceCapturedEvent";
        j["pieceId"] = e.capturedPiece.id;
        j["pieceColor"] = (e.capturedPiece.color == PieceColor::White) ? "White" : "Black";
        j["pieceKind"] = static_cast<int>(e.capturedPiece.kind);
        
        std::lock_guard<std::mutex> lock(eventsMutex);
        pendingEvents.push_back(j); });

    serverBus.subscribe<MoveCompletedEvent>([this](const MoveCompletedEvent &e)
                                            {
        json j;
        j["type"] = "MoveCompletedEvent";
        j["pieceId"] = e.piece.id;
        j["pieceColor"] = (e.piece.color == PieceColor::White) ? "White" : "Black";
        j["pieceKind"] = static_cast<int>(e.piece.kind);
        j["sourceRow"] = e.source.row;
        j["sourceCol"] = e.source.col;
        j["destRow"] = e.dest.row;
        j["destCol"] = e.dest.col;
        j["destinationCapture"] = e.destinationCapture;
        j["timeMs"] = e.timeMs;

        std::lock_guard<std::mutex> lock(eventsMutex);
        pendingEvents.push_back(j); });

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

PlayerRole GameSession::addClient(websocketpp::connection_hdl hdl, const std::string &username)
{
    std::lock_guard<std::mutex> lock(playersMutex);
    PlayerRole assignedRole;

    if (!whiteTaken)
    {
        assignedRole = PlayerRole::White;
        whiteTaken = true;
        m_whiteName = username;
        std::cout << "[GameSession] Client joined - Assigned as White Player: " << username << std::endl;
    }
    else if (!blackTaken)
    {
        assignedRole = PlayerRole::Black;
        blackTaken = true;
        m_blackName = username;
        std::cout << "[GameSession] Client joined - Assigned as Black Player: " << username << std::endl;
    }
    else
    {
        assignedRole = PlayerRole::Spectator;
        std::cout << "[GameSession] Client joined - Assigned as Spectator: " << username << std::endl;
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
            m_whiteName = "Waiting...";
            std::cout << "[GameSession] White Player disconnected. Slot is open." << std::endl;
        }
        else if (it->second == PlayerRole::Black)
        {
            blackTaken = false;
            m_blackName = "Waiting...";
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
    j["whitePlayerName"] = snap.whitePlayerName;
    j["blackPlayerName"] = snap.blackPlayerName;

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

    j["events"] = json::array();
    {
        std::lock_guard<std::mutex> lock(eventsMutex);
        for (const auto &eJson : pendingEvents)
        {
            j["events"].push_back(eJson);
        }
        pendingEvents.clear();
    }

    return j.dump();
}

void GameSession::broadcastMessage(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(playersMutex);
    for (const auto &pair : players)
    {
        if (sendCallback)
        {
            sendCallback(pair.first, msg);
        }
    }
}
