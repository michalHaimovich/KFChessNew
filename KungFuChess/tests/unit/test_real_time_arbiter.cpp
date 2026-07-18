#include "doctest.h"
#include "realtime/real_time_arbiter.hpp"
#include "model/piece.hpp"

TEST_CASE("RealTimeArbiter - Time Management") {
    RealTimeArbiter arbiter;

    SUBCASE("Initial time is zero") {
        // Verify that a newly created arbiter starts at time 0
        CHECK(arbiter.getCurrentTime() == 0);
    }

    SUBCASE("Advancing time increases current time correctly") {
        // Advance time by 50ms and verify
        arbiter.advanceTime(50);
        CHECK(arbiter.getCurrentTime() == 50);
        
        // Advance time by another 100ms and verify the total is 150ms
        arbiter.advanceTime(100);
        CHECK(arbiter.getCurrentTime() == 150);
    }
}

TEST_CASE("RealTimeArbiter - Motion Management") {
    RealTimeArbiter arbiter;
    Piece testPiece(1, PieceColor::White, PieceKind::Pawn, Position{1, 1});

    SUBCASE("Starting a motion adds it to active motions") {
        // Start a normal motion taking 1000ms
        arbiter.startMotion(testPiece, Position{1, 1}, Position{2, 1}, 1000);
        
        // Retrieve a constant reference to the active motions list
        const auto& motions = arbiter.getActiveMotionsConst();
        
        REQUIRE(motions.size() == 1);
        CHECK(motions[0].piece.id == 1);
        CHECK(motions[0].source == Position{1, 1});
        CHECK(motions[0].destination == Position{2, 1});
    }

    SUBCASE("cleanupDeadMotions removes motions marked as dead") {
        arbiter.startMotion(testPiece, Position{1, 1}, Position{2, 1}, 1000);
        
        // Retrieve a mutable reference and manually mark the motion as dead
        arbiter.getActiveMotionsRef()[0].isDead = true;
        
        // Call the cleanup method
        arbiter.cleanupDeadMotions();
        
        // Verify the motion was completely removed from the list
        CHECK(arbiter.getActiveMotionsConst().empty() == true);
    }

    SUBCASE("popArrivedMotions retrieves and removes completed motions") {
        // Start a motion that takes 1000ms to arrive
        arbiter.startMotion(testPiece, Position{1, 1}, Position{2, 1}, 1000);
        
        // Advance time partially (500ms). The piece should still be in transit.
        arbiter.advanceTime(500);
        auto arrivedEarly = arbiter.popArrivedMotions();
        
        CHECK(arrivedEarly.empty() == true);
        CHECK(arbiter.getActiveMotionsConst().size() == 1); // Still active
        
        // Advance time past the arrival time (total 1100ms).
        arbiter.advanceTime(600); 
        auto arrivedOnTime = arbiter.popArrivedMotions();
        
        // Verify the motion was popped out of the active list
        REQUIRE(arrivedOnTime.size() == 1);
        CHECK(arrivedOnTime[0].piece.id == 1);
        
        // Verify the active list is now empty
        CHECK(arbiter.getActiveMotionsConst().empty() == true);
    }
}