#include "view/renderer.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

Renderer::Renderer(int winWidth, int winHeight, 
                   int logicalRows, int logicalCols, 
                   int marginLeft, int marginTop, int marginRight, int marginBottom,
                   const std::string& assetsPath,
                   ScoreManager* scoreMgr,
                   MoveHistoryManager* historyMgr)
    : layout(winWidth, winHeight, logicalRows, logicalCols, marginLeft, marginTop, marginRight, marginBottom),
      assetsBasePath(assetsPath),
      scoreManager(scoreMgr),
      historyManager(historyMgr) {
    
    assets.loadAllAssets(assetsBasePath, layout.getCellSize());
}

void Renderer::renderFrame(const GameSnapshot& snapshot, long currentTime, long dt) {
    clearScreen();
    drawUIBackground();
    drawBoardBackground();
    
    drawStationaryPieces(snapshot, dt);
    drawActiveMotions(snapshot, currentTime, dt);
    
    drawSidePanels(snapshot); // הציור של כל הפאנלים והטקסטים
    
    cv::imshow("Kung Fu Chess", windowBuffer.get_mat());
}

void Renderer::clearScreen() {
    cv::Mat& mat = const_cast<cv::Mat&>(windowBuffer.get_mat());
    
    if (mat.empty() || mat.cols == 0 || mat.rows == 0) {
        mat = cv::Mat(700, 1000, CV_8UC3, cv::Scalar(110, 110, 110)); // שינוי לרקע אפור בהיר יפה של ה-UI
    } else {
        mat.setTo(cv::Scalar(110, 110, 110));
    }
}

void Renderer::drawBoardBackground() {
    cv::Mat& canvas = const_cast<cv::Mat&>(windowBuffer.get_mat());
    if (canvas.empty()) return; 

    auto [startX, startY] = layout.getBoardStartPos();
    int cellSize = layout.getCellSize();
    
    // ציור מסגרת שחורה דקה מסביב ללוח השחמט
    cv::rectangle(canvas, cv::Point(startX - 2, startY - 2), 
                  cv::Point(startX + 8 * cellSize + 2, startY + 8 * cellSize + 2), 
                  cv::Scalar(0, 0, 0), 2);

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            cv::Scalar color = ((r + c) % 2 == 0) ? cv::Scalar(219, 235, 251) : cv::Scalar(124, 163, 206); // צבעי עץ קלאסיים (חום בהיר וחום כהה)
            cv::rectangle(canvas, cv::Rect(startX + c*cellSize, startY + r*cellSize, cellSize, cellSize), color, cv::FILLED);
        }
    }
    
    // ציור האותיות (a-h) והמספרים (1-8) מסביב ללוח
    for (int i = 0; i < 8; ++i) {
        std::string colLabel(1, 'a' + i);
        std::string rowLabel(1, '8' - i);
        
        // אותיות למטה ולמעלה
        windowBuffer.put_text(colLabel, startX + i * cellSize + (cellSize / 2) - 5, startY - 15, 0.5, cv::Scalar(40, 40, 40), 1);
        windowBuffer.put_text(colLabel, startX + i * cellSize + (cellSize / 2) - 5, startY + 8 * cellSize + 25, 0.5, cv::Scalar(40, 40, 40), 1);
        
        // מספרים משמאל ומימין
        windowBuffer.put_text(rowLabel, startX - 25, startY + i * cellSize + (cellSize / 2) + 5, 0.5, cv::Scalar(40, 40, 40), 1);
        windowBuffer.put_text(rowLabel, startX + 8 * cellSize + 15, startY + i * cellSize + (cellSize / 2) + 5, 0.5, cv::Scalar(40, 40, 40), 1);
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

void Renderer::drawUIBackground() {
    // השתמשנו ב-clearScreen כדי לצבוע את כל המסך באפור, אין צורך בציור רקע נוסף כאן
}

void Renderer::drawSidePanels(const GameSnapshot& snapshot) {
    cv::Mat& canvas = const_cast<cv::Mat&>(windowBuffer.get_mat());
    if (canvas.empty()) return;

    // שליפת מיקומי הלוח בשביל למקם את ה-UI בצורה יחסית מושלמת
    auto [boardX, boardY] = layout.getBoardStartPos();
    int cellSize = layout.getCellSize();
    int boardWidth = cellSize * 8;

    // 1. ציור שמות השחקנים והניקוד שלהם
    std::string blackScoreStr = "Score: " + std::to_string(scoreManager->getBlackScore());
    std::string whiteScoreStr = "Score: " + std::to_string(scoreManager->getWhiteScore());

    // שחור (למעלה)
    windowBuffer.put_text(blackScoreStr, boardX + (boardWidth / 2) - 40, boardY - 45, 0.6, cv::Scalar(20, 20, 20), 2);

    // לבן (למטה)
    windowBuffer.put_text(whiteScoreStr, boardX + (boardWidth / 2) - 40, boardY + boardWidth + 45, 0.6, cv::Scalar(20, 20, 20), 2);

    // 2. הגדרת מיקום ומימדים לשתי טבלאות התנועות בצדדים
    int tableW = 220;
    int tableH = 380;
    int tableY = boardY + 20;

    int leftTableX = boardX - tableW - 40; // משמאל ללוח
    int rightTableX = boardX + boardWidth + 40; // מימין ללוח

    // 3. ציור שתי הטבלאות בפועל
    drawMoveTable("Black Actions", leftTableX, tableY, tableW, tableH, historyManager->getBlackMoves());
    drawMoveTable("White Actions", rightTableX, tableY, tableW, tableH, historyManager->getWhiteMoves());
}

void Renderer::drawMoveTable(const std::string& title, int x, int y, int w, int h, const std::vector<MoveRecord>& moves) {
    cv::Mat& canvas = const_cast<cv::Mat&>(windowBuffer.get_mat());
    
    // ציור הרקע הלבן של הטבלה
    cv::rectangle(canvas, cv::Rect(x, y, w, h), cv::Scalar(255, 255, 255), cv::FILLED);
    cv::rectangle(canvas, cv::Rect(x, y, w, h), cv::Scalar(0, 0, 0), 2); // מסגרת שחורה דקה

    // כותרת השחקן של הטבלה (למשל "Black Actions" מעל הרקע הלבן)
    windowBuffer.put_text(title, x + 10, y - 10, 0.55, cv::Scalar(20, 20, 20), 2);

    // ציור קו מפריד לכותרות העמודות (Time | Move)
    int headerH = 30;
    cv::line(canvas, cv::Point(x, y + headerH), cv::Point(x + w, y + headerH), cv::Scalar(0, 0, 0), 1);
    cv::line(canvas, cv::Point(x + 100, y), cv::Point(x + 100, y + h), cv::Scalar(0, 0, 0), 1); // קו מפריד אנכי

    windowBuffer.put_text("Time", x + 15, y + 20, 0.45, cv::Scalar(100, 100, 100), 1);
    windowBuffer.put_text("Move", x + 115, y + 20, 0.45, cv::Scalar(100, 100, 100), 1);

    // הגבלת כמות השורות הנראות בטבלה כדי למנוע גלישת טקסט
    int maxRows = 14;
    int startIdx = 0;
    if (moves.size() > static_cast<size_t>(maxRows)) {
        startIdx = moves.size() - maxRows; // מציג תמיד את התנועות האחרונות (Auto-Scrolling)
    }

    // הדפסת השורות בתוך הטבלה
    int rowY = y + headerH + 22;
    for (size_t i = startIdx; i < moves.size(); ++i) {
        const auto& record = moves[i];
        
        // הדפסת זמן ותנועה
        windowBuffer.put_text(record.timeStr, x + 8, rowY, 0.4, cv::Scalar(40, 40, 40), 1);
        windowBuffer.put_text(record.moveStr, x + 115, rowY, 0.4, cv::Scalar(10, 10, 200), 1); // צבע אדום כהה לתנועה
        
        // קו תחתון מפריד דק לכל שורה
        cv::line(canvas, cv::Point(x, rowY + 5), cv::Point(x + w, rowY + 5), cv::Scalar(230, 230, 230), 1);
        
        rowY += 24; // ירידה של שורה למטה
    }
}