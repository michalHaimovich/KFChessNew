#include "doctest.h"
#include "engine/game_engine.hpp"
#include "model/game_observer.hpp"

// Mock Observer to track event emissions without needing the real UI
class MockObserver : public GameObserver {
public:
    int movesCompleted = 0;
    int piecesCaptured = 0;

    void onMoveCompleted(const Piece& piece, Position source, Position dest, bool destinationCapture, long timeMs) override {
        movesCompleted++;
    }

    void onPieceCaptured(const Piece& capturedPiece) override {
        piecesCaptured++;
    }
};

TEST_CASE("GameSnapshot - Read-Only State Representation") {
    GameEngine engine(8, 8);
    engine.setupStandardBoard();
    GameSnapshot snap = engine.getSnapshot();

    SUBCASE("isInside returns true for valid coordinates") {
        CHECK(snap.isInside(Position{0, 0}) == true);
        CHECK(snap.isInside(Position{7, 7}) == true);
        CHECK(snap.isInside(Position{3, 4}) == true);
    }

    SUBCASE("isInside returns false for out-of-bound coordinates") {
        CHECK(snap.isInside(Position{-1, 0}) == false);
        CHECK(snap.isInside(Position{0, -1}) == false);
        CHECK(snap.isInside(Position{8, 8}) == false);
        CHECK(snap.isInside(Position{10, 5}) == false);
    }

    SUBCASE("getPieceAt returns the correct piece for occupied cells") {
        auto pieceOpt = snap.getPieceAt(Position{1, 0}); // Black Pawn standard position
        REQUIRE(pieceOpt.has_value());
        CHECK(pieceOpt.value().kind == PieceKind::Pawn);
        CHECK(pieceOpt.value().color == PieceColor::Black);
    }

    SUBCASE("getPieceAt returns std::nullopt for empty cells") {
        auto pieceOpt = snap.getPieceAt(Position{4, 4}); // Middle of standard board is empty
        CHECK(pieceOpt.has_value() == false);
    }
}

TEST_CASE("GameEngine - Initialization and Setup") {
    SUBCASE("Engine initializes with correct dimensions and active state") {
        GameEngine engine(8, 8);
        GameSnapshot snap = engine.getSnapshot();
        
        CHECK(snap.boardWidth == 8);
        CHECK(snap.boardHeight == 8);
        CHECK(snap.isGameOver == false);
        CHECK(snap.stationaryPieces.empty() == true);
        CHECK(snap.activeMotions.empty() == true);
    }

    SUBCASE("setupStandardBoard places exactly 32 pieces") {
        GameEngine engine(8, 8);
        engine.setupStandardBoard();
        GameSnapshot snap = engine.getSnapshot();
        
        CHECK(snap.stationaryPieces.size() == 32);
        CHECK(snap.activeMotions.empty() == true); // Pieces should be stationary initially
    }
}

TEST_CASE("GameEngine - Requesting Moves") {
    GameEngine engine(8, 8);
    engine.setupStandardBoard();

    SUBCASE("Valid move is accepted and starts motion") {
        // Moving a White Pawn forward
        bool result = engine.requestMove(Position{6, 0}, Position{5, 0}, 1000);
        CHECK(result == true);
        
        GameSnapshot snap = engine.getSnapshot();
        // The piece should be removed from stationary and added to active motions
        CHECK(snap.getPieceAt(Position{6, 0}).has_value() == false); 
        CHECK(snap.activeMotions.size() == 1);
    }

    SUBCASE("Move is rejected if requested from an empty cell") {
        bool result = engine.requestMove(Position{4, 4}, Position{3, 4}, 1000);
        CHECK(result == false);
    }

    SUBCASE("Move is rejected if piece is already moving") {
        engine.requestMove(Position{6, 0}, Position{5, 0}, 1000);
        
        // Try to move the same piece again from its source (which is now empty)
        bool result1 = engine.requestMove(Position{6, 0}, Position{4, 0}, 1000);
        CHECK(result1 == false);
    }

    SUBCASE("Move is rejected if the game is over") {
        engine.getGameState().isGameOver = true; // Simulating game over
        bool result = engine.requestMove(Position{6, 0}, Position{5, 0}, 1000);
        CHECK(result == false);
    }
}

TEST_CASE("GameEngine - Requesting Jumps") {
    GameEngine engine(8, 8);
    engine.setupStandardBoard();

    SUBCASE("Valid jump request is accepted") {
        bool result = engine.requestJump(Position{6, 0});
        CHECK(result == true);

        GameSnapshot snap = engine.getSnapshot();
        CHECK(snap.activeMotions.size() == 1);
        CHECK(snap.activeMotions[0].type == MotionType::Jump);
    }

    SUBCASE("Jump is rejected on empty cell") {
        bool result = engine.requestJump(Position{4, 4});
        CHECK(result == false);
    }

    SUBCASE("Jump is rejected if the game is over") {
        engine.getGameState().isGameOver = true;
        bool result = engine.requestJump(Position{6, 0});
        CHECK(result == false);
    }
}

TEST_CASE("GameEngine - Observers and Events") {
    GameEngine engine(8, 8);
    MockObserver mockUI;
    engine.addObserver(&mockUI);

    // Setup custom board to force a quick capture
    Piece attacker(1, PieceColor::White, PieceKind::Rook, Position{0, 0});
    Piece victim(2, PieceColor::Black, PieceKind::Pawn, Position{0, 3});
    
    engine.getGameState().board.addPiece(attacker);
    engine.getGameState().board.addPiece(victim);

    SUBCASE("Completing a move triggers onMoveCompleted") {
        engine.requestMove(Position{0, 0}, Position{0, 1}, 1000);
        
        engine.wait(500); // Mid-way
        CHECK(mockUI.movesCompleted == 0);

        engine.wait(600); // Arrived (1100ms total)
        CHECK(mockUI.movesCompleted == 1);
        CHECK(mockUI.piecesCaptured == 0);
    }

    SUBCASE("Capturing a piece triggers both onMoveCompleted and onPieceCaptured") {
        // Move rook directly onto the pawn
        engine.requestMove(Position{0, 0}, Position{0, 3}, 1000);
        
        engine.wait(1100); // Wait for arrival
        
        CHECK(mockUI.movesCompleted == 1);
        CHECK(mockUI.piecesCaptured == 1);
    }
}
TEST_CASE("GameEngine - Airborne Collisions and Fatal Deaths") {
    GameEngine engine(8, 8);
    
    SUBCASE("Mid-air collision between enemies results in a capture") {
        Piece whiteRook(1, PieceColor::White, PieceKind::Rook, Position{4, 0});
        Piece blackRook(2, PieceColor::Black, PieceKind::Rook, Position{4, 7});
        
        engine.getGameState().board.addPiece(whiteRook);
        engine.getGameState().board.addPiece(blackRook);
        
        MockObserver mockUI;
        engine.addObserver(&mockUI);

        // Verify that both requests are legal and start successfully
        REQUIRE(engine.requestMove(Position{4, 0}, Position{4, 4}, 1000) == true);
        REQUIRE(engine.requestMove(Position{4, 7}, Position{4, 3}, 1000) == true);

        // 600ms is not enough time for them to cross paths from columns 0 and 7.
        // Wait 900ms so they meet in the middle and trigger the collision!
        engine.wait(900); 

        CHECK(mockUI.piecesCaptured >= 1);
    }

    SUBCASE("Capturing the King ends the game") {
        Piece whiteKing(1, PieceColor::White, PieceKind::King, Position{4, 0});
        Piece blackRook(2, PieceColor::Black, PieceKind::Rook, Position{4, 7});
        
        engine.getGameState().board.addPiece(whiteKing);
        engine.getGameState().board.addPiece(blackRook);

        engine.requestMove(Position{4, 7}, Position{4, 0}, 1000); 
        CHECK(engine.getSnapshot().isGameOver == false);
        
        engine.wait(1100); 
        
        CHECK(engine.getSnapshot().isGameOver == true);
    }
}

TEST_CASE("GameEngine - Time Advancement and Rest States") {
    GameEngine engine(8, 8);
    
    Piece rook(1, PieceColor::White, PieceKind::Rook, Position{0, 0});
    engine.getGameState().board.addPiece(rook);

    SUBCASE("Pieces return to Idle state after cooldown period expires") {
        engine.requestMove(Position{0, 0}, Position{1, 0}, 1000);
        
        engine.wait(1000);
        
        auto pieceOpt = engine.getSnapshot().getPieceAt(Position{1, 0});
        REQUIRE(pieceOpt.has_value());
        
        CHECK((pieceOpt.value().state == PieceState::LongRest || pieceOpt.value().state == PieceState::ShortRest));
        
        engine.wait(5000); 
        
        auto refreshedPiece = engine.getSnapshot().getPieceAt(Position{1, 0});
        REQUIRE(refreshedPiece.has_value());
        CHECK(refreshedPiece.value().state == PieceState::Idle);
    }
}