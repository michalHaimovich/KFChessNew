#pragma once
#include <string>
#include <vector>
#include "model/board.hpp"

class BoardParser {
public:
    // Parses a multiline board description into a board object.
    static Board parse(const std::string& text);
};