#pragma once
#include "view/img.hpp"
#include "view/board_layout.hpp"
#include "model/game_snapshot.hpp"
#include "view/theme.hpp"

class BoardRenderer {
public:
    static void drawBackground(Img& windowBuffer, const BoardLayout& layout) {
        cv::Mat& canvas = const_cast<cv::Mat&>(windowBuffer.get_mat());
        if (canvas.empty()) return;

        auto [startX, startY] = layout.getBoardStartPos();
        int cellSize = layout.getCellSize();

        cv::rectangle(canvas, cv::Point(startX - Theme::BoardOutlineThickness, startY - Theme::BoardOutlineThickness),
                      cv::Point(startX + Theme::BoardCells * cellSize + Theme::BoardOutlineThickness, startY + Theme::BoardCells * cellSize + Theme::BoardOutlineThickness),
                      Theme::TableBorder, Theme::ThickBold);

        for (int r = 0; r < Theme::BoardCells; ++r) {
            for (int c = 0; c < Theme::BoardCells; ++c) {
                cv::Scalar color = ((r + c) % 2 == 0) ? Theme::BoardLight : Theme::BoardDark;
                cv::rectangle(canvas, cv::Rect(startX + c * cellSize, startY + r * cellSize, cellSize, cellSize), color, cv::FILLED);
            }
        }

        for (int i = 0; i < Theme::BoardCells; ++i) {
            std::string colLabel(1, 'a' + i);
            std::string rowLabel(1, '8' - i);
            windowBuffer.put_text(colLabel, startX + i * cellSize + (cellSize / 2) - Theme::PadSmall, startY - Theme::PadLarge, Theme::FontSmall, Theme::TextDark, Theme::ThickNormal);
            windowBuffer.put_text(colLabel, startX + i * cellSize + (cellSize / 2) - Theme::PadSmall, startY + Theme::BoardCells * cellSize + Theme::PadHuge, Theme::FontSmall, Theme::TextDark, Theme::ThickNormal);
            windowBuffer.put_text(rowLabel, startX - Theme::PadHuge, startY + i * cellSize + (cellSize / 2) + Theme::PadSmall, Theme::FontSmall, Theme::TextDark, Theme::ThickNormal);
            windowBuffer.put_text(rowLabel, startX + Theme::BoardCells * cellSize + Theme::PadLarge, startY + i * cellSize + (cellSize / 2) + Theme::PadSmall, Theme::FontSmall, Theme::TextDark, Theme::ThickNormal);
        }
    }

    static void drawHighlights(Img& windowBuffer, const BoardLayout& layout, const GameSnapshot& snapshot) {
        cv::Mat& canvas = const_cast<cv::Mat&>(windowBuffer.get_mat());
        if (canvas.empty()) return;

        int cellSize = layout.getCellSize();
        cv::Mat overlay;
        canvas.copyTo(overlay);
        bool shouldBlend = false;

        if (snapshot.selectedPiece.has_value()) {
            auto [pixelX, pixelY] = layout.getCellPixelPos(snapshot.selectedPiece.value().cell.row, snapshot.selectedPiece.value().cell.col);
            cv::rectangle(overlay, cv::Rect(pixelX, pixelY, cellSize, cellSize), Theme::HighlightSelected, cv::FILLED);
            shouldBlend = true;
        }

        for (const Position& pos : snapshot.highlightedCells) {
            auto [pixelX, pixelY] = layout.getCellPixelPos(pos.row, pos.col);
            cv::Scalar color = snapshot.getPieceAt(pos).has_value() ? Theme::HighlightEnemy : Theme::HighlightEmpty;
            cv::rectangle(overlay, cv::Rect(pixelX, pixelY, cellSize, cellSize), color, cv::FILLED);
            shouldBlend = true;
        }

        if (shouldBlend) {
            cv::addWeighted(overlay, Theme::AlphaBase, canvas, Theme::AlphaOverlay, 0, canvas);
        }
    }
};