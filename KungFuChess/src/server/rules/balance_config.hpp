#pragma once

struct CooldownConfig {
    long shortRestMs; 
    long longRestMs;  
};

struct GameBalance {
    CooldownConfig pawn   { 800,  1500 };
    CooldownConfig knight { 1000, 2000 };
    CooldownConfig bishop { 1000, 2200 };
    CooldownConfig rook   { 1200, 2500 };
    CooldownConfig queen  { 1500, 3000 };
    CooldownConfig king   { 1000, 2000 };
    
    // void loadFromJson(const std::string& path);
};