#include "doctest.h"
#include "io/board_parser.hpp"
#include "model/board.hpp"
#include <stdexcept>

TEST_CASE("BoardParser - Parsing text to Board") {

    SUBCASE("Parses a valid rectangular board correctly") {
        std::string text = 
            "wK . bR\n"
            ". . .\n"
            "wN . bK";
            
        Board board = BoardParser::parse(text);
        
        CHECK(board.getWidth() == 3);
        CHECK(board.getHeight() == 3);
        
        REQUIRE(board.getPiece(Position{0, 0}).has_value());
        CHECK(board.getPiece(Position{0, 0})->kind == PieceKind::King);
        CHECK(board.getPiece(Position{0, 0})->color == PieceColor::White);
        
        REQUIRE(board.getPiece(Position{0, 2}).has_value());
        CHECK(board.getPiece(Position{0, 2})->kind == PieceKind::Rook);
        CHECK(board.getPiece(Position{0, 2})->color == PieceColor::Black);
        
        CHECK(board.getPiece(Position{1, 1}).has_value() == false);
    }

    SUBCASE("Throws exception on inconsistent row lengths") {
        std::string invalidText = 
            "wK . bR\n"
            ". .\n"       
            "wN . bK";
            
        CHECK_THROWS_AS(BoardParser::parse(invalidText), std::invalid_argument);
    }

    SUBCASE("Throws exception on illegal piece token") {
        std::string invalidTokenText = 
            "wK xZ bR\n" 
            ". . .\n"
            "wN . bK";
            
        CHECK_THROWS_AS(BoardParser::parse(invalidTokenText), std::invalid_argument);
    }
}