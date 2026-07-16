#include "view/animation_player.hpp"

AnimationPlayer::AnimationPlayer() 
    : data(nullptr), elapsedMs(0), currentFrameIdx(0), finished(false) {}

void AnimationPlayer::play(std::shared_ptr<AnimationData> newData) {
    if (data == newData) return; 
    
    data = newData;
    elapsedMs = 0;
    currentFrameIdx = 0;
    finished = false;
}

void AnimationPlayer::update(long dt) {
    if (finished || !data || data->frames.empty()) return;

    elapsedMs += dt;
    
    while (elapsedMs >= data->msPerFrame) {
        elapsedMs -= data->msPerFrame;
        currentFrameIdx++;

        if (currentFrameIdx >= data->frames.size()) {
            if (data->isLoop) {
                currentFrameIdx = 0;
            } else {
                currentFrameIdx = data->frames.size() - 1;
                finished = true;
                break; 
            }
        }
    }
}

const Img& AnimationPlayer::getCurrentFrame() const {
    if (!data || data->frames.empty()) {
        static Img emptyImg; 
        return emptyImg;
    }
    return data->frames[currentFrameIdx];
}

bool AnimationPlayer::isFinished() const {
    return finished;
}