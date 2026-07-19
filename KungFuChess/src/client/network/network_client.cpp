#include "network_client.hpp"
#include <iostream>

NetworkClient::NetworkClient() : m_connected(false) {
    m_client.clear_access_channels(websocketpp::log::alevel::all);
    m_client.init_asio();

    m_client.set_open_handler(std::bind(&NetworkClient::on_open, this, std::placeholders::_1));
    m_client.set_message_handler(std::bind(&NetworkClient::on_message, this, std::placeholders::_1, std::placeholders::_2));
}

NetworkClient::~NetworkClient() {
    disconnect();
}

bool NetworkClient::connect(const std::string& uri) {
    websocketpp::lib::error_code ec;
    client::connection_ptr con = m_client.get_connection(uri, ec);
    
    if (ec) {
        std::cerr << "Could not create connection: " << ec.message() << std::endl;
        return false;
    }

    m_client.connect(con);
    m_thread = std::thread([this]() { m_client.run(); });
    return true;
}

void NetworkClient::disconnect() {
    if (m_connected) {
        websocketpp::lib::error_code ec;
        m_client.close(m_hdl, websocketpp::close::status::normal, "", ec);
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }
}

void NetworkClient::on_open(websocketpp::connection_hdl hdl) {
    m_hdl = hdl;
    m_connected = true;
    std::cout << "Connected to Kung Fu Chess Server!" << std::endl;
}

void NetworkClient::on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
    if (m_messageCallback) {
        m_messageCallback(msg->get_payload());
    }
}

void NetworkClient::sendMove(int fromX, int fromY, int toX, int toY) {
    if (!m_connected) return;
    std::string payload = "MOVE " + std::to_string(fromX) + "," + std::to_string(fromY) + 
                          " TO " + std::to_string(toX) + "," + std::to_string(toY);
    m_client.send(m_hdl, payload, websocketpp::frame::opcode::text);
}

void NetworkClient::sendJump(int toX, int toY) {
    if (!m_connected) return;
    std::string payload = "JUMP TO " + std::to_string(toX) + "," + std::to_string(toY);
    m_client.send(m_hdl, payload, websocketpp::frame::opcode::text);
}

void NetworkClient::setOnMessageCallback(std::function<void(const std::string&)> callback) {
    m_messageCallback = callback;
}