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
    // 1. Calculate the available space for the board after subtracting UI margins
    int availableWidth = windowWidth - marginLeft - marginRight;
    int availableHeight = windowHeight - marginTop - marginBottom;

    // 2. Determine the maximum possible cell size that fits the available space
    int cellWidth = availableWidth / logicalCols;
    int cellHeight = availableHeight / logicalRows;
    
    // We must keep the cells perfectly square, so we take the smaller of the two
    cellSizePixels = std::min(cellWidth, cellHeight);

    // 3. Calculate the actual starting position of the board to keep it centered 
    // within the available space (in case the available space is not a perfect square)
    int actualBoardWidth = cellSizePixels * logicalCols;
    int actualBoardHeight = cellSizePixels * logicalRows;

    boardStartX = marginLeft + (availableWidth - actualBoardWidth) / 2;
    boardStartY = marginTop + (availableHeight - actualBoardHeight) / 2;
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