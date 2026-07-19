#include "texttests/script_runner.hpp"
#include "texttests/script_parser.hpp"
#include <iostream>
#include "engine/game_engine.hpp"
#include "input/controller.hpp"
#include "input/board_mapper.hpp"
#include "io/board_parser.hpp"
#include "io/board_printer.hpp"

bool ScriptRunner::run(const std::string& filepath) {
    std::vector<TestCommand> commands = ScriptParser::parse(filepath);
    if (commands.empty()) return false;
    return runFromMemory(commands); 
}

bool ScriptRunner::runFromMemory(const std::vector<TestCommand>& commands) {
    if (commands.empty()) return false;

    GameEngine* engine = nullptr;
    Controller* controller = nullptr;
    bool allTestsPassed = true;

    for (const auto& cmd : commands) {
        switch (cmd.type) {
            case CommandType::BoardSetup: {
                Board b = BoardParser::parse(cmd.textData);
                engine = new GameEngine(b.getWidth(), b.getHeight());
                engine->getGameState().board = b;
                
                int rows = static_cast<int>(b.getHeight());
                int cols = static_cast<int>(b.getWidth());
                
                BoardMapper testMapper(0, 0, 100);
                
                controller = new Controller(*engine, testMapper);
                break;
            }
            case CommandType::Click:
                if (controller) controller->click(cmd.x, cmd.y);
                break;
            
           case CommandType::Jump:
                if (controller) controller->jump(cmd.x, cmd.y);
                 break;
                 
            case CommandType::Wait:
                if (engine) engine->wait(cmd.ms);
                break;
                
            case CommandType::PrintBoard: { 
                if (engine) {
                    GameSnapshot snapshot = engine->getSnapshot();
                    GameState renderState(snapshot.boardWidth, snapshot.boardHeight);
                    for (const auto& p : snapshot.stationaryPieces) {
                        renderState.board.addPiece(p);
                    }

                    std::string actual = BoardPrinter::print(renderState);
                    std::cout << actual;
                    
                    if (!cmd.textData.empty() && actual != cmd.textData) {
                        allTestsPassed = false;
                    }
                }
                break;
            }
        }
    }

    delete controller;
    delete engine;

    return allTestsPassed;
}

bool ScriptRunner::runFromMemory(const std::vector<std::string>& lines) {
    std::vector<TestCommand> commands = ScriptParser::parseFromMemory(lines);
    return runFromMemory(commands);
}