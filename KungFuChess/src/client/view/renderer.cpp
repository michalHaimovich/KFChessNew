#include "view/renderer.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

// Constants for Rest Bar
const cv::Scalar kRestBarColour{90, 200, 90, 255};
const cv::Scalar kRestBarTrackColour{40, 40, 40, 255};
constexpr int kRestBarHeight = 5;
constexpr int kRestBarInset = 4;

Renderer::Renderer(int winWidth, int winHeight,
                   int logicalRows, int logicalCols,
                   int marginLeft, int marginTop, int marginRight, int marginBottom,
                   const std::string &assetsPath,
                   ScoreManager *scoreMgr,
                   MoveHistoryManager *historyMgr)
    : layout(winWidth, winHeight, logicalRows, logicalCols, marginLeft, marginTop, marginRight, marginBottom),
      assetsBasePath(assetsPath),
      scoreManager(scoreMgr),
      historyManager(historyMgr)
{

    assets.loadAllAssets(assetsBasePath, layout.getCellSize());
}

void Renderer::renderFrame(const GameSnapshot &snapshot, long currentTime, long dt)
{
    clearScreen();
    drawUIBackground();
    drawBoardBackground();

    // Draw highlights BEFORE pieces so they don't cover them
    drawHighlights(snapshot);

    // Passed currentTime so pieces know how to draw their Rest Bars
    drawStationaryPieces(snapshot, currentTime, dt);
    drawActiveMotions(snapshot, currentTime, dt);

    drawSidePanels(snapshot);

    // Draw Game Over overlay if the game ended
    drawGameOverOverlay(snapshot);

    cv::imshow("Kung Fu Chess", windowBuffer.get_mat());
}

void Renderer::clearScreen()
{
    cv::Mat &mat = const_cast<cv::Mat &>(windowBuffer.get_mat());

    if (mat.empty() || mat.cols == 0 || mat.rows == 0)
    {
        mat = cv::Mat(700, 1000, CV_8UC3, cv::Scalar(110, 110, 110));
    }
    else
    {
        mat.setTo(cv::Scalar(110, 110, 110));
    }
}

void Renderer::drawBoardBackground()
{
    cv::Mat &canvas = const_cast<cv::Mat &>(windowBuffer.get_mat());
    if (canvas.empty())
        return;

    auto [startX, startY] = layout.getBoardStartPos();
    int cellSize = layout.getCellSize();

    cv::rectangle(canvas, cv::Point(startX - 2, startY - 2),
                  cv::Point(startX + 8 * cellSize + 2, startY + 8 * cellSize + 2),
                  cv::Scalar(0, 0, 0), 2);

    for (int r = 0; r < 8; ++r)
    {
        for (int c = 0; c < 8; ++c)
        {
            cv::Scalar color = ((r + c) % 2 == 0) ? cv::Scalar(219, 235, 251) : cv::Scalar(124, 163, 206);
            cv::rectangle(canvas, cv::Rect(startX + c * cellSize, startY + r * cellSize, cellSize, cellSize), color, cv::FILLED);
        }
    }

    for (int i = 0; i < 8; ++i)
    {
        std::string colLabel(1, 'a' + i);
        std::string rowLabel(1, '8' - i);

        windowBuffer.put_text(colLabel, startX + i * cellSize + (cellSize / 2) - 5, startY - 15, 0.5, cv::Scalar(40, 40, 40), 1);
        windowBuffer.put_text(colLabel, startX + i * cellSize + (cellSize / 2) - 5, startY + 8 * cellSize + 25, 0.5, cv::Scalar(40, 40, 40), 1);

        windowBuffer.put_text(rowLabel, startX - 25, startY + i * cellSize + (cellSize / 2) + 5, 0.5, cv::Scalar(40, 40, 40), 1);
        windowBuffer.put_text(rowLabel, startX + 8 * cellSize + 15, startY + i * cellSize + (cellSize / 2) + 5, 0.5, cv::Scalar(40, 40, 40), 1);
    }
}

