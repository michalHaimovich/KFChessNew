#pragma once
#include <string>
#include "texttests/script_parser.hpp"

class ScriptRunner {
public:
    // Runs a script file and returns whether the checks passed.
    static bool run(const std::string& filepath);

    // Runs a script from an in-memory list of lines.
    static bool runFromMemory(const std::vector<std::string>& lines);

    // Runs a script from parsed commands.
    static bool runFromMemory(const std::vector<TestCommand>& commands);
};