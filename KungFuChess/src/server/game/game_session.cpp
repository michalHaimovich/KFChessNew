#include "game_session.hpp"
#include <cmath>
#include <iostream>

namespace
{
    constexpr long TARGET_FPS = 30;
    constexpr long BROADCAST_INTERVAL_MS = 1000 / TARGET_FPS;
    int kFactor = 32;
}

void GameSession::update(long absoluteTime)
{
    // 1. קידום מנוע המשחק
    long engineDt = absoluteTime - lastEngineUpdateTime;
    if (engineDt > 0)
    {
        engine.wait(engineDt);
        lastEngineUpdateTime = absoluteTime;
    }

    // 2. שידור המצב ללקוחות 
    long timeSinceLastBroadcast = absoluteTime - lastBroadcastTime;
    if (timeSinceLastBroadcast >= BROADCAST_INTERVAL_MS)
    {
        GameSnapshot snap = engine.getSnapshot();

        if (snap.isGameOver && !eloUpdated)
        {
            if (snap.winner.has_value())
            {
                processGameOver(snap);
                eloUpdated = true;
            }
        }

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
}   

void GameSession::processGameOver(const GameSnapshot& snap)
{
    std::cout << "[DEBUG ELO] Starting ELO process for White: '" << m_whiteName << "' vs Black: '" << m_blackName << "'" << std::endl;

    auto whiteUser = userRepo.getByUsername(m_whiteName);
    auto blackUser = userRepo.getByUsername(m_blackName);

    if (!whiteUser) {
        std::cerr << "[DEBUG ELO] ERROR: Could not find White user '" << m_whiteName << "' in DB!" << std::endl;
        return; 
    }
    if (!blackUser) {
        std::cerr << "[DEBUG ELO] ERROR: Could not find Black user '" << m_blackName << "' in DB!" << std::endl;
        return; 
    }

    int currentWhiteElo = whiteUser->rating;
    int currentBlackElo = blackUser->rating;
    std::cout << "[DEBUG ELO] Fetched Current Ratings -> White: " << currentWhiteElo << ", Black: " << currentBlackElo << std::endl;

    float expectedWhite = 1.0f / (1.0f + std::pow(10.0f, (currentBlackElo - currentWhiteElo) / 400.0f));
    float expectedBlack = 1.0f - expectedWhite;

    float scoreWhite = (snap.winner.value() == PieceColor::White) ? 1.0f : 0.0f;
    float scoreBlack = 1.0f - scoreWhite;
    
    std::cout << "[DEBUG ELO] Winner is: " << ((scoreWhite == 1.0f) ? "White" : "Black") << std::endl;

    int kFactor = 32;
    int newWhiteElo = currentWhiteElo + static_cast<int>(kFactor * (scoreWhite - expectedWhite));
    int newBlackElo = currentBlackElo + static_cast<int>(kFactor * (scoreBlack - expectedBlack));

    std::cout << "[DEBUG ELO] Calculated New Ratings -> White: " << newWhiteElo << ", Black: " << newBlackElo << std::endl;

    // ניסיון לעדכן את המסד ושמירת התוצאה (האם הצליח או נכשל)
    bool whiteSuccess = userRepo.updateRating(m_whiteName, newWhiteElo);
    bool blackSuccess = userRepo.updateRating(m_blackName, newBlackElo);

    if (!whiteSuccess) {
        std::cerr << "[DEBUG ELO] DATABASE ERROR: Failed to update rating for White user!" << std::endl;
    }
    if (!blackSuccess) {
        std::cerr << "[DEBUG ELO] DATABASE ERROR: Failed to update rating for Black user!" << std::endl;
    }
    
    if (whiteSuccess && blackSuccess) {
        std::cout << "[DEBUG ELO] SUCCESS: Database completely updated!" << std::endl;
    }
}

GameSession::GameSession(std::function<void(websocketpp::connection_hdl, const std::string &)> sendCb, UserRepository &repo)
    : engine(8, 8), whiteTaken(false), blackTaken(false), sendCallback(sendCb), userRepo(repo)
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
}

GameSession::~GameSession()
{
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