void Renderer::drawStationaryPieces(const GameSnapshot &snapshot, long currentTime, long dt)
{
    for (const Piece &piece : snapshot.stationaryPieces)
    {
        if (pieceViews.find(piece.id) == pieceViews.end())
        {
            pieceViews.emplace(piece.id, PieceView(piece.id));
        }

        pieceViews[piece.id].syncWithModel(piece, assets, dt);

        auto [pixelX, pixelY] = layout.getCellPixelPos(piece.cell.row, piece.cell.col);
        const Img &frame = pieceViews[piece.id].getFrame();

        try
        {
            const_cast<Img &>(frame).draw_on(windowBuffer, pixelX, pixelY);
        }
        catch (...)
        {
            std::cerr << "Warning: Stationary piece out of bounds at " << pixelX << ", " << pixelY << std::endl;
        }

        // Draw the Rest Bar if the piece is cooling down
        if (piece.state == PieceState::ShortRest || piece.state == PieceState::LongRest)
        {
            drawRestBar(piece, currentTime);
        }
    }
}

void Renderer::drawActiveMotions(const GameSnapshot &snapshot, long currentTime, long dt)
{
    int cellSize = layout.getCellSize();

    for (const auto &motion : snapshot.activeMotions)
    {
        if (motion.isDead)
            continue;

        if (pieceViews.find(motion.piece.id) == pieceViews.end())
        {
            pieceViews.emplace(motion.piece.id, PieceView(motion.piece.id));
        }

        pieceViews[motion.piece.id].syncWithModel(motion.piece, assets, dt);

        auto [startX, startY] = layout.getCellPixelPos(motion.source.row, motion.source.col);
        auto [endX, endY] = layout.getCellPixelPos(motion.destination.row, motion.destination.col);

        double progress = 0.0;
        if (motion.arrivalTime > motion.startTime)
        {
            progress = static_cast<double>(currentTime - motion.startTime) / (motion.arrivalTime - motion.startTime);
            progress = std::max(0.0, std::min(1.0, progress));
        }

        int currentX = startX + static_cast<int>((endX - startX) * progress);
        int currentY = startY + static_cast<int>((endY - startY) * progress);

        if (motion.type == MotionType::Jump)
        {
            double arc = std::sin(progress * 3.14159265358979323846);
            int jumpHeightPixels = static_cast<int>(cellSize * 1.5);
            currentY -= static_cast<int>(arc * jumpHeightPixels);
        }

        const Img &frame = pieceViews[motion.piece.id].getFrame();

        try
        {
            const_cast<Img &>(frame).draw_on(windowBuffer, currentX, currentY);
        }
        catch (...)
        {
        }
    }
}

void Renderer::drawUIBackground()
{
}

void Renderer::drawSidePanels(const GameSnapshot &snapshot)
{
    cv::Mat &canvas = const_cast<cv::Mat &>(windowBuffer.get_mat());
    if (canvas.empty())
        return;

    auto [boardX, boardY] = layout.getBoardStartPos();
    int cellSize = layout.getCellSize();
    int boardWidth = cellSize * 8;

    std::string blackScoreStr = snapshot.blackPlayerName + " (Black): " + std::to_string(scoreManager->getBlackScore());
    std::string whiteScoreStr = snapshot.whitePlayerName + " (White): " + std::to_string(scoreManager->getWhiteScore());

    windowBuffer.put_text(blackScoreStr, boardX + (boardWidth / 2) - 40, boardY - 45, 0.6, cv::Scalar(20, 20, 20), 2);

    windowBuffer.put_text(whiteScoreStr, boardX + (boardWidth / 2) - 40, boardY + boardWidth + 45, 0.6, cv::Scalar(20, 20, 20), 2);

    int tableW = 220;
    int tableH = 380;
    int tableY = boardY + 20;

    int leftTableX = boardX - tableW - 40;
    int rightTableX = boardX + boardWidth + 40;

    drawMoveTable("Black Actions", leftTableX, tableY, tableW, tableH, historyManager->getBlackMoves());
    drawMoveTable("White Actions", rightTableX, tableY, tableW, tableH, historyManager->getWhiteMoves());
}

