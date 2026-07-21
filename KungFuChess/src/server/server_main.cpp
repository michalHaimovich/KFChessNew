#include "network/network_server.hpp"
#include <iostream>

int PORT = 9002; // make it env later

int main() {
    try {
        NetworkServer server;
        std::cout << "Starting Kung Fu Chess Server..." << std::endl;
        server.run(PORT);
    } catch (const std::exception& e) {
        std::cerr << "Server Error: " << e.what() << std::endl;
    }
    return 0;
}