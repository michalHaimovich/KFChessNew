#pragma once
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <string>
#include <thread>
#include <functional>

typedef websocketpp::client<websocketpp::config::asio_client> client;

class NetworkClient {
public:
    NetworkClient();
    ~NetworkClient();

    bool connect(const std::string& uri);
    void disconnect();
    
    void sendMove(int fromX, int fromY, int toX, int toY);
    void sendJump(int toX, int toY);

    void setOnMessageCallback(std::function<void(const std::string&)> callback);

    // Generic send function for JSON strings
    void send(const std::string& message);
    
    //  Getter to check if connection is fully established
    bool isConnected() const { return m_connected; }

private:
    void on_open(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg);

    client m_client;
    websocketpp::connection_hdl m_hdl;
    std::thread m_thread;
    bool m_connected;
    std::function<void(const std::string&)> m_messageCallback;
};