void Renderer::drawMoveTable(const std::string &title, int x, int y, int w, int h, const std::vector<MoveRecord> &moves)
{
    cv::Mat &canvas = const_cast<cv::Mat &>(windowBuffer.get_mat());

    cv::rectangle(canvas, cv::Rect(x, y, w, h), cv::Scalar(255, 255, 255), cv::FILLED);
    cv::rectangle(canvas, cv::Rect(x, y, w, h), cv::Scalar(0, 0, 0), 2);

    windowBuffer.put_text(title, x + 10, y - 10, 0.55, cv::Scalar(20, 20, 20), 2);

    int headerH = 30;
    cv::line(canvas, cv::Point(x, y + headerH), cv::Point(x + w, y + headerH), cv::Scalar(0, 0, 0), 1);
    cv::line(canvas, cv::Point(x + 100, y), cv::Point(x + 100, y + h), cv::Scalar(0, 0, 0), 1);
    windowBuffer.put_text("Time", x + 15, y + 20, 0.45, cv::Scalar(100, 100, 100), 1);
    windowBuffer.put_text("Move", x + 115, y + 20, 0.45, cv::Scalar(100, 100, 100), 1);

    int maxRows = 14;
    int startIdx = 0;
    if (moves.size() > static_cast<size_t>(maxRows))
    {
        startIdx = moves.size() - maxRows;
    }
    int rowY = y + headerH + 22;
    for (size_t i = startIdx; i < moves.size(); ++i)
    {
        const auto &record = moves[i];

        windowBuffer.put_text(record.timeStr, x + 8, rowY, 0.4, cv::Scalar(40, 40, 40), 1);
        windowBuffer.put_text(record.moveStr, x + 115, rowY, 0.4, cv::Scalar(10, 10, 200), 1);

        cv::line(canvas, cv::Point(x, rowY + 5), cv::Point(x + w, rowY + 5), cv::Scalar(230, 230, 230), 1);

        rowY += 24;
    }
}

// ==========================================
// NEW UI ELEMENTS
// ==========================================

void Renderer::drawHighlights(const GameSnapshot &snapshot)
{
    cv::Mat &canvas = const_cast<cv::Mat &>(windowBuffer.get_mat());
    if (canvas.empty())
        return;

    int cellSize = layout.getCellSize();
    cv::Mat overlay;
    canvas.copyTo(overlay);
    bool shouldBlend = false;

    // 1. Highlight the currently selected piece in Soft Blue
    if (snapshot.selectedPiece.has_value())
    {
        auto [pixelX, pixelY] = layout.getCellPixelPos(snapshot.selectedPiece.value().cell.row, snapshot.selectedPiece.value().cell.col);
        cv::rectangle(overlay, cv::Rect(pixelX, pixelY, cellSize, cellSize),
                      cv::Scalar(255, 200, 100), cv::FILLED); // BGR format for Blueish
        shouldBlend = true;
    }

    // 2. Highlight all legal destinations
    for (const Position &pos : snapshot.highlightedCells)
    {
        auto [pixelX, pixelY] = layout.getCellPixelPos(pos.row, pos.col);

        // Differentiate color if an enemy piece is standing there
        auto targetPiece = snapshot.getPieceAt(pos);
        if (targetPiece.has_value())
        {
            // Enemy piece - Red Highlight
            cv::rectangle(overlay, cv::Rect(pixelX, pixelY, cellSize, cellSize),
                          cv::Scalar(100, 100, 255), cv::FILLED);
        }
        else
        {
            // Empty Cell - Green Highlight
            cv::rectangle(overlay, cv::Rect(pixelX, pixelY, cellSize, cellSize),
                          cv::Scalar(150, 220, 150), cv::FILLED);
        }
        shouldBlend = true;
    }

    // Apply the transparent overlay
    if (shouldBlend)
    {
        cv::addWeighted(overlay, 0.4, canvas, 0.6, 0, canvas);
    }
}

