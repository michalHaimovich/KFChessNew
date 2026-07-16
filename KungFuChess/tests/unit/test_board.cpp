#include "doctest.h"

#include "model/board.hpp"

#include "model/piece.hpp"

TEST_CASE("Board - Logical Occupancy and Movement") {
    // Create an 8x8 board
    Board board(8, 8);

    SUBCASE("Board dimensions are inferred/stored correctly") {
        CHECK(board.getWidth() == 8);
        CHECK(board.getHeight() == 8);
    }

    SUBCASE("Empty cells return no piece") {
        CHECK_FALSE(board.getPiece({0, 0}).has_value());
    }

    SUBCASE("Occupied cells return the correct piece") {
        Piece pawn(1, PieceColor::White, PieceKind::Pawn, {1, 1});
        bool added = board.addPiece(pawn);

        CHECK(added == true);
        auto piece_opt = board.getPiece({1, 1});
        REQUIRE(piece_opt.has_value());
        CHECK(piece_opt->id == 1);
    }

    SUBCASE("Adding two pieces to the same cell fails (reject duplicate occupancy)") {
        Piece first(1, PieceColor::White, PieceKind::Rook, {0, 0});
        Piece second(2, PieceColor::Black, PieceKind::Knight, {0, 0});

        CHECK(board.addPiece(first) == true);
        // The second insertion should be rejected
        CHECK(board.addPiece(second) == false); 
    }

    SUBCASE("Moving a piece updates source and destination") {
        Piece rook(1, PieceColor::White, PieceKind::Rook, {0, 0});
        board.addPiece(rook);

        // Move the rook to the other side of the board
        board.movePiece({0, 0}, {0, 7});

        // Source must be empty
        CHECK_FALSE(board.getPiece({0, 0}).has_value()); 
        
        // Destination must contain the piece
        auto dest_piece = board.getPiece({0, 7});
        REQUIRE(dest_piece.has_value());
        CHECK(dest_piece->id == 1);
        
        // The piece's internal cell property must update too
        CHECK(dest_piece->cell == Position{0, 7}); 
    }

    SUBCASE("Removing a captured piece clears its cell") {
        Piece knight(1, PieceColor::Black, PieceKind::Knight, {5, 5});
        board.addPiece(knight);

        bool removed = board.removePiece({5, 5});

        CHECK(removed == true);
        CHECK_FALSE(board.getPiece({5, 5}).has_value());
    }
}