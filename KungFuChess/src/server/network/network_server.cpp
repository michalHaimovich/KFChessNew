#include "network_server.hpp"
#include <functional>


NetworkServer::NetworkServer() {
    m_endpoint.set_error_channels(websocketpp::log::elevel::all);
    m_endpoint.set_access_channels(websocketpp::log::alevel::all);

    m_endpoint.init_asio();

    m_endpoint.set_open_handler(std::bind(&NetworkServer::on_open, this, std::placeholders::_1));
    m_endpoint.set_close_handler(std::bind(&NetworkServer::on_close, this, std::placeholders::_1));
    m_endpoint.set_message_handler(std::bind(&NetworkServer::on_message, this, std::placeholders::_1, std::placeholders::_2));
}

void NetworkServer::run(uint16_t port) {
    m_endpoint.listen(port);
    m_endpoint.start_accept();
    std::cout << "Server listening on port " << port << "..." << std::endl;
    m_endpoint.run();
}

void NetworkServer::on_open(websocketpp::connection_hdl hdl) {
    std::cout << "New client connected!" << std::endl;
}

void NetworkServer::on_close(websocketpp::connection_hdl hdl) {
    std::cout << "Client disconnected." << std::endl;
}

void NetworkServer::on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "Received message: " << msg->get_payload() << std::endl;
}