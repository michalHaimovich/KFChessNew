#include "view/renderer.hpp"
#include "view/theme.hpp"
#include "view/board_renderer.hpp"
#include "view/ui_renderer.hpp"
#include <cmath>
#include <iostream>

Renderer::Renderer(int winWidth, int winHeight, int logicalRows, int logicalCols,
                   int marginLeft, int marginTop, int marginRight, int marginBottom,
                   const std::string &assetsPath, ScoreManager *scoreMgr, MoveHistoryManager *historyMgr)
    : layout(winWidth, winHeight, logicalRows, logicalCols, marginLeft, marginTop, marginRight, marginBottom),
      assetsBasePath(assetsPath), scoreManager(scoreMgr), historyManager(historyMgr)
{
    assets.loadAllAssets(assetsBasePath, layout.getCellSize());
}

void Renderer::renderFrame(const GameSnapshot &snapshot, long currentTime, long dt)
{
    clearScreen();

    BoardRenderer::drawBackground(windowBuffer, layout);
    BoardRenderer::drawHighlights(windowBuffer, layout, snapshot);

    drawStationaryPieces(snapshot, currentTime, dt);
    drawActiveMotions(snapshot, currentTime, dt);

    UIRenderer::drawSidePanels(windowBuffer, layout, snapshot, scoreManager, historyManager);
    UIRenderer::drawGameOverOverlay(windowBuffer, layout, snapshot);

    cv::imshow("Kung Fu Chess", windowBuffer.get_mat());
}

void Renderer::clearScreen()
{
    cv::Mat &mat = const_cast<cv::Mat &>(windowBuffer.get_mat());
    if (mat.empty() || mat.cols == 0 || mat.rows == 0) {
        mat = cv::Mat(700, 1000, CV_8UC3, Theme::Background);
    } else {
        mat.setTo(Theme::Background);
    }
}

void Renderer::drawStationaryPieces(const GameSnapshot &snapshot, long currentTime, long dt)
{
    for (const Piece &piece : snapshot.stationaryPieces) {
        if (pieceViews.find(piece.id) == pieceViews.end()) {
            pieceViews.emplace(piece.id, PieceView(piece.id));
        }

        pieceViews[piece.id].syncWithModel(piece, assets, dt);

        auto [pixelX, pixelY] = layout.getCellPixelPos(piece.cell.row, piece.cell.col);
        const Img &frame = pieceViews[piece.id].getFrame();

        try {
            const_cast<Img &>(frame).draw_resized_on(windowBuffer, pixelX, pixelY, layout.getCellSize(), layout.getCellSize());
        } catch (...) {
            std::cerr << "Warning: Stationary piece out of bounds at " << pixelX << ", " << pixelY << std::endl;
        }

        if (piece.state == PieceState::ShortRest || piece.state == PieceState::LongRest) {
            UIRenderer::drawRestBar(windowBuffer, layout, piece, currentTime);
        }
    }
}

void Renderer::drawActiveMotions(const GameSnapshot &snapshot, long currentTime, long dt)
{
    int cellSize = layout.getCellSize();

    for (const auto &motion : snapshot.activeMotions) {
        if (motion.isDead) continue;

        if (pieceViews.find(motion.piece.id) == pieceViews.end()) {
            pieceViews.emplace(motion.piece.id, PieceView(motion.piece.id));
        }

        pieceViews[motion.piece.id].syncWithModel(motion.piece, assets, dt);

        auto [startX, startY] = layout.getCellPixelPos(motion.source.row, motion.source.col);
        auto [endX, endY] = layout.getCellPixelPos(motion.destination.row, motion.destination.col);

        double progress = 0.0;
        if (motion.arrivalTime > motion.startTime) {
            progress = static_cast<double>(currentTime - motion.startTime) / (motion.arrivalTime - motion.startTime);
            progress = std::max(0.0, std::min(1.0, progress));
        }

        int currentX = startX + static_cast<int>((endX - startX) * progress);
        int currentY = startY + static_cast<int>((endY - startY) * progress);

        if (motion.type == MotionType::Jump) {
            double arc = std::sin(progress * Theme::Pi);
            int jumpHeightPixels = static_cast<int>(cellSize * 1.5);
            currentY -= static_cast<int>(arc * jumpHeightPixels);
        }

        const Img &frame = pieceViews[motion.piece.id].getFrame();
        try {
            const_cast<Img &>(frame).draw_resized_on(windowBuffer, currentX, currentY, cellSize, cellSize);
        } catch (...) {}
    }
}

void Renderer::updateWindowSize(int w, int h)
{
    layout.updateWindowSize(w, h);
    const_cast<cv::Mat &>(windowBuffer.get_mat()) = cv::Mat(h, w, CV_8UC3, Theme::Background);
}

int Renderer::getWindowWidth() const { return windowBuffer.get_mat().cols; }

int Renderer::getWindowHeight() const { return windowBuffer.get_mat().rows; }