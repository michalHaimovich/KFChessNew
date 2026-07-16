#include "view/renderer.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

Renderer::Renderer(int winWidth, int winHeight, 
                   int logicalRows, int logicalCols, 
                   int marginLeft, int marginTop, int marginRight, int marginBottom,
                   const std::string& assetsPath)
    : layout(winWidth, winHeight, logicalRows, logicalCols, marginLeft, marginTop, marginRight, marginBottom),
      assetsBasePath(assetsPath) {
    
    // פקודה אחת שטוענת את כל הזיכרון בתחילת המשחק!
    assets.loadAllAssets(assetsBasePath, layout.getCellSize());
}

void Renderer::renderFrame(const GameSnapshot& snapshot, long currentTime, long dt) {
    clearScreen();
    drawUIBackground();
    drawBoardBackground();
    
    drawStationaryPieces(snapshot, dt);
    drawActiveMotions(snapshot, currentTime, dt);
    
    drawSidePanels(snapshot);
    
    cv::imshow("Kung Fu Chess", windowBuffer.get_mat());
}

void Renderer::clearScreen() {
    cv::Mat& mat = const_cast<cv::Mat&>(windowBuffer.get_mat());
    
    if (mat.empty() || mat.cols == 0 || mat.rows == 0) {
        mat = cv::Mat(700, 1000, CV_8UC3, cv::Scalar(40, 40, 40));
    } else {
        mat.setTo(cv::Scalar(40, 40, 40));
    }
}

void Renderer::drawBoardBackground() {
    cv::Mat& canvas = const_cast<cv::Mat&>(windowBuffer.get_mat());
    if (canvas.empty()) return; 

    auto [startX, startY] = layout.getBoardStartPos();
    int cellSize = layout.getCellSize();
    
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            cv::Scalar color = ((r + c) % 2 == 0) ? cv::Scalar(255, 255, 255) : cv::Scalar(200, 200, 200);
            cv::rectangle(canvas, cv::Rect(startX + c*cellSize, startY + r*cellSize, cellSize, cellSize), color, cv::FILLED);
        }
    }
}

void Renderer::drawStationaryPieces(const GameSnapshot& snapshot, long dt) {
    for (const Piece& piece : snapshot.stationaryPieces) {
        if (pieceViews.find(piece.id) == pieceViews.end()) {
            pieceViews.emplace(piece.id, PieceView(piece.id));
        }
        
        pieceViews[piece.id].syncWithModel(piece, assets, dt);
        
        auto [pixelX, pixelY] = layout.getCellPixelPos(piece.cell.row, piece.cell.col);
        const Img& frame = pieceViews[piece.id].getFrame();
        
        try {
            const_cast<Img&>(frame).draw_on(windowBuffer, pixelX, pixelY);
        } catch (...) {
            std::cerr << "Warning: Stationary piece out of bounds at " << pixelX << ", " << pixelY << std::endl;
        }
    }
}

void Renderer::drawActiveMotions(const GameSnapshot& snapshot, long currentTime, long dt) {
    int cellSize = layout.getCellSize();

    for (const auto& motion : snapshot.activeMotions) {
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
            double arc = std::sin(progress * 3.14159265358979323846);
            int jumpHeightPixels = static_cast<int>(cellSize * 1.5);
            currentY -= static_cast<int>(arc * jumpHeightPixels);
        }

        const Img& frame = pieceViews[motion.piece.id].getFrame();

        try {
            const_cast<Img&>(frame).draw_on(windowBuffer, currentX, currentY);
        } catch (...) {
        }
    }
}

void Renderer::drawUIBackground() {}
void Renderer::drawSidePanels(const GameSnapshot& snapshot) {}