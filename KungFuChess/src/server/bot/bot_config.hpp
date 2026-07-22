#pragma once
#include <algorithm>

namespace BotConstants {
    // ELO Tiers bounds
    constexpr int ELO_BEGINNER = 1000;
    constexpr int ELO_NOVICE = 1200;
    constexpr int ELO_INTERMEDIATE = 1400;
    constexpr int ELO_ADVANCED = 1600;
    constexpr int ELO_CLUB = 1800;
    constexpr int ELO_EXPERIENCED = 2000;
    constexpr int ELO_CANDIDATE = 2200;
    constexpr int ELO_MASTER = 2400;

    // Default configuration values
    constexpr long DEFAULT_REACTION_MS = 2000;
    constexpr long MIN_REACTION_MS = 100;
    constexpr int DEFAULT_APM = 15;
    constexpr int MAX_APM = 300; // Represents "unlimited" for masters
}

struct BotConfig {
    long reactionTimeMs;  // How long before the bot notices a threat
    long actionDelayMs;   // Derived from APM (60000 / APM) - Minimum time between any action
    bool canJump;         // Is the bot aware of the jump mechanic?
    int lookaheadRadius;  // How far the bot searches for opportunities/threats (in cells)

    // Static factory method to generate a configuration based on ELO
    static BotConfig fromElo(int elo) {
        BotConfig config;

        if (elo < BotConstants::ELO_BEGINNER) {
            config.reactionTimeMs = 2000;
            config.actionDelayMs = 60000 / 15; // 15 APM
            config.canJump = false;
            config.lookaheadRadius = 1;
        } else if (elo < BotConstants::ELO_NOVICE) {
            config.reactionTimeMs = 1500;
            config.actionDelayMs = 60000 / 25; // 25 APM
            config.canJump = false;
            config.lookaheadRadius = 2;
        } else if (elo < BotConstants::ELO_INTERMEDIATE) {
            config.reactionTimeMs = 1200;
            config.actionDelayMs = 60000 / 40; // 40 APM
            config.canJump = false;
            config.lookaheadRadius = 3;
        } else if (elo < BotConstants::ELO_ADVANCED) {
            config.reactionTimeMs = 800;
            config.actionDelayMs = 60000 / 60; // 60 APM
            config.canJump = false;
            config.lookaheadRadius = 5;
        } else if (elo < BotConstants::ELO_CLUB) {
            config.reactionTimeMs = 500;
            config.actionDelayMs = 60000 / 80; // 80 APM
            config.canJump = true;             // Starts learning to jump
            config.lookaheadRadius = 8;        // Full board
        } else if (elo < BotConstants::ELO_EXPERIENCED) {
            config.reactionTimeMs = 300;
            config.actionDelayMs = 60000 / 120; // 120 APM
            config.canJump = true;
            config.lookaheadRadius = 8;
        } else if (elo < BotConstants::ELO_CANDIDATE) {
            config.reactionTimeMs = 200;
            config.actionDelayMs = 60000 / 200; // 200 APM
            config.canJump = true;
            config.lookaheadRadius = 8;
        } else {
            // Master level
            config.reactionTimeMs = BotConstants::MIN_REACTION_MS;
            config.actionDelayMs = 60000 / BotConstants::MAX_APM; 
            config.canJump = true;
            config.lookaheadRadius = 8;
        }

        return config;
    }
};