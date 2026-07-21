#pragma once
#include "view/img.hpp"
#include "view/board_layout.hpp"
#include "model/game_snapshot.hpp"
#include "view/score_manager.hpp"
#include "view/move_history_manager.hpp"
#include "view/theme.hpp"

class UIRenderer {
private:
    static void drawMoveTable(Img& windowBuffer, const std::string& title, int x, int y, int w, int h, const std::vector<MoveRecord>& moves) {
        cv::Mat& canvas = const_cast<cv::Mat&>(windowBuffer.get_mat());

        cv::rectangle(canvas, cv::Rect(x, y, w, h), Theme::TextLight, cv::FILLED);
        cv::rectangle(canvas, cv::Rect(x, y, w, h), Theme::TableBorder, Theme::ThickBold);

        windowBuffer.put_text(title, x + Theme::PadMedium, y - Theme::PadMedium, Theme::FontMedium, Theme::TextDark, Theme::ThickBold);

        cv::line(canvas, cv::Point(x, y + Theme::TableHeaderHeight), cv::Point(x + w, y + Theme::TableHeaderHeight), Theme::TableBorder, Theme::ThickNormal);
        cv::line(canvas, cv::Point(x + Theme::TableDividerX, y), cv::Point(x + Theme::TableDividerX, y + h), Theme::TableBorder, Theme::ThickNormal);
        
        windowBuffer.put_text("Time", x + Theme::PadLarge, y + Theme::PadExtra, Theme::FontSmall, Theme::TextMuted, Theme::ThickNormal);
        windowBuffer.put_text("Move", x + Theme::MoveColOffset, y + Theme::PadExtra, Theme::FontSmall, Theme::TextMuted, Theme::ThickNormal);

        int startIdx = (moves.size() > static_cast<size_t>(Theme::TableMaxMoves)) ? moves.size() - Theme::TableMaxMoves : 0;
        int rowY = y + Theme::TableHeaderHeight + Theme::RowStartYOffset;
        
        for (size_t i = startIdx; i < moves.size(); ++i) {
            windowBuffer.put_text(moves[i].timeStr, x + Theme::TimeColOffset, rowY, Theme::FontTiny, Theme::TextDark, Theme::ThickNormal);
            windowBuffer.put_text(moves[i].moveStr, x + Theme::MoveColOffset, rowY, Theme::FontTiny, Theme::TextRed, Theme::ThickNormal);
            cv::line(canvas, cv::Point(x, rowY + Theme::PadSmall), cv::Point(x + w, rowY + Theme::PadSmall), Theme::LineLight, Theme::ThickNormal);
            rowY += Theme::TableRowHeight;
        }
    }

public:
    static void drawSidePanels(Img& windowBuffer, const BoardLayout& layout, const GameSnapshot& snapshot, ScoreManager* scoreMgr, MoveHistoryManager* historyMgr) {
        cv::Mat& canvas = const_cast<cv::Mat&>(windowBuffer.get_mat());
        if (canvas.empty()) return;

        auto [boardX, boardY] = layout.getBoardStartPos();
        int boardWidth = layout.getCellSize() * Theme::BoardCells;
        int winHeight = canvas.rows;

        int tableW = std::min(Theme::TableMaxW, boardX - Theme::PadExtra);
        if (tableW < Theme::TableMinW) tableW = Theme::TableMinW;
        int tableH = std::min(Theme::TableMaxH, winHeight - boardY - Theme::PadExtra);
        
        int leftTableX = boardX - tableW - Theme::PadMedium;
        int rightTableX = boardX + boardWidth + Theme::PadMedium;
        int tableY = boardY + Theme::PadExtra;

        std::string blackScoreStr = snapshot.blackPlayerName + " (Black): " + std::to_string(scoreMgr->getBlackScore());
        std::string whiteScoreStr = snapshot.whitePlayerName + " (White): " + std::to_string(scoreMgr->getWhiteScore());

        double fontScale = std::max(Theme::FontTiny, std::min(Theme::FontLarge, tableW / 200.0));
        windowBuffer.put_text(blackScoreStr, leftTableX, boardY - Theme::PadLarge, fontScale, Theme::TextDark, Theme::ThickBold);
        windowBuffer.put_text(whiteScoreStr, rightTableX, boardY - Theme::PadLarge, fontScale, Theme::TextDark, Theme::ThickBold);

        drawMoveTable(windowBuffer, "Black Actions", leftTableX, tableY, tableW, tableH, historyMgr->getBlackMoves());
        drawMoveTable(windowBuffer, "White Actions", rightTableX, tableY, tableW, tableH, historyMgr->getWhiteMoves());
    }

