// #include "doctest.h"
// #include "model/piece.hpp"
// #include "model/position.hpp"

// TEST_CASE("Piece - Lifecycle and State") {
    
//     // Create a White Rook at position (0, 0) with ID 1
//     Piece rook(1, PieceColor::White, PieceKind::Rook, Position{0, 0});

//     SUBCASE("Piece initializes with correct attributes and Idle state") {
//         CHECK(rook.id == 1);
//         CHECK(rook.color == PieceColor::White);
//         CHECK(rook.kind == PieceKind::Rook);
//         CHECK(rook.cell == Position{0, 0});
        
//         // State should default to Idle
//         CHECK(rook.state == PieceState::Idle);
//     }

//     SUBCASE("Piece state can transition to moving or captured without storing destination or time") {
//         // Transition to moving
//         rook.state = PieceState::Moving;
//         CHECK(rook.state == PieceState::Moving);

//         // Transition to captured
//         rook.state = PieceState::Captured;
//         CHECK(rook.state == PieceState::Captured);
        
//         // Verify other attributes remain completely unaffected by state changes
//         CHECK(rook.cell == Position{0, 0});
//     }
// }