#include "game_application.hpp"
#include <iostream>

int main() {
    try {
        GameApplication app;
        app.run();      
    } catch (const std::exception& e) {
        std::cerr << "Fatal Crash: " << e.what() << std::endl;
    }
    return 0;
}