    static void drawGameOverOverlay(Img& windowBuffer, const BoardLayout& layout, const GameSnapshot& snapshot) {
        if (!snapshot.isGameOver) return;

        cv::Mat& canvas = const_cast<cv::Mat&>(windowBuffer.get_mat());
        if (canvas.empty()) return;

        auto [boardX, boardY] = layout.getBoardStartPos();
        int boardW = layout.getCellSize() * Theme::BoardCells;
        int boardH = boardW;

        cv::Mat overlay;
        canvas.copyTo(overlay);
        cv::rectangle(overlay, cv::Rect(boardX, boardY, boardW, boardH), Theme::OverlayDark, cv::FILLED);
        cv::addWeighted(overlay, Theme::AlphaOverlay, canvas, Theme::AlphaBase, 0, canvas);

        std::string text = "GAME OVER";
        cv::Scalar textColor = Theme::TextMuted;

        if (snapshot.winner.has_value()) {
            text = (snapshot.winner.value() == PieceColor::White) ? "WHITE WINS!" : "BLACK WINS!";
            textColor = (snapshot.winner.value() == PieceColor::White) ? Theme::TextLight : Theme::TextDark;
        }

        int baseline = 0;
        cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_DUPLEX, Theme::FontHuge, Theme::ThickBold, &baseline);
        int textX = boardX + (boardW - textSize.width) / 2;
        int textY = boardY + (boardH + textSize.height) / 2;

        cv::Scalar outlineColor = (textColor[0] > 128) ? Theme::OverlayDark : Theme::TextLight;
        cv::putText(canvas, text, cv::Point(textX, textY), cv::FONT_HERSHEY_DUPLEX, Theme::FontHuge, outlineColor, Theme::ThickTitle, cv::LINE_AA);
        cv::putText(canvas, text, cv::Point(textX, textY), cv::FONT_HERSHEY_DUPLEX, Theme::FontHuge, textColor, Theme::ThickBold, cv::LINE_AA);

        int btnX = boardX + (boardW - Theme::BtnWidth) / 2;
        int btnY = boardY + (boardH) / 2 + Theme::BtnOffsetY;
        
        cv::rectangle(canvas, cv::Rect(btnX, btnY, Theme::BtnWidth, Theme::BtnHeight), Theme::ButtonBg, cv::FILLED);
        cv::rectangle(canvas, cv::Rect(btnX, btnY, Theme::BtnWidth, Theme::BtnHeight), Theme::TableBorder, Theme::ThickBold);
        
        baseline = 0;
        cv::Size btnTextSize = cv::getTextSize("Return to Lobby", cv::FONT_HERSHEY_DUPLEX, Theme::FontLarge, Theme::ThickNormal, &baseline);
        cv::putText(canvas, "Return to Lobby", 
                    cv::Point(btnX + (Theme::BtnWidth - btnTextSize.width) / 2, btnY + (Theme::BtnHeight + btnTextSize.height) / 2), 
                    cv::FONT_HERSHEY_DUPLEX, Theme::FontLarge, Theme::TableBorder, Theme::ThickNormal, cv::LINE_AA);
    }

    static void drawRestBar(Img& windowBuffer, const BoardLayout& layout, const Piece& piece, long currentTime) {
        cv::Mat& pixels = const_cast<cv::Mat&>(windowBuffer.get_mat());
        auto [pixelX, pixelY] = layout.getCellPixelPos(piece.cell.row, piece.cell.col);

        int width = layout.getCellSize() - 2 * Theme::RestBarInset;
        int x = pixelX + Theme::RestBarInset;
        int y = pixelY + layout.getCellSize() - Theme::RestBarInset - Theme::RestBarHeight;

        cv::rectangle(pixels, cv::Rect(x, y, width, Theme::RestBarHeight), Theme::RestBarTrack, cv::FILLED);

        double progress = 1.0;
        if (piece.readyTime > currentTime) {
            long totalCooldown = (piece.state == PieceState::LongRest) ? Theme::CooldownLong : Theme::CooldownShort;
            progress = 1.0 - (static_cast<double>(piece.readyTime - currentTime) / totalCooldown);
        }
        progress = std::max(0.0, std::min(1.0, progress));

        int left = static_cast<int>(width * progress);
        if (left > 0) {
            cv::rectangle(pixels, cv::Rect(x, y, left, Theme::RestBarHeight), Theme::RestBarColor, cv::FILLED);
        }
    }
};