void Renderer::drawRestBar(const Piece &piece, long currentTime)
{
    cv::Mat &pixels = const_cast<cv::Mat &>(windowBuffer.get_mat());
    auto [pixelX, pixelY] = layout.getCellPixelPos(piece.cell.row, piece.cell.col);

    int width = layout.getCellSize() - 2 * kRestBarInset;
    int x = pixelX + kRestBarInset;
    int y = pixelY + layout.getCellSize() - kRestBarInset - kRestBarHeight;

    cv::rectangle(pixels, cv::Rect(x, y, width, kRestBarHeight),
                  kRestBarTrackColour, cv::FILLED);

    double progress = 1.0;
    if (piece.readyTime > currentTime)
    {
        long remaining = piece.readyTime - currentTime;
        // Default max values if config isn't explicitly passed, adapt if needed:
        long totalCooldown = (piece.state == PieceState::LongRest) ? 2000 : 1000;
        progress = 1.0 - (static_cast<double>(remaining) / static_cast<double>(totalCooldown));
    }
    progress = std::max(0.0, std::min(1.0, progress));

    int left = static_cast<int>(width * progress);
    if (left > 0)
    {
        cv::rectangle(pixels, cv::Rect(x, y, left, kRestBarHeight),
                      kRestBarColour, cv::FILLED);
    }
}

void Renderer::drawGameOverOverlay(const GameSnapshot &snapshot)
{
    if (!snapshot.isGameOver)
        return;

    cv::Mat &canvas = const_cast<cv::Mat &>(windowBuffer.get_mat());
    if (canvas.empty())
        return;

    auto [boardX, boardY] = layout.getBoardStartPos();
    int boardW = layout.getCellSize() * 8;
    int boardH = layout.getCellSize() * 8;

    cv::Mat overlay;
    canvas.copyTo(overlay);
    cv::rectangle(overlay, cv::Rect(boardX, boardY, boardW, boardH), cv::Scalar(0, 0, 0), cv::FILLED);
    cv::addWeighted(overlay, 0.6, canvas, 0.4, 0, canvas);

    std::string text = "GAME OVER";
    cv::Scalar textColor(150, 150, 150);

    if (snapshot.winner.has_value())
    {
        if (snapshot.winner.value() == PieceColor::White)
        {
            text = "WHITE WINS!";
            textColor = cv::Scalar(255, 255, 255);
        }
        else
        {
            text = "BLACK WINS!";
            textColor = cv::Scalar(20, 20, 20);
        }
    }

    int fontFace = cv::FONT_HERSHEY_DUPLEX;
    double fontScale = 1.3;
    int thickness = 2;
    int baseline = 0;

    cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
    int textX = boardX + (boardW - textSize.width) / 2;
    int textY = boardY + (boardH + textSize.height) / 2;

    cv::Scalar outlineColor = (textColor[0] > 128) ? cv::Scalar(0, 0, 0) : cv::Scalar(255, 255, 255);

    cv::putText(canvas, text, cv::Point(textX, textY), fontFace, fontScale, outlineColor, thickness + 3, cv::LINE_AA);
    cv::putText(canvas, text, cv::Point(textX, textY), fontFace, fontScale, textColor, thickness, cv::LINE_AA);
}

void Renderer::updateWindowSize(int w, int h)
{
    layout.updateWindowSize(w, h);
    const_cast<cv::Mat &>(windowBuffer.get_mat()) = cv::Mat(h, w, CV_8UC3, cv::Scalar(110, 110, 110));
}

int Renderer::getWindowWidth() const { return windowBuffer.get_mat().cols; }
int Renderer::getWindowHeight() const { return windowBuffer.get_mat().rows; }