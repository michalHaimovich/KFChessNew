#include "io/board_parser.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib> // needed for std::exit

Board BoardParser::parse(const std::string& text) {
    std::istringstream stream(text);
    std::string line;
    std::vector<std::vector<std::string>> grid;

    // 1. read text and split into tokens
    while (std::getline(stream, line)) {
        if (line.empty() || line.find("Board") != std::string::npos) continue; 

        std::vector<std::string> rowTokens;
        std::istringstream lineStream(line);
        std::string token;
        while (lineStream >> token) {
            rowTokens.push_back(token);
        }
        if (!rowTokens.empty()) {
            grid.push_back(rowTokens);
        }
    }

    if (grid.empty()) return Board(0, 0);

    // ===============================================
    // step 2: validation (checks that prevent test crashes)
    // ===============================================
    
    // check A: are all rows the same width? (prevent out of bounds)
    size_t expectedWidth = grid[0].size();
    for (const auto& row : grid) {
        if (row.size() != expectedWidth) {
            std::cout << "ERROR ROW_WIDTH_MISMATCH\n";
            std::exit(0); 
        }
    }

    // check B: are there invalid tokens?
    for (int r = 0; r < grid.size(); ++r) {
        for (int c = 0; c < expectedWidth; ++c) {
            std::string token = grid[r][c];
            
            if (token == ".") continue;

            if (token.length() != 2 || (token[0] != 'w' && token[0] != 'b')) {
                std::cout << "ERROR UNKNOWN_TOKEN\n";
                std::exit(0);
            }

            char kind = token[1];
            if (kind != 'K' && kind != 'Q' && kind != 'R' && 
                kind != 'B' && kind != 'N' && kind != 'P') {
                std::cout << "ERROR UNKNOWN_TOKEN\n";
                std::exit(0);
            }
        }
    }

    // ===============================================
    // step 3: build the board only after validation
    // ===============================================
    int height = grid.size();
    int width = expectedWidth;
    Board board(width, height);

    int nextId = 1;

    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            std::string token = grid[r][c];
            if (token == ".") continue;

            PieceColor color = (token[0] == 'w') ? PieceColor::White : PieceColor::Black;
            PieceKind kind;
            char kindChar = token[1];

            switch (kindChar) {
                case 'K': kind = PieceKind::King; break;
                case 'Q': kind = PieceKind::Queen; break;
                case 'R': kind = PieceKind::Rook; break;
                case 'B': kind = PieceKind::Bishop; break;
                case 'N': kind = PieceKind::Knight; break;
                case 'P': kind = PieceKind::Pawn; break;
            }

            Piece piece(nextId++, color, kind, Position{r, c});
            board.addPiece(piece);
        }
    }

    return board;
}