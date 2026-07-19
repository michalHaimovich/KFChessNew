#pragma once
#include <websocketpp/config/asio_no_tls.hpp> 
#include <websocketpp/server.hpp>
#include <iostream>

typedef websocketpp::server<websocketpp::config::asio> server_t;
typedef server_t::message_ptr message_ptr;

class NetworkServer {
public:
    NetworkServer();
    
    void run(uint16_t port); 

private:
    server_t m_endpoint;

    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, message_ptr msg);
};