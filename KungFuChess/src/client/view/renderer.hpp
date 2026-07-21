#pragma once

#include <map>
#include <string>
#include "view/board_layout.hpp"
#include "view/asset_manager.hpp"
#include "view/piece_view.hpp"
#include "view/img.hpp"
#include "model/game_snapshot.hpp"
#include "score_manager.hpp"
#include "move_history_manager.hpp"

class Renderer {
private:
    Img windowBuffer;
    BoardLayout layout;
    std::string assetsBasePath;
    AssetManager assets;
    std::map<int, PieceView> pieceViews;
    ScoreManager *scoreManager;
    MoveHistoryManager *historyManager;

    void clearScreen();
    void drawStationaryPieces(const GameSnapshot &snapshot, long currentTime, long dt);
    void drawActiveMotions(const GameSnapshot &snapshot, long currentTime, long dt);

public:
    Renderer(int winWidth, int winHeight, int logicalRows, int logicalCols,
             int marginLeft, int marginTop, int marginRight, int marginBottom,
             const std::string &assetsPath, ScoreManager *scoreMgr, MoveHistoryManager *historyMgr);

    int getBoardStartX() const { return layout.getBoardStartPos().first; }
    int getBoardStartY() const { return layout.getBoardStartPos().second; }
    int getCellSize() const { return layout.getCellSize(); }

    void renderFrame(const GameSnapshot &snapshot, long currentTime, long dt);
    void updateWindowSize(int w, int h);
    int getWindowWidth() const;
    int getWindowHeight() const;
};