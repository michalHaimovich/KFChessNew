#pragma once
#include <string>
#include <vector>
#include <optional>

enum class CommandType {
    BoardSetup,
    Jump,
    Click,
    Wait,
    PrintBoard
};

struct TestCommand {
    CommandType type;

    int x = 0;
    int y = 0;
    long ms = 0;
    std::string textData = "";
};

class ScriptParser {
public:
    // Parses a script file into a list of commands.
    static std::vector<TestCommand> parse(const std::string& filepath);

    // Parses commands directly from an in-memory list of lines.
    static std::vector<TestCommand> parseFromMemory(const std::vector<std::string>& lines);
};