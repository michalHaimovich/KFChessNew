#pragma once

#include <utility>

class BoardLayout {
private:
    int windowWidth;
    int windowHeight;
    
    // Logical dimensions of the board (e.g., 8x8, 10x10)
    int logicalRows;
    int logicalCols;

    // Margins around the board to leave space for UI panels
    int marginLeft;
    int marginTop;
    int marginRight;
    int marginBottom;

    // Calculated fields
    int cellSizePixels;
    int boardStartX;
    int boardStartY;

    // Recalculates the internal pixel values based on current window and margins
    void calculateLayout();

public:
    // Constructor allows full generic configuration without magic numbers
    BoardLayout(int winWidth, int winHeight, 
                int rows, int cols, 
                int mLeft = 0, int mTop = 0, int mRight = 0, int mBottom = 0);

    // Updates the window size and recalculates everything (useful if window is resizable)
    void updateWindowSize(int newWidth, int newHeight);

    // Returns the exact size of a single cell in pixels
    int getCellSize() const;

    // Returns the top-left X,Y pixel coordinate for a specific logical row and column
    std::pair<int, int> getCellPixelPos(int row, int col) const;

    // Returns the starting X,Y pixel coordinate of the entire board
    std::pair<int, int> getBoardStartPos() const;
};