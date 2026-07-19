#include "network/network_server.hpp"
#include <iostream>

int main() {
    try {
        NetworkServer server;
        std::cout << "Starting Kung Fu Chess Server..." << std::endl;
        server.run(9002);
    } catch (const std::exception& e) {
        std::cerr << "Server Error: " << e.what() << std::endl;
    }
    return 0;
}