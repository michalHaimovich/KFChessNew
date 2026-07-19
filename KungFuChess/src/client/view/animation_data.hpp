#pragma once
#include <vector>
#include <string>
#include "view/img.hpp"

class AnimationData {
public:
    std::vector<Img> frames;
    long msPerFrame; 
    bool isLoop;

    AnimationData(const std::string& directoryPath, int targetSize);

private:
    void loadConfig(const std::string& configPath);
    void loadFrames(const std::string& dirPath, int targetSize);
};