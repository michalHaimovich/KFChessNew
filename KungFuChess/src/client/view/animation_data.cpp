#include "view/animation_data.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

AnimationData::AnimationData(const std::string& directoryPath, int targetSize) 
    : msPerFrame(1000), isLoop(false) {
    loadConfig(directoryPath + "/config.json");
    loadFrames(directoryPath, targetSize);
}

void AnimationData::loadConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    int framesPerSec = 6;
    isLoop = true;
    
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        size_t fpsPos = content.find("\"frames_per_sec\"");
        if (fpsPos != std::string::npos) {
            size_t colonPos = content.find(":", fpsPos);
            size_t commaPos = content.find(",", colonPos);
            if (colonPos != std::string::npos && commaPos != std::string::npos) {
                framesPerSec = std::stoi(content.substr(colonPos + 1, commaPos - colonPos - 1));
            }
        }

        size_t loopPos = content.find("\"is_loop\"");
        if (loopPos != std::string::npos) {
            size_t colonPos = content.find(":", loopPos);
            size_t endPos = content.find("\n", colonPos);
            if (colonPos != std::string::npos && endPos != std::string::npos) {
                isLoop = (content.substr(colonPos + 1, endPos - colonPos - 1).find("true") != std::string::npos);
            }
        }
    }
    
    msPerFrame = 1000 / (framesPerSec > 0 ? framesPerSec : 1);
}

void AnimationData::loadFrames(const std::string& dirPath, int targetSize) {
    int i = 1;
    while (true) {
        std::string filePath = dirPath + "/sprites/" + std::to_string(i) + ".png";
        if (!std::filesystem::exists(filePath)) break;
        
        Img frame;
        frame.read(filePath, std::make_pair(targetSize, targetSize), true);
        frames.push_back(frame);
        i++;
    }
}