#include "realtime/real_time_arbiter.hpp"
#include <algorithm>

RealTimeArbiter::RealTimeArbiter() : currentTime(0) {}

void RealTimeArbiter::startMotion(const Piece& piece, Position source, Position destination, long durationMs, MotionType type) {
    long targetTime = currentTime + durationMs;
    activeMotions.push_back({piece, source, destination, currentTime, targetTime, type});
}

void RealTimeArbiter::advanceTime(long ms) {
    currentTime += ms;
}

std::vector<Motion>& RealTimeArbiter::getActiveMotionsRef() {
    return activeMotions;
}

std::vector<Motion> RealTimeArbiter::popArrivedMotions() {
    std::vector<Motion> arrived;
    std::vector<Motion> stillActive;

    for (const auto& motion : activeMotions) {
        if (currentTime >= motion.arrivalTime) {
            arrived.push_back(motion);
        } else {
            stillActive.push_back(motion);
        }
    }
    activeMotions = stillActive;
    return arrived;
}

void RealTimeArbiter::cleanupDeadMotions() {
    activeMotions.erase(
        std::remove_if(activeMotions.begin(), activeMotions.end(),
            [](const Motion& m) { return m.isDead; }),
        activeMotions.end()
    );
}