#include "view/board_layout.hpp"
#include <algorithm>

BoardLayout::BoardLayout(int winWidth, int winHeight, int rows, int cols, 
                         int mLeft, int mTop, int mRight, int mBottom)
    : windowWidth(winWidth), windowHeight(winHeight), 
      logicalRows(rows), logicalCols(cols),
      marginLeft(mLeft), marginTop(mTop), marginRight(mRight), marginBottom(mBottom) {
    
    calculateLayout();
}

void BoardLayout::calculateLayout() {
    int curMarginLeft = std::min(marginLeft, windowWidth / 4);
    int curMarginRight = std::min(marginRight, windowWidth / 4);
    int curMarginTop = std::min(marginTop, windowHeight / 8);
    int curMarginBottom = std::min(marginBottom, windowHeight / 8);

    int availableWidth = std::max(10, windowWidth - curMarginLeft - curMarginRight);
    int availableHeight = std::max(10, windowHeight - curMarginTop - curMarginBottom);

    cellSizePixels = std::min(availableWidth / logicalCols, availableHeight / logicalRows);
    
    int actualBoardWidth = cellSizePixels * logicalCols;
    int actualBoardHeight = cellSizePixels * logicalRows;

    boardStartX = curMarginLeft + (availableWidth - actualBoardWidth) / 2;
    boardStartY = curMarginTop + (availableHeight - actualBoardHeight) / 2;
}

void BoardLayout::updateWindowSize(int newWidth, int newHeight) {
    windowWidth = newWidth;
    windowHeight = newHeight;
    calculateLayout();
}

int BoardLayout::getCellSize() const {
    return cellSizePixels;
}

std::pair<int, int> BoardLayout::getCellPixelPos(int row, int col) const {
    // X axis relates to columns, Y axis relates to rows
    int pixelX = boardStartX + (col * cellSizePixels);
    int pixelY = boardStartY + (row * cellSizePixels);
    
    return {pixelX, pixelY};
}

std::pair<int, int> BoardLayout::getBoardStartPos() const {
    return {boardStartX, boardStartY};
}