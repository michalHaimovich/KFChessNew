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
#include "model/motion.hpp"

#include "score_manager.hpp"
#include "move_history_manager.hpp"

class Renderer
{
private:
    Img windowBuffer;
    BoardLayout layout;

    std::string assetsBasePath;

    AssetManager assets;
    std::map<int, PieceView> pieceViews;

    ScoreManager *scoreManager;
    MoveHistoryManager *historyManager;

    void clearScreen();
    void drawUIBackground();
    void drawBoardBackground();

    // Updated to receive currentTime for the Rest Bar calculation
    void drawStationaryPieces(const GameSnapshot &snapshot, long currentTime, long dt);
    void drawActiveMotions(const GameSnapshot &snapshot, long currentTime, long dt);

    void drawSidePanels(const GameSnapshot &snapshot);
    void drawMoveTable(const std::string &title, int x, int y, int w, int h, const std::vector<MoveRecord> &moves);

    // New UI Elements
    void drawHighlights(const GameSnapshot &snapshot);
    void drawRestBar(const Piece &piece, long currentTime);
    void drawGameOverOverlay(const GameSnapshot &snapshot);

public:
    Renderer(int winWidth, int winHeight,
             int logicalRows, int logicalCols,
             int marginLeft, int marginTop, int marginRight, int marginBottom,
             const std::string &assetsPath,
             ScoreManager *scoreMgr,
             MoveHistoryManager *historyMgr);

    int getBoardStartX() const { return layout.getBoardStartPos().first; }
    int getBoardStartY() const { return layout.getBoardStartPos().second; }
    int getCellSize() const { return layout.getCellSize(); }

    void renderFrame(const GameSnapshot &snapshot, long currentTime, long dt);

    void updateWindowSize(int w, int h);
    int getWindowWidth() const;
    int getWindowHeight() const;
};