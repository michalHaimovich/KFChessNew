#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <optional>
#include "view/board_layout.hpp"
#include "view/asset_manager.hpp"
#include "view/piece_view.hpp"
#include "view/img.hpp"
#include "engine/game_engine.hpp" 
#include "realtime/motion.hpp" 

// ה-includes החדשים למנהלי ה-UI
#include "score_manager.hpp"
#include "move_history_manager.hpp"

class Renderer {
private:
    Img windowBuffer;
    BoardLayout layout;
    
    std::string assetsBasePath;
    
    AssetManager assets; 
    std::map<int, PieceView> pieceViews; 

    // מצביעים למנהלי הנתונים (שנרשמו כ-Observers ב-Engine)
    ScoreManager* scoreManager;
    MoveHistoryManager* historyManager;

    void clearScreen();
    void drawUIBackground();
    void drawBoardBackground();
    
    void drawStationaryPieces(const GameSnapshot& snapshot, long dt);
    void drawActiveMotions(const GameSnapshot& snapshot, long currentTime, long dt);
    
    // פונקציות עזר לציור ה-UI החדש
    void drawSidePanels(const GameSnapshot& snapshot);
    void drawMoveTable(const std::string& title, int x, int y, int w, int h, const std::vector<MoveRecord>& moves);

public:
    // עדכון הבנאי שיקבל גם את המנהלים של ה-UI
    Renderer(int winWidth, int winHeight, 
             int logicalRows, int logicalCols, 
             int marginLeft, int marginTop, int marginRight, int marginBottom,
             const std::string& assetsPath,
             ScoreManager* scoreMgr,
             MoveHistoryManager* historyMgr);

    int getBoardStartX() const { return layout.getBoardStartPos().first; }
    int getBoardStartY() const { return layout.getBoardStartPos().second; }
    int getCellSize() const { return layout.getCellSize(); }
    
    void renderFrame(const GameSnapshot& snapshot, long currentTime, long dt);
};