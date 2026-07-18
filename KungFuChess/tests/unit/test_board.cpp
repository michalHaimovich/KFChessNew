#include "doctest.h"
#include "model/board.hpp"
#include "model/piece.hpp"

TEST_CASE("Board - Dimensions and Bounds") {
    Board board(8, 8); 
    SUBCASE("Board initializes with correct dimensions") {
        CHECK(board.getWidth() == 8);
        CHECK(board.getHeight() == 8);
    }

    SUBCASE("isInside correctly identifies valid positions") {
        CHECK(board.isInside(Position{0, 0}) == true);  
        CHECK(board.isInside(Position{7, 7}) == true);   
        CHECK(board.isInside(Position{3, 4}) == true);   
    }

    SUBCASE("isInside correctly identifies invalid positions") {
        CHECK(board.isInside(Position{-1, 0}) == false);
        CHECK(board.isInside(Position{0, -1}) == false); 
        CHECK(board.isInside(Position{8, 0}) == false);  
        CHECK(board.isInside(Position{0, 8}) == false);  
    }
}

TEST_CASE("Board - Piece Management") {
    Board board(8, 8);
    Piece whitePawn(1, PieceColor::White, PieceKind::Pawn, Position{1, 1});
    Piece blackKnight(2, PieceColor::Black, PieceKind::Knight, Position{7, 7});

    SUBCASE("Getting a piece from an empty cell returns nullopt") {
        CHECK(board.getPiece(Position{2, 2}).has_value() == false);
    }

    SUBCASE("Adding and retrieving a piece successfully") {
        CHECK(board.addPiece(whitePawn) == true);
        
        auto retrievedOpt = board.getPiece(Position{1, 1});
        REQUIRE(retrievedOpt.has_value());
        CHECK(retrievedOpt->id == 1);
        CHECK(retrievedOpt->color == PieceColor::White);
        CHECK(retrievedOpt->kind == PieceKind::Pawn);
    }

    SUBCASE("Adding a piece out of bounds returns false") {
        Piece outOfBoundsPiece(3, PieceColor::White, PieceKind::Rook, Position{10, 10});
        CHECK(board.addPiece(outOfBoundsPiece) == false);
    }

    SUBCASE("Removing an existing piece returns true and clears the cell") {
        board.addPiece(whitePawn);
        CHECK(board.removePiece(Position{1, 1}) == true);
        CHECK(board.getPiece(Position{1, 1}).has_value() == false);
    }

    SUBCASE("Removing a piece from an empty cell returns false") {
        CHECK(board.removePiece(Position{5, 5}) == false);
    }

    SUBCASE("Moving a piece clears the source and updates the destination") {
        board.addPiece(blackKnight);
        
        board.movePiece(Position{7, 7}, Position{5, 6});
        
        CHECK(board.getPiece(Position{7, 7}).has_value() == false);
        
        auto movedPieceOpt = board.getPiece(Position{5, 6});
        REQUIRE(movedPieceOpt.has_value());
        CHECK(movedPieceOpt->id == 2);
        CHECK(movedPieceOpt->kind == PieceKind::Knight);
    }
}