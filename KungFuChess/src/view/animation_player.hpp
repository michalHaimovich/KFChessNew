#pragma once
#include <memory>
#include "view/animation_data.hpp"

class AnimationPlayer {
private:
    std::shared_ptr<AnimationData> data;
    long elapsedMs;
    int currentFrameIdx;
    bool finished;

public:
    AnimationPlayer();
    
    void play(std::shared_ptr<AnimationData> newData);
    
    void update(long dt);
    
    const Img& getCurrentFrame() const;
    bool isFinished() const;
};