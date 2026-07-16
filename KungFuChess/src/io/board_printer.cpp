#include "io/board_printer.hpp"
#include <sstream>

std::string BoardPrinter::print(const GameState& state) {
    std::ostringstream output;
    
    for (int r = 0; r < state.board.getHeight(); ++r) {
        for (int c = 0; c < state.board.getWidth(); ++c) {
            auto pieceOpt = state.board.getPiece(Position{r, c});
            
            if (pieceOpt.has_value()) {
                Piece p = pieceOpt.value();
                output << (p.color == PieceColor::White ? "w" : "b");
                switch (p.kind) {
                    case PieceKind::King:   output << "K"; break;
                    case PieceKind::Queen:  output << "Q"; break;
                    case PieceKind::Rook:   output << "R"; break;
                    case PieceKind::Bishop: output << "B"; break;
                    case PieceKind::Knight: output << "N"; break;
                    case PieceKind::Pawn:   output << "P"; break;
                }
            } else {
                output << "."; // mark empty cell
            }
            
            if (c < state.board.getWidth() - 1) {
                output << " "; // separator between cells
            }
        }
        output << "\n"; // newline after each board row
    }
    
    return output.str();
}