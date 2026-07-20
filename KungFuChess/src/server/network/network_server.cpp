#include "network_server.hpp"
#include <iostream>

NetworkServer::NetworkServer() 
    : m_gameSession([this](websocketpp::connection_hdl hdl, const std::string& msg) { 
          this->sendToClient(hdl, msg); 
      }), 
      m_controller(m_gameSession.getEngine()) 
{    
    m_server.init_asio();

    m_server.set_open_handler(bind(&NetworkServer::on_open, this, std::placeholders::_1));
    m_server.set_close_handler(bind(&NetworkServer::on_close, this, std::placeholders::_1));
    m_server.set_message_handler(bind(&NetworkServer::on_message, this, std::placeholders::_1, std::placeholders::_2));
}

void NetworkServer::sendToClient(websocketpp::connection_hdl hdl, const std::string& message) {
    try {
        m_server.send(hdl, message, websocketpp::frame::opcode::text);
    } catch (const websocketpp::exception& e) {
        std::cout << "Send failed: " << e.what() << std::endl;
    }
}

void NetworkServer::run(uint16_t port) {
    m_server.listen(port);
    m_server.start_accept();
    std::cout << "Server listening on port " << port << "..." << std::endl;
    m_server.run(); 
}

void NetworkServer::on_open(websocketpp::connection_hdl hdl) {
    m_gameSession.addClient(hdl);
}

void NetworkServer::on_close(websocketpp::connection_hdl hdl) {
    m_gameSession.removeClient(hdl);
}

void NetworkServer::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    std::string payload = msg->get_payload(); 

    PlayerRole role = m_gameSession.getRole(hdl);
    
    bool success = m_controller.processCommand(payload, role);
    
    if (!success) {
        std::cout << "[NetworkServer] Invalid command or unauthorized role." << std::endl;
    }
}