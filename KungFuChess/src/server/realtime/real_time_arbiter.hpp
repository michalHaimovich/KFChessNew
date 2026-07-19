#pragma once
#include <vector>
#include "model/motion.hpp"

class RealTimeArbiter {
private:
    long currentTime;
    std::vector<Motion> activeMotions;

public:
    // Constructs the arbiter with an initial time value.
    RealTimeArbiter();

    // Returns the current simulated time.
    long getCurrentTime() const { return currentTime; }

    // Starts a new motion for a piece.
    void startMotion(const Piece& piece, Position source, Position destination, long durationMs, MotionType type = MotionType::Normal);

    // Advances the simulation clock.
    void advanceTime(long ms);

    // Returns a mutable reference to the active motions.
    std::vector<Motion>& getActiveMotionsRef();

    // Returns a read-only view of the active motions.
    const std::vector<Motion>& getActiveMotionsConst() const { return activeMotions; }

    // Removes motions that have finished arriving.
    std::vector<Motion> popArrivedMotions();

    // Removes motions that have been marked dead.
    void cleanupDeadMotions();
};