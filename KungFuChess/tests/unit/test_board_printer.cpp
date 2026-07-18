#include "doctest.h"
#include "io/board_printer.hpp"
#include "model/game_state.hpp"

TEST_CASE("BoardPrinter - Printing GameState to text") {

    SUBCASE("Prints a basic board state correctly") {
        GameState state(3, 3);
        
        state.board.addPiece(Piece(1, PieceColor::White, PieceKind::King, Position{0, 0}));
        state.board.addPiece(Piece(2, PieceColor::Black, PieceKind::Rook, Position{0, 2}));
        state.board.addPiece(Piece(3, PieceColor::White, PieceKind::Knight, Position{2, 0}));
        state.board.addPiece(Piece(4, PieceColor::Black, PieceKind::King, Position{2, 2}));

        std::string expectedOutput = 
            "wK . bR\n"
            ". . .\n"
            "wN . bK\n"; 

        std::string actualOutput = BoardPrinter::print(state);
        CHECK(actualOutput == expectedOutput);
    }

    SUBCASE("Prints a completely empty board") {
        GameState state(2, 2);
        
        std::string expectedOutput = 
            ". .\n"
            ". .\n"; 
            
        CHECK(BoardPrinter::print(state) == expectedOutput);
    }
}