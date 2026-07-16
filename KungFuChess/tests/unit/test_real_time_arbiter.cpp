// #include "doctest.h"
// #include "realtime/motion.hpp"
// #include "realtime/real_time_arbiter.hpp"

// TEST_CASE("RealTimeArbiter - Simultaneous Motions") {
//     RealTimeArbiter arbiter;
//     Piece whiteKnight(1, PieceColor::White, PieceKind::Knight, Position{0, 1});
//     Piece blackRook(2, PieceColor::Black, PieceKind::Rook, Position{7, 7});

//     SUBCASE("Motions resolve exactly when their simulated time is reached") {
//         // Start two simultaneous motions with different durations
//         arbiter.startMotion(whiteKnight, Position{0, 1}, Position{2, 2}, 1000); // Arrives at t=1000
//         arbiter.startMotion(blackRook, Position{7, 7}, Position{7, 0}, 1500);   // Arrives at t=1500

//         // Advance 500ms - no one should arrive yet
//         auto arrivals1 = arbiter.advanceTime(500); 
//         CHECK(arrivals1.empty() == true);

//         // Advance another 500ms (Total time = 1000ms) - Knight arrives
//         auto arrivals2 = arbiter.advanceTime(500); 
//         REQUIRE(arrivals2.size() == 1);
//         CHECK(arrivals2[0].piece.id == 1); // White Knight

//         // Advance another 500ms (Total time = 1500ms) - Rook arrives
//         auto arrivals3 = arbiter.advanceTime(500); 
//         REQUIRE(arrivals3.size() == 1);
//         CHECK(arrivals3[0].piece.id == 2); // Black Rook
//     }
// }