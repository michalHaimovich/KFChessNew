#include "texttests/script_parser.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream stream(line);
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool isCommandKeyword(const std::string& line) {
    return line == "Board" || line == "print board" || 
           line.find("click") == 0 || line.find("wait") == 0 || line.find("jump") == 0;
}

std::vector<TestCommand> ScriptParser::parse(const std::string& filepath) {
    std::vector<TestCommand> commands;
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open script file " << filepath << "\n";
        return commands;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) lines.push_back(line);
    }

    return parseFromMemory(lines); // avoid code duplication; call the other function
}

std::vector<TestCommand> ScriptParser::parseFromMemory(const std::vector<std::string>& lines) {
    std::vector<TestCommand> commands;

    for (size_t i = 0; i < lines.size(); ++i) {
        std::string cmdStr = lines[i];
        std::vector<std::string> tokens = tokenize(cmdStr);
        
        if (tokens.empty()) continue;

        TestCommand cmd;

        if (cmdStr == "Board") {
            cmd.type = CommandType::BoardSetup;
            while (i + 1 < lines.size() && !isCommandKeyword(lines[i + 1])) {
                cmd.textData += lines[++i] + "\n";
            }
            commands.push_back(cmd);
        } 
        else if (tokens[0] == "click" && tokens.size() >= 3) {
            cmd.type = CommandType::Click;
            cmd.x = std::stoi(tokens[1]);
            cmd.y = std::stoi(tokens[2]);
            commands.push_back(cmd);
        } 
        else if (tokens[0] == "jump" && tokens.size() >= 3) { // jump command addition
            cmd.type = CommandType::Jump;
            cmd.x = std::stoi(tokens[1]);
            cmd.y = std::stoi(tokens[2]);
            commands.push_back(cmd);
        }
        else if (tokens[0] == "wait" && tokens.size() >= 2) {
            cmd.type = CommandType::Wait;
            cmd.ms = std::stol(tokens[1]);
            commands.push_back(cmd);
        } 
        else if (cmdStr == "print board") {
            cmd.type = CommandType::PrintBoard;
            while (i + 1 < lines.size() && !isCommandKeyword(lines[i + 1])) {
                cmd.textData += lines[++i] + "\n";
            }
            commands.push_back(cmd);
        }
    }

    return commands